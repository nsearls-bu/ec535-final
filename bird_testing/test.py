import numpy as np
import tensorflow as tf
import librosa

MODEL_PATH = "BirdNET_GLOBAL_6K_V2.4_Model_FP16.tflite"
LABELS_PATH = "BirdNET_GLOBAL_6K_V2.4_Labels.txt"

interpreter = tf.lite.Interpreter(model_path=MODEL_PATH)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

INPUT_INDEX = input_details[0]['index']
OUTPUT_INDEX = output_details[0]['index']



with open(LABELS_PATH, "r") as f:
    LABELS = [line.strip() for line in f]

def predict(audio):
    """
    audio: numpy array shaped exactly like the BirdNET input
           typically (144000,) float32 clipped [-1,1]
    """
    audio = np.expand_dims(audio, axis=0).astype(np.float32)

    interpreter.resize_tensor_input(INPUT_INDEX, audio.shape)
    interpreter.allocate_tensors()

    interpreter.set_tensor(INPUT_INDEX, audio)
    interpreter.invoke()

    out = interpreter.get_tensor(OUTPUT_INDEX)[0]
    return out

def top_predictions(scores, n=5):
    print(scores)
    idx = np.argsort(scores)[::-1][:n]
    return [(LABELS[i], scores[i]) for i in idx]



dummy_audio = np.zeros(144000, dtype=np.float32)  # 3 seconds @ 48kHz
data , sampling_rate = librosa.load('soundscape.wav')
audio_48k = librosa.resample(data, orig_sr=22050, target_sr=48000)

data_tensor = tf.convert_to_tensor( audio_48k )

for window in range(len(data_tensor) // 144000):
    start = window * 144000
    scores = predict(data_tensor[start:start + 144000])
    print("Top results:")
    for label, score in top_predictions(scores, n=5):
        print(f"{label}: {score:.4f}")
        pass
