import flask
from flask import Flask, request, make_response, render_template
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

import matplotlib.image as mpimage
from make_spectrograms import make_spectrogram_from_wav_file

import google_speech_api
accelerometer_data = {}


import os
current_dir = ""
try:
    current_dir = os.path.dirname(__file__)
except:
    current_dir = os.getcwd()

saved_model_dir = f"{current_dir}/saved_model/"

@app.route("/microp")
def greetings():
    return render_template('index.html')

class Home(Resource):
    def get(self):
        return {'hello': 'world'}

class Speech(Resource):
    def __init__(self):
        self.classifier = None
        self.use_custom_model

    def post(self):
        """
        Method that handles the incoming data.
        """
        # Read the image file from the request
        audio_file = BytesIO()
        request.files["audio"].save(audio_file)
        audio_file.seek(0)


        # NOTE: Uncomment the next line to switch between the custom model and Google Speech API. 
        # result = self.use_custom_model(audio_file)
        result = self.use_google_model(audio_file)

        return {"result": result}

    def use_google_model(self, audio_file):
        """ Uses the Google Speech API to determine which digit is spoken in the audio clip. """
        string_result = google_speech_api.transcribe_audio(audio_file.read())
        print("Google Speech API result: ", string_result)
        result = 0
        try:
            result = int(string_result)
        finally:
            return result

    def use_custom_model(self, audio_file):
        """ Uses the TensorFlow model to determine which digit is spoken in the audio clip. """
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
        pred_dict = next(predictions)
        predictions.close()
            
        #TODO: figure out the common format we will use.
        # result = {
        #     "classes": int(pred_dict["classes"]),
        #     "probabilities": pred_dict["probabilities"].tolist()
        # }
        result = int(pred_dict["classes"])
        return str(result)


class Accelerometer(Resource):

    def post(self):
          # Read the image file from the request
        csv_file = BytesIO()
        request.files["accelerometer"].save(csv_file)
        csv_file.seek(0)

        data = np.genfromtxt(csv_file, delimiter=",", dtype=float)
        data = np.reshape(data, (10000, 2))
        
        t = np.linspace(0, 10, 10000)
        pitch = data[:10000, 0]
        roll = data[:10000, 1]
        
        from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
        from matplotlib.figure import Figure
        fig = Figure(dpi=600)
        axis = fig.add_subplot(1, 1, 1)
        axis.plot(t, pitch, 'b', label="pitch")
        axis.plot(t, roll, 'r', label="roll")
        fig.legend()
        
        canvas = FigureCanvas(fig)
        output = BytesIO()
        canvas.print_png(output)
        response = make_response(output.getvalue())
        response.mimetype = 'image/png'
        return response



api.add_resource(Home, '/')
api.add_resource(Accelerometer, '/accelerometer/')
api.add_resource(Speech, '/speech/')

if __name__ == '__main__':
    app.run(debug=True)

