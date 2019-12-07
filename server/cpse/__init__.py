from datetime import datetime
import os, io, base64, pickle, re, json

from flask import Flask, request
from flask_redis import FlaskRedis

redis_client = FlaskRedis()

total_img = ""
total_count = -1
count = -1

def create_app(test_config=None):
    # create and configure the app
    app = Flask(__name__, instance_relative_config=True)
    redis_client.init_app(app)
    # app.config.from_mapping(
    #     SECRET_KEY='dev',
    #     DATABASE=os.path.join(app.instance_path, 'flaskr.sqlite'),
    # )

    if test_config is None:
        # load the instance config, if it exists, when not testing
        app.config.from_pyfile('config.py', silent=True)
    else:
        # load the test config if passed in
        app.config.from_mapping(test_config)

    # ensure the instance folder exists
    try:
        os.makedirs(app.instance_path)
    except OSError:
        pass

    @app.route('/')
    def hello():
        # redis_client.set("n_people", 1)
        # n_people = redis_client.get("n_people")
        # print(n_people)
        return 'Hello, World!'

    @app.route('/decibels', methods=['POST'])
    def set_decibels():
        print(request.is_json)
        if request.is_json:
            data = request.get_json()
            decibels = int(data['db'])
            ts = datetime.now().timestamp()
            tup_db = (ts, decibels)
            redis_client.lpush('db_values', pickle.dumps(tup_db))
        return 'Setted tuple Redis'

    @app.route('/getdecibels')
    def get_decibels():
        db_values = redis_client.lpop('db_values')
        if (db_values is not None):
            obj = pickle.loads(db_values)
            print(obj)
        return 'Getted tuple Redis'

    @app.route('/img', methods=['POST'])
    def get_img():
        global total_img
        global total_count
        global count
        print(request)
        print(request.mimetype)
        print(request.content_length)
        print(request.is_json)

        if request.is_json:
            data = request.get_json()
            total_count = int(data['count'])
            count = total_count
            total_img = ""
            print("Total Count: {}\t Count: {}".format(total_count, count))
        else:
            # print(request.data)
            print("Data length: {}\tContent length: {}".format(len(request.data), request.content_length))
            total_img += request.data.decode()
            count -= 1
            print("Count: {}".format(count))
            if (count == 0):
                image_64_decode = base64.b64decode(total_img)
                img_name = datetime.now().strftime("pics/pic_%d_%m_%Y_%H_%M_%S") + ".jpg"
                with open(img_name, "wb") as f:
                    f.write(image_64_decode)
                print("Image \"{}\" saved!".format(img_name))
                count = total_count
                total_img = ""
        return 'OK'

    return app