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

    print("Finished recording.")
    

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


def record_some_test_samples(person_recording="FABRICE"):
    """
    Utility method used to record some test samples.

    TODO: change the name of the person recording so we can identify the sound clips.
    """
    import random
    import time
    rd = random.Random()

    def get_next_counts(sound_dir=SOUND_DIR):
        """
        Returns the number of samples of this type that are present in SOUND_DIR.
        """
        from collections import Counter
                
        sound_files = os.listdir(sound_dir)
        get_label = lambda filename : int(filename.split("_")[0])
        
        return Counter([get_label(f) for f in sound_files])

    next_counts = get_next_counts()

    while True:
        number_to_record = rd.randint(0,9)
        count = next_counts[number_to_record]

        print("Next Number to record:", number_to_record)
        print("Press ENTER to start recording. There is a 0.5 second delay. Enter anything else to exit.", end="\r")
        start = input() == ""
        if not start:
            break
        time.sleep(0.5)

        sample_width, audio_frames = record()
        play_sound_data(sample_width, audio_frames)
        user_input = input("\r\r\rKeep sample? (y/n) -->")
        if user_input == "y":

            save_path = f"{SOUND_DIR}/{number_to_record}_{person_recording}_{count}.wav"


            write_to_file(sample_width, audio_frames, save_path)
            next_counts[number_to_record] += 1

            print("\rSaved at ", save_path)
        elif user_input == "n":
            continue
        else:
            break

    print("DONE. Current sample counts per label: ")
    for label, count in next_counts.items():
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