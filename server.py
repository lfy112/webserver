from flask import Flask, jsonify

app = Flask(__name__)

# 模拟要发送的 JSON 数据
data = {
    "name": "John Doe",
    "age": 30,
    "email": "john@example.com"
}

@app.route('/data', methods=['GET'])
def get_data():
    return jsonify(data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=9006)
