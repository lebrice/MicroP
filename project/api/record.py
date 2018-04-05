"""


TAKEN FROM https://stackoverflow.com/questions/892199/detect-record-audio-in-python


"""


from sys import byteorder
from array import array
from struct import pack

import pyaudio
import wave
from tqdm import tqdm



import os
current_dir = ""
try:
    current_dir = os.path.dirname(__file__)
except:
    current_dir = os.getcwd()

CHUNK_SIZE = 1024
FORMAT = pyaudio.paInt16
RATE = 8000

def record():
    """
    Record a word or words from the microphone and 
    return the data as an array of signed shorts.
    """
    audio = pyaudio.PyAudio()

    RECORD_SECONDS = 1

    stream = audio.open(
        format=FORMAT,
        channels=1,
        rate=RATE,
        input=True,
        frames_per_buffer=CHUNK_SIZE
    )

    audio_frames = []
    print("Recording...")
    for _ in tqdm(range(int(RATE / CHUNK_SIZE * RECORD_SECONDS))):
        # Print a nice progress bar so its easy to follow.
        audio_frames.append(stream.read(CHUNK_SIZE))
    

    # stop Recording
    stream.stop_stream()
    stream.close()
    audio.terminate()

    sample_width = audio.get_sample_size(FORMAT)
    
    return sample_width, audio_frames

def write_to_file(sample_width, audio_frames, path):
    "Records from the microphone and outputs the resulting data to 'path'"
    waveFile = wave.open(path, 'wb')
    waveFile.setnchannels(1)
    waveFile.setsampwidth(sample_width)
    waveFile.setframerate(RATE)
    waveFile.writeframes(b''.join(audio_frames))
    waveFile.close()

SOUND_DIR = f"{current_dir}/sounds"

def record_to_file(path):
    """
    Records a one second clip of sound, then saves it as a .wav file at the given path.
    """
    sample_width, audio_frames = record()
    write_to_file(sample_width, audio_frames, path)


def play_sound_file(path):
    """
    Plays a sound file.
    """
    wf = wave.open(path, 'rb')    
    sample_width = wf.getsampwidth()

    data = wf.readframes(CHUNK_SIZE)
    audio_frames = []
    audio_frames.append(data)
    while len(data) > 0:
        data = wf.readframes(CHUNK_SIZE)
        audio_frames.append(data)

    play_sound_data(sample_width, data)


def play_sound_data(sample_width, data):
    """
    Plays some sound data.
    """
    # instantiate PyAudio (1)
    p = pyaudio.PyAudio()

    # open stream (2)
    stream = p.open(format=p.get_format_from_width(sample_width),
                    channels=1,
                    rate=RATE,
                    output=True)

    # read data
    # play stream (3)
    # while len(data) > 0:
    for frame in data:
        stream.write(frame)
    # stream.write(data)
        # data = wf.readframes(CHUNK_SIZE)

    # stop stream (4)
    stream.stop_stream()
    stream.close()

    # close PyAudio (5)
    p.terminate()


from getpass import getuser

CURRENT_USER = getuser().upper()


def record_some_test_samples(person_recording=CURRENT_USER):
    """
    Utility method used to record some test samples.
    """
    import random
    from time import sleep
    
    from collections import Counter
    rd = random.Random()

    get_label = lambda filename : int(filename.split("_")[0])

    def get_next_counts(sound_dir=SOUND_DIR):
        """
        Returns the number of samples of this type that are present in SOUND_DIR.
        """
        sound_files = os.listdir(sound_dir)
        return Counter([get_label(f) for f in sound_files])

    def get_least_represented_number(counts, expected_labels=list(range(10))):
        """
        Returns the label with the least number of training examples.
        Once all training examples have the same number of training samples, returns a random number.
        """
        # print(counts.items())
        result = min(expected_labels, key=lambda label: counts[label])
        # print("The least represented is ", result)
        return result

    counts = get_next_counts()
    
    print("Starting sample counts per label: ")
    for label, count in counts.items():
        print(f"'{label}'", ":", count)


    number_to_record = get_least_represented_number(counts)

    while True:
        count = counts[number_to_record]

        print("Next Number: \t-->\t", number_to_record, "\t<--")
        print("Press ENTER to start recording.", end="\r")
        start = input() == ""
        if not start:
            break
        sleep(0.5)

        sample_width, audio_frames = record()
        play_sound_data(sample_width, audio_frames)
        print("Re-Playing Audio...", end="\r")


        print("Keep sample? (y/n)\t", end="")
        user_input = input()
        if user_input == "y":
            save_path = f"{SOUND_DIR}/{number_to_record}_{person_recording}_{count}.wav"


            write_to_file(sample_width, audio_frames, save_path)
            counts[number_to_record] += 1

            print("\rSaved at ", save_path)
            number_to_record = get_least_represented_number(counts)
        elif user_input == "n":
            continue
        else:
            break

    print("DONE. Current sample counts per label: ")
    for label, count in counts.items():
        print(f"'{label}'", ":", count)

def main():
    record_some_test_samples()


import os
current_dir = ""
try:
    current_dir = os.path.dirname(__file__)
except:
    current_dir = os.getcwd()

if __name__ == '__main__':
    main()