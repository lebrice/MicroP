import flask
import os
from flask import Flask, request, make_response, render_template
from flask_restful import Resource, Api
import matplotlib.pyplot as plt
import json
import tensorflow as tf
import tablib
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

message = "Newman"

@app.route("/microp")
def greetings():
    return render_template('index.html', message=message)

@app.route("/accelerometer")
def acc():
    with open("data.csv", 'r') as f:
        dataset = tablib.Dataset()
        dataset.csv = f.read()
    return render_template('accelerometer.html', data=dataset.html)

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
        print("GOT POST FOR ACC DATA")
          # Read the image file from the request
        csv = open("data.csv", 'w')
        csv.write("Pitch, Roll\n")
        numbers = request.form["accelerometer"][1:-2]
        count = 0
        pitches = []
        rolls = []
        raw_nums = numbers.split(', ')
#        cleaned_nums = [v for i, v in enumerate(raw_nums) if (i < 4) or (i-4) % 5 > 0]
        cleaned_nums = [v for v in raw_nums if float(v) != 0.0]
        for val in cleaned_nums:
            if count % 2 == 0:
                csv.write(val + ", ")
                pitches.append(float(val))
            else:
                csv.write(val + "\n")
                rolls.append(float(val))
            count += 1
        if count % 2 == 1:
            csv.write("0.0\n")
            rolls.append(float(val))
        csv.close()

        xs = range(len(pitches))
        os.remove("static/graph.png")
        plt.plot(xs, pitches)
        plt.plot(xs, rolls)
        plt.legend(["Pitch", "Roll"])
        plt.title("Pitch and roll samples read from the STM32F407 Discovery Board")
        plt.xlabel("Timestep")
        plt.ylabel("Degrees")
        plt.savefig("static/graph.png")
#        data = np.genfromtxt(csv_file, delimiter=",", dtype=float)
#        data = np.reshape(data, (10000, 2))
#        
#        t = np.linspace(0, 10, 10000)
#        pitch = data[:10000, 0]
#        roll = data[:10000, 1]
#        
#        from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
#        from matplotlib.figure import Figure
#        fig = Figure(dpi=600)
#        axis = fig.add_subplot(1, 1, 1)
#        axis.plot(t, pitch, 'b', label="pitch")
#        axis.plot(t, roll, 'r', label="roll")
#        fig.legend()
#        
#        canvas = FigureCanvas(fig)
#        output = BytesIO()
#        canvas.print_png(output)
#        response = make_response(output.getvalue())
#        response.mimetype = 'image/png'
        return "something"

class Test(Resource):
    def put(self):
        print("Hello, Newman")
        f=open("test.csv", 'w')
        f.write("Hello, Newman\n")
        f.write("Hello, Jerry\n")
        global message
        message = request.form["message"]
        f.write(message + "\n")
        f.close()



api.add_resource(Home, '/')
api.add_resource(Accelerometer, '/accelerometer/')
api.add_resource(Speech, '/speech/')
api.add_resource(Test, '/test')

if __name__ == '__main__':
    app.run(host='0.0.0.0',debug=True)

