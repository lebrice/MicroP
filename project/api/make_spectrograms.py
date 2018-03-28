

import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt
import scipy.io.wavfile as wav
import matplotlib.image as mpimage
import skimage
from skimage.transform import resize
from io import StringIO, BytesIO

import os
current_dir = ""
try:
	current_dir = os.path.dirname(__file__)
except:
	currend_dir = os.getcwd()

SAMPLING_FREQ = 8000

def make_spectrogram_from_wav_file(audio_file_path, saved_spectrogram_path, input_size=(64,64)):
    """
    Generates a 64x64 spectrogram image from the given audio file.
    """
    #Read the audio file
    sample_rate, samples = wav.read(audio_file_path)
    
    # Set the options on pyplot such that the figure has only one box.
    


    spectrum, frequencies, times, image = plt.specgram(
        samples,
        Fs=sample_rate,
        cmap="gray"
    )
    # Create a buffer for holding the bytes of the spectrogram before it is resized.
    buffer = BytesIO()
    # Save the figure in the buffer.
    plt.savefig(buffer)
    #Read the image back
    buffer.seek(0)
    spectrogram = mpimage.imread(buffer)

    #Resize the image
    resized_image = resize(spectrogram, input_size, mode='constant')

    #Save it at the required path.
    plt.imshow(resized_image)
    mpimage.imsave(saved_spectrogram_path, resized_image)
    plt.clf()
    return resized_image


# Print iterations progress
def printProgressBar (iteration, total, prefix = '', suffix = '', decimals = 2, length = 100, fill = '█'):
    """
    TAKEN FROM https://stackoverflow.com/questions/3173320/text-progress-bar-in-the-console
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
    """
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    print('\r%s |%s| %s%% %s' % (prefix, bar, percent, suffix), end = '\r')
    # Print New Line on Complete
    if iteration == total: 
        print('\r')



audio_dir = """C:\\Users\\Fabrice\\repos\\free-spoken-digit-dataset\\recordings"""

spectrograms_dir = f"{current_dir}/spectrograms/"

wav_files = os.listdir(audio_dir)
num_files = len(wav_files)

wav_files = sorted(wav_files)
print("Creating 64x64 spectrogram images for all", num_files, "files.")

fig = plt.figure(frameon=False)
ax = plt.Axes(fig, [0., 0., 1., 1.])
ax.set_axis_off()
fig.add_axes(ax)

skipped_count = 0
for i, file_name in enumerate(wav_files):
    printProgressBar(i, num_files)
    audio_path = f"{audio_dir}/{file_name}"
    spect_path = f"{spectrograms_dir}/{file_name.replace('.wav','.png')}"

    try:
        #Check if we created the spectrogram for this file, if so, we can skip the next step.
        if not os.path.isfile(spect_path):
            make_spectrogram_from_wav_file(audio_path, spect_path)
            plt.clf()
    except:
        pass
        skipped_count += 1
        # print(f"Error in file '{audio_path}', skipping it.")

print("Created", num_files-skipped_count, "/", num_files, "spectrographs.")