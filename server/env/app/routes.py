from app import app
from flask import request, jsonify, Response
import json

@app.route('/')
@app.route('/index')
def index():
    return "Hello, World!"

@app.route('/data', methods=['POST'])
def parse_request():
    print(request.data)
    print(request.is_json)
    content=""
    if request.is_json:
        content = request.get_json()
        print("content:", content)
    return json.dumps(content)