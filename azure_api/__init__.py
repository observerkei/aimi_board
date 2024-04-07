import os
import azure.cognitiveservices.speech as speechsdk


log_dbg = print

def voice_recognition(filename: str, lang="zh-cn"):
    # This example requires environment variables named "SPEECH_KEY" and "SPEECH_REGION"
    speech_config = speechsdk.SpeechConfig(subscription=os.environ.get('SPEECH_KEY'), region=os.environ.get('SPEECH_REGION'))
    speech_config.speech_recognition_language=lang

    audio_config = speechsdk.audio.AudioConfig(filename=filename)
    speech_recognizer = speechsdk.SpeechRecognizer(speech_config=speech_config, audio_config=audio_config)

    log_dbg("Speak into your file.")
    speech_recognition_result = speech_recognizer.recognize_once_async().get()

    if speech_recognition_result.reason == speechsdk.ResultReason.RecognizedSpeech:
        log_dbg("Recognized: {}".format(speech_recognition_result.text))
        return speech_recognition_result.text
    elif speech_recognition_result.reason == speechsdk.ResultReason.NoMatch:
        log_dbg("No speech could be recognized: {}".format(speech_recognition_result.no_match_details))
    elif speech_recognition_result.reason == speechsdk.ResultReason.Canceled:
        cancellation_details = speech_recognition_result.cancellation_details
        log_dbg("Speech Recognition canceled: {}".format(cancellation_details.reason))
        if cancellation_details.reason == speechsdk.CancellationReason.Error:
            log_dbg("Error details: {}".format(cancellation_details.error_details))
            log_dbg("Did you set the speech resource key and region values?")
    return "Error"


if __name__ == '__main__':
    SPEECH_KEY = os.environ["SPEECH_KEY"]
    SPEECH_REGION = os.environ["SPEECH_REGION"]
    if not SPEECH_KEY or not len(SPEECH_KEY):
        log_dbg("export SPEECH_KEY=your-key")
    if not SPEECH_REGION or not len(SPEECH_REGION):
        log_dbg("export SPEECH_REGION=your-region")

    voice_recognition("/tmp/record/record.wav")
