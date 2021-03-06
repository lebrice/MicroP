
import requests
from requests import put, get
import json

import scipy.io.wavfile as wav
from io import BytesIO, StringIO

import matplotlib.pyplot as plt
import matplotlib.image as mpimage

import os
current_dir = ""
try:
    current_dir = os.path.dirname(__file__)
except:
    current_dir = os.getcwd()
import numpy as np

SPEECH_URL = "http://localhost:5000/speech/"
ACCELEROMETER_URL = "http://localhost:5000/accelerometer/"

# np.random.seed(123123)
def send_to_speech_api(file_path, url=SPEECH_URL):
    with open(file_path, "rb") as f:
        files = {
            "audio": f
        }

        response = requests.post(url, files=files)
        response_data = response.json()
        # pred_label = response_data["classes"]
        # pred_probabilities = response_data["probabilities"]
        # return pred_label, pred_probabilities
        result = response_data["result"]
        return result

def send_to_accelerometer_api(file_path, url=ACCELEROMETER_URL):
    with open(file_path) as f:
        files = {
            "accelerometer": f
        }

        response = requests.post(url, files=files)
        # response_data = response.json()
        image = mpimage.imread(BytesIO(response.content), format="png")
        return image

def test_with_validation_images(test_count=100):
    image_files = os.listdir(f"{current_dir}/spectrograms/valid")
    np.random.shuffle(image_files)
    test_files = image_files[:test_count]

    print("testing with filenames:", test_files)

    get_label = lambda filename : int(filename.split("_")[0])
    success = 0
    fail = 0

    import time

    start_time = time.time()
    for i, file_name in enumerate(test_files):
        true_label = get_label(file_name)
        result = send_to_speech_api(f"{current_dir}/spectrograms/valid/"+file_name)

        print("File",file_name,"true Label:", true_label, "Predicted label:", result)
        # pred_label, pred_probabilities = send_to_api(f"{current_dir}/spectrograms/valid/"+file_name)
        # print("File",file_name,"true Label:", true_label, "Predicted label:", pred_label, "confidence:", pred_probabilities[pred_label])

        if true_label == result:
            success += 1
        else:
            fail += 1

    total_time = time.time() - start_time

    print(f"Succeeded: {success}/{test_count}, failed {fail}/{test_count}")
    print(f"Accuracy: {success/test_count:2.3%}")
    print(f"seconds per image: {total_time / test_count:2.3}")


def test_with_live_recording(duration=1, sampling_rate=10000):
    """
    Test the API by recording a digit using the microphone and then preprocessing it, sending the spectrogram and awaiting the result.
    """
    from time import sleep
    from make_spectrograms import make_spectrogram_from_wav_file
    from record import record_to_file, play_sound_file, record, write_to_file

    test_audio_file = f"{current_dir}/tmp/api_test.wav"
    test_img_path = f"{current_dir}/tmp/api_test.png"
    
    # do a quick countdown before recording.
    for i in reversed(range(4)):
        print(f"\rRecording in {i}", end="\r")
        sleep(1)
    
    record_to_file(test_audio_file, recording_length_secs=duration,sampling_freq=sampling_rate)
    play_sound_file(test_audio_file)
    result = send_to_speech_api(test_audio_file)
    print(result)

def test_with_recording():
    import wave
    from record import record_to_file, play_sound_file
        
    test_audio_file = f"{current_dir}/tmp/api_test.wav"
    if not os.path.isfile(test_audio_file):
        record_to_file(test_audio_file, sampling_freq=10000)
    play_sound_file(test_audio_file)
    result = send_to_speech_api(test_audio_file)
    print(result)


def test_accelerometer():
    t = np.linspace(0, 10, 10000)
    pitch = np.sin(t)
    roll = np.sin(2*t)
    stacked = np.stack([pitch, roll]).T

    # fig, ax = plt.subplots()
    # ax.fill(t, pitch, 'b', t, roll, 'r', alpha=0.3)
    # plt.show()

    test_acc_file = f"{current_dir}/tmp/api_test.csv"
    np.savetxt(test_acc_file, stacked, delimiter=",")
    image = send_to_accelerometer_api(test_acc_file, url=ACCELEROMETER_URL)
    mpimage.imsave(f"{current_dir}/tmp/api_test_acc.png", image)



    return


def main():
    # test_with_validation_images()
    # test_with_live_recording()
    test_with_live_recording(duration=1, sampling_rate=10000)
    # test_with_recording()
    # test_accelerometer()


if __name__ == '__main__':
    main()
