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

num_classes = 10

# A free dataset of 1500 examples of spoken digits.
audio_dir_1 = """C:\\Users\\Fabrice\\repos\\free-spoken-digit-dataset\\recordings\\"""
# Our own training data.
audio_dir_2 = f"{current_dir}/sounds"

audio_dirs = [
#    audio_dir_1,
    audio_dir_2
]


spectrograms_dir = f"{current_dir}/spectrograms/"

wav_files = []
for audio_dir in audio_dirs:
    wav_files.extend([audio_dir +"/"+ f_name for f_name in os.listdir(audio_dir)])
num_files = len(wav_files)

wav_files = sorted(wav_files)

fig = plt.figure(frameon=False)
def preprocess(samples: np.ndarray, sampling_rate=8000, input_size=(64,64)) -> np.ndarray:    
    """
    Creates a spectrogram for the given image, contained in "samples".
    """
    plt.set_cmap("gray_r")
    ax = plt.Axes(fig, [0., 0., 1., 1.])
    ax.set_axis_off()
    fig.add_axes(ax)

    # Set the options on pyplot such that the figure has only one box.
    spectrum, frequencies, times, image = plt.specgram(
        samples,
        Fs=sampling_rate,
        cmap="gray_r",
        noverlap=16
    )
    # Create a buffer for holding the bytes of the spectrogram before it is resized.
    buffer = BytesIO()
    
    # Save the figure in the buffer.
    plt.savefig(buffer, frameon=False, format="png")
    #Read the image back
    buffer.seek(0)
    spectrogram = mpimage.imread(buffer)

    #Resize the image
    resized_image = resize(spectrogram, input_size, mode='constant')
    resized_image = resized_image[:,:,0]
    return resized_image
   
def make_spectrogram_from_wav_file(audio_file_path, saved_spectrogram_path=None, input_size=(64,64)):
    """
    Generates a 64x64 spectrogram image from the given audio file.
    """
        
    #Read the audio file
    sample_rate, samples = wav.read(audio_file_path)

    resized_image = preprocess(samples, sample_rate, input_size)
    #Save it at the required path.    
    if saved_spectrogram_path != None:
        plt.imshow(resized_image)
        mpimage.imsave(saved_spectrogram_path, resized_image)
        plt.cla()
    
    return resized_image

get_label = lambda filename : int(os.path.split(filename)[-1].split("_")[0])

validation_ratio = 0.2

files_per_label = dict(zip(range(num_classes), [[] for _ in range(num_classes)]))

for filename in wav_files:
    label = get_label(filename)
    files_per_label[label].append(filename)

train_files = []
valid_files = []

np.random.seed(12345678)
#Create training and validation sets.
for label, files in files_per_label.items():
    # First shuffle the file names
    np.random.shuffle(files)

    # figure out where we will cut the list into valid and train set.
    num_files = len(files)
    cutoff_index = int(validation_ratio * num_files)
    
    valid_files.extend(files[:cutoff_index])
    train_files.extend(files[cutoff_index:])


np.random.shuffle(train_files)
np.random.shuffle(valid_files)

train_dir = f"{current_dir}/spectrograms/train"
valid_dir = f"{current_dir}/spectrograms/valid"

def make_spectrograms_dir(input_file_paths, output_dir):
    file_count = len(input_file_paths)
    skipped_count = 0
    created_count = 0
    for i, file_path in enumerate(input_file_paths):        
        printProgressBar(i, file_count)



        spect_path = f"{output_dir}/{os.path.split(file_path)[-1].replace('.wav','.png')}"

        try:
            #Check if we created the spectrogram for this file, if so, we can skip the next step.
            if not os.path.exists(spect_path):
                make_spectrogram_from_wav_file(file_path, spect_path)

        except KeyboardInterrupt:
            exit()
        except (wav.WavFileWarning, ValueError):
            skipped_count += 1
            # print(f"Error in file '{audio_path}', skipping it.")
        else:
            created_count += 1

    print("Created:", created_count, ", skipped: ", skipped_count, "out of ", num_files, f"spectrograms in {output_dir}")

def check_no_duplicate_files():
    train_files = set(os.listdir(train_dir))
    valid_files = set(os.listdir(valid_dir))
    # assert that there is no intersection, meaning that no files are both in training and testing directories.
    intersection = train_files.intersection(valid_files)
    if intersection != set():
        # raise RuntimeError("There is an intersection between the training and testing sets!!", intersection)

        from os import remove
        print("There is an intersection between the training and testing sets. Will remove these files:", intersection)
        for file in intersection:
            remove(train_dir + "/" + file)
            remove(valid_dir + "/" + file)
    
            



def main():
    make_spectrograms_dir(train_files, train_dir)
    make_spectrograms_dir(valid_files, valid_dir)
    check_no_duplicate_files()


if __name__ == '__main__':
    main()
