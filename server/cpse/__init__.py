from datetime import datetime
import os, base64, pickle

from flask import Flask, request, render_template
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
    
    ### People ###
    @app.route('/reset_avg')
    def reset_avg():
        avg_people = int(request.args.get("avg"))
        total_people = int(request.args.get("total"))
        redis_client.set("avg_people", avg_people)
        redis_client.set("total_people", total_people)
        app.logger.info("avg_people setted to %d", avg_people)
        app.logger.info("total_people setted to %d", total_people)
        return 'OK'

    def get_avg_people():
        avg_people = redis_client.get("avg_people")
        # total_people = request.args.get('total')
        print("Avg People: {}".format(avg_people))
        return avg

    # def update_avg(n_people):
    #     avg_people = get_avg_people()


    #     return avg

    @app.route('/people', methods=['POST'])
    def set_people():
        print(request.is_json)
        if request.is_json:
            data = request.get_json()
            n_people = int(data['n_people'])
            redis_client.set("n_people", n_people)
            app.logger.info('n_people setted to %d', n_people)
        return 'OK'

    # @app.route('/getpeople')
    def get_people():
        n_people = int(redis_client.get("n_people"))
        print("People: {}".format(n_people))
        return n_people

    ### Decibels ###
    @app.route('/decibels', methods=['POST'])
    def set_decibels():
        print(request.is_json)
        if request.is_json:
            data = request.get_json()
            decibels = int(data['db'])
            ts = datetime.now().timestamp()
            tup_db = (ts, decibels)
            redis_client.lpush('db_values', pickle.dumps(tup_db))
            app.logger.info('%d decibels sent for ts %d', decibels, ts)
        return 'OK'

    # @app.route('/getdb')
    def get_decibels():
        decibels = []
        tuples = redis_client.lrange('db_values', 0, 50)
        tuples.reverse()
        for t in tuples:
            obj = pickle.loads(t)
            dt_object = datetime.fromtimestamp(obj[0])
            print("Date: {}\tdb: {}".format(dt_object, obj[1]))
            decibels.append({"ts": dt_object.strftime("%d/%m/%Y %H:%M:%S"), "db": obj[1]})
        return decibels

    @app.route('/pop')
    def pip():
        db_values = redis_client.lpop('db_values')
        if (db_values is not None):
            obj = pickle.loads(db_values)
            print(obj)
        return 'OK'

    ### Receiving Image ###
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
            # print("Total Count: {}\t Count: {}".format(total_count, count))
            app.logger.info("Total Count: %d\tCount: {}".format(total_count, count))
        else:
            # print(request.data)
            print("Data length: {}\tContent length: {}".format(len(request.data), request.content_length))
            total_img += request.data.decode()
            count -= 1
            # print("Count: {}".format(count))
            app.logger.info("Count: %d".format(count))
            if (count == 0):
                image_64_decode = base64.b64decode(total_img)
                img_name = datetime.now().strftime("pics/pic_%d_%m_%Y_%H_%M_%S") + ".jpg"
                with open(img_name, "wb") as f:
                    f.write(image_64_decode)
                app.logger.info("Image %s saved!".format(img_name))
                count = total_count
                total_img = ""
        return 'OK'

    ### Main ###
    @app.route('/')
    def index():
        # redis_client.set("n_people", 1)
        # n_people = redis_client.get("n_people")
        # print(n_people)
        people = {"number": get_people()}
        decibels = get_decibels()
        return render_template('index.html', people=people, decibels=decibels)

    @app.route('/hello')
    def hello():
        # redis_client.set("n_people", 1)
        # n_people = redis_client.get("n_people")
        # print(n_people)
        return 'Hello, World!'

    return app