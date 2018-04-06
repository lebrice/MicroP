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
from io import StringIO, BytesIO

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

        from make_spectrograms import make_spectrogram_from_wav_file

        # Read the image file from the request
        audio_file = BytesIO()
        request.files["audio"].save(audio_file)
        audio_file.seek(0)
        spectrogram = make_spectrogram_from_wav_file(audio_file)

        
        
        # load up a classifier
        if self.classifier is None:
            self.classifier = speech_model.get_model()
        
        # flatten the spectrogram into a one-row vector.
        predict_x = np.reshape(spectrogram, (1, 64*64))

        def eval_input_fn(spectrogram, batch_size=1):
            """An input function for evaluation or prediction"""

            spectrogram = tf.reshape(spectrogram, (1, 64*64))
            spectrogram = tf.cast(spectrogram, tf.float32)

            features={
                "x": spectrogram
            }
            # plt.imshow(np.reshape(features["x"], (64,64)))
            # plt.show()
            inputs = features

            # Convert the inputs to a Dataset.
            dataset = tf.data.Dataset.from_tensor_slices(inputs)
            # Batch the examples
            dataset = dataset.batch(batch_size)
            # Return the dataset.
            return dataset
        

        predictions = self.classifier.predict(lambda: eval_input_fn(predict_x))
        results = []
        for pred_dict in predictions:
            
            result = {
                "classes": int(pred_dict["classes"]),
                "probabilities": pred_dict["probabilities"].tolist()
            }
            results.append(result)
        predictions.close()
        return results


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

