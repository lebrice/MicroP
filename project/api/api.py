from flask import Flask, request
from flask_restful import Resource, Api
import matplotlib.pyplot as plt
import json
import tensorflow as tf
from typing import List, Dict, Tuple
import numpy as np
app = Flask(__name__)
api = Api(app)
from flask_restful import reqparse
import speech_model
from speech_model import get_dummy_model

accelerometer_data = {}


import os
current_dir = ""
try:
    current_dir = os.path.dirname(__file__)
except:
    current_dir = os.getcwd()

saved_model_dir = f"{current_dir}/saved_model/"


class Home(Resource):
    def get(self):
        return {'hello': 'world'}

class Speech(Resource):
    def __init__(self):
        self.classifier = None

    def post(self):
        request_data = request.get_json()
        import matplotlib.image as mpimage
        image = mpimage.imread(request.files["image"])
        # import matplotlib.pyplot as plt
        # plt.imshow(image)
        # plt.show()

        # data = request_data["data"]
        # _x = data["x"]
        # _y = data["y"]
        # _z = data["z"]
        if self.classifier == None:
            self.classifier = speech_model.get_model()
        
        image = mpimage.imread(f"{current_dir}/spectrograms/valid/0_jackson_6.png")

        def predict_input_fn():
            return {
                "x": np.reshape(image[...,0], (64,64,1))
            }

        result = next(self.classifier.predict(predict_input_fn))
        print(result)
        return {
            "classes": int(result["classes"]),
            "probabilities": result["probabilities"].tolist()

        }


class Accelerometer(Resource):
    # def __init__(self):

    def get(self):
        return accelerometer_data

    

        # with tf.Session(graph=graph) as session:
        #     session.run(tf.global_variables_initializer())

        #     prediction = session.run(
        #         output_tensor,
        #         feed_dict={
        #             input_tensor: data
        #         }
        #     )
        #     print("Prediction:", prediction)
        #     return {
        #         "result": int(prediction)
        #     }
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
api.add_resource(Speech, '/speech/')

if __name__ == '__main__':
    app.run(debug=True)

