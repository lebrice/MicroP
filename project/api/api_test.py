
import requests
from requests import put, get
import json

import scipy.io.wavfile as wav
fake_data = {
    "data": {
        "x": [1,2,3,4,5,6,7],
        "y": [1,2,3,4,5,6,7],
        "z": [1,2,3,4,5,6,7],
    }
}

fake_data = {
    "data": 3
}


import os
current_dir = ""
try:
    current_dir = os.path.dirname(__file__)
except:
    current_dir = os.getcwd()
import numpy as np
np.random.seed(123123)

image_files = os.listdir(f"{current_dir}/spectrograms/valid")
np.random.shuffle(image_files)

total = 100
test_files = image_files[:total]

print("testing with filenames:", test_files)

get_label = lambda filename : int(filename.split("_")[0])
success = 0
fail = 0

import time

start_time = time.time()
for i, file_name in enumerate(test_files):
    files = {
        "image": open(f"{current_dir}/spectrograms/valid/"+file_name, "rb")
    }
    url = 'http://localhost:5000/speech/'

    response = requests.post(url, files=files)

    true_label = get_label(file_name)

    response_data = response.json()[0]

    pred_label = response_data["classes"]
    pred_probabilities = response_data["probabilities"]


    print("File",file_name,"true Label:", true_label, "Predicted label:", pred_label, "confidence:", pred_probabilities[pred_label])

    if true_label == pred_label:
        success += 1
    else:
        fail += 1

total_time = time.time() - start_time

print(f"Succeeded: {success}/{total}, failed {fail}/{total}")
print("seconds per image: ", total_time / total)