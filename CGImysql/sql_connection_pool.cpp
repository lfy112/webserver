#include "sql_connection_pool.h"

#include <mysql/mysql.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <list>
#include <string>

using namespace std;

connection_pool::connection_pool() {
  m_CurConn = 0;
  m_FreeConn = 0;
}

// 局部static变量保证线程安全，单例模式，懒汉初始化，局部static变量的生命周期基本是程序运行周期
connection_pool *connection_pool::GetInstance() {
  static connection_pool connPool;
  return &connPool;
}

// 构造初始化
void connection_pool::init(string url, string User, string PassWord,
                           string DBName, int Port, int MaxConn,
                           int close_log) {
  m_url = url;
  m_Port = Port;
  m_User = User;
  m_PassWord = PassWord;
  m_DatabaseName = DBName;
  m_close_log = close_log;

  for (int i = 0; i < MaxConn; i++) {
    MYSQL *con = NULL;
    con = mysql_init(con);

    if (con == NULL) {
      LOG_ERROR("MySQL Error");
      exit(1);
    }
    con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(),
                             DBName.c_str(), Port, NULL, 0);

    if (con == NULL) {
      LOG_ERROR("MySQL Error");
      exit(1);
    }
    connList.push_back(con);
    ++m_FreeConn;
  }

  reserve = sem(m_FreeConn);  // 有m_FreeConn个连接资源，初始化信号量

  m_MaxConn = m_FreeConn;
}

// 当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *connection_pool::GetConnection() {
  MYSQL *con = NULL;

  if (0 == connList.size()) return NULL;

  reserve.wait();

  lock.lock();

  con = connList.front();
  connList.pop_front();

  --m_FreeConn;
  ++m_CurConn;

  lock.unlock();
  return con;
}

// 释放当前使用的连接
bool connection_pool::ReleaseConnection(MYSQL *con) {
  if (NULL == con) return false;

  lock.lock();

  connList.push_back(con);
  ++m_FreeConn;
  --m_CurConn;

  lock.unlock();

  reserve.post();  // 释放连接，原子+1
  return true;
}

// 销毁数据库连接池
void connection_pool::DestroyPool() {
  lock.lock();
  if (connList.size() > 0) {
    list<MYSQL *>::iterator it;
    for (it = connList.begin(); it != connList.end(); ++it) {
      MYSQL *con = *it;
      mysql_close(con);
    }
    m_CurConn = 0;
    m_FreeConn = 0;
    connList.clear();
  }

  lock.unlock();
}

// 当前空闲的连接数
int connection_pool::GetFreeConn() { return this->m_FreeConn; }

connection_pool::~connection_pool() { DestroyPool(); }

// RAII机制释放数据库连接
connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool) {
  *SQL = connPool->GetConnection();

  conRAII = *SQL;
  poolRAII = connPool;
}

connectionRAII::~connectionRAII() { poolRAII->ReleaseConnection(conRAII); }