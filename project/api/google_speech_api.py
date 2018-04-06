"""

"""


import os
current_dir = ""
try:
    current_dir = os.path.dirname(__file__)
except:
    current_dir = os.getcwd()

def transcribe_file(speech_file):
    with open(speech_file, 'rb') as audio_file:
        content = audio_file.read()
        return transcribe_audio(content)

def transcribe_audio(audio_sample):
    """Transcribe the given audio file.
    
    Taken from https://cloud.google.com/speech/docs/sync-recognize#speech-sync-recognize-python
    """
    from google.cloud import speech
    from google.cloud.speech import enums
    from google.cloud.speech import types

    from google.oauth2 import service_account

    credentials = service_account.Credentials.from_service_account_file(f"{current_dir}/google_api_keys.json")

    # scoped_credentials = credentials.with_scopes(
    #     ['https://speech.googleapis.com/v1/speech '])
    # Explicitly use service account credentials by specifying the private key
    # file.
    client = speech.SpeechClient(credentials=credentials)

    # # with open(speech_file, 'rb') as audio_file:
    #     content = audio_file.read()
    content = audio_sample

    audio = types.RecognitionAudio(content=content)
    config = types.RecognitionConfig(
        encoding=enums.RecognitionConfig.AudioEncoding.LINEAR16,
        language_code='en-US')

    response = client.recognize(config, audio)
    # Each result is for a consecutive portion of the audio. Iterate through
    # them to get the transcripts for the entire audio file.
    results = []
    for result in response.results:
        # The first alternative is the most likely one for this portion.
        print('Transcript: {}'.format(result.alternatives[0].transcript))
        results.append(result.alternatives[0].transcript)
    return results[0]

def main():

    import wave
    from record import record_to_file, play_sound_file
        
    test_audio_file = f"{current_dir}/tmp/api_test.wav"
    if not os.path.isfile(test_audio_file):
        record_to_file(test_audio_file, sampling_freq=16000)
    play_sound_file(test_audio_file)
    result = transcribe_file(f"{current_dir}/tmp/api_test.wav")
    print(result)

if __name__ == '__main__':
    main()