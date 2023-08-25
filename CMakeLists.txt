cmake_minimum_required(VERSION 3.14)

project(tinywebserver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(tinywebserver
main.cpp  
./timer/lst_timer.cpp 
./http/http_conn.cpp 
./log/log.cpp 
./CGImysql/sql_connection_pool.cpp  
webserver.cpp 
config.cpp
)
target_link_libraries(tinywebserver
pthread
mysqlclient
)
# install(TARGETS tinywebserver
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})