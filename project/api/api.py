from flask import Flask, request
from flask_restful import Resource, Api
import matplotlib.pyplot as plt
import json
import tensorflow as tf
from typing import List, Dict, Tuple

app = Flask(__name__)
api = Api(app)
from flask_restful import reqparse

from speech_model import get_model


accelerometer_data = {}

class Home(Resource):
    def get(self):
        return {'hello': 'world'}

class Speech(Resource):
    pass


class Accelerometer(Resource):
    # def __init__(self):

    def get(self):
        return accelerometer_data

    def put(self, id):
        request_data = request.get_json()
        print(request_data)
        data = request_data["data"]
        # _x = data["x"]
        # _y = data["y"]
        # _z = data["z"]
        graph, input_tensor, output_tensor = get_model()
        with tf.Session(graph=graph) as session:
            session.run(tf.global_variables_initializer())

            prediction = session.run(
                output_tensor,
                feed_dict={
                    input_tensor: data
                }
            )
            print("Prediction:", prediction)
            return {
                "result": int(result)
            }
        # parser = reqparse.RequestParser()
        # parser.add_argument('x', type=List[float], help='X-axis accelerometer values')
        # parser.add_argument('y', type=List[float], help='Y-axis accelerometer values')
        # parser.add_argument('z', type=List[float], help='Z-axis accelerometer values')
        # args = parser.parse_args()
        # try:
        #     x = args["x"]
        #     y = args["y"]
        #     z = args["z"]
        #     return {"OK: OK"}, 200
        # except json.JSONDecodeError as e:
        #     return {"ERROR": e.msg}, 400
        # except RuntimeError as e:
        #     return {"ERROR": e.msg}, 400

api.add_resource(Home, '/')
api.add_resource(Accelerometer, '/accelerometer/<int:id>')

if __name__ == '__main__':
    app.run(debug=True)

