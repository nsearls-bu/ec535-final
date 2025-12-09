import numpy as np
import tensorflow as tf
import librosa

MODEL_PATH = "tinybird_fp32.tflite"
LABELS_PATH = "tiny_bird_labels.txt"
SAMPLE_RATE = 16000
N_MELS = 64

WINDOW_SIZE = SAMPLE_RATE * 3
HOP_LENGTH = 128
N_FFT = 256

interpreter = tf.lite.Interpreter(model_path=MODEL_PATH)
interpreter.allocate_tensors()
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()
INPUT_INDEX = input_details[0]['index']
OUTPUT_INDEX = output_details[0]['index']

with open(LABELS_PATH, "r") as f:
    LABELS = [line.strip() for line in f]

def make_mel(segment):
    mel = librosa.feature.melspectrogram(
        y=segment,
        sr=SAMPLE_RATE,
        n_fft=N_FFT,
        hop_length=HOP_LENGTH,
        n_mels=N_MELS,
        fmin=0,
        fmax=SAMPLE_RATE // 2,
        power=2.0,
    )
    mel_db = librosa.power_to_db(mel, ref=np.max)
    mel_norm = (mel_db - mel_db.min()) / (mel_db.max() - mel_db.min() + 1e-9)
    mel_norm = librosa.util.fix_length(mel_norm, size=64, axis=1)
    return mel_norm.astype(np.float32).reshape(1, 64, 64, 1)

def predict_from_mel(mel):
    interpreter.set_tensor(INPUT_INDEX, mel)
    interpreter.invoke()
    return interpreter.get_tensor(OUTPUT_INDEX)[0]

def top_predictions(scores, n=5):
    idx = np.argsort(scores)[::-1][:n]
    return [(LABELS[i], float(scores[i])) for i in idx]

data, sr = librosa.load("soundscape.wav", sr=None)
audio = librosa.resample(data, orig_sr=sr, target_sr=SAMPLE_RATE)

HOP_SIZE = SAMPLE_RATE // 2

for window_idx in range((len(audio) - WINDOW_SIZE) // HOP_SIZE):
    start = window_idx * HOP_SIZE
    segment = audio[start:start + WINDOW_SIZE]
    mel = make_mel(segment)
    scores = predict_from_mel(mel)

    time_sec = start / SAMPLE_RATE
    print(f"\nWindow {window_idx} (t={time_sec:.1f}s)")
    print("-------------------------------------------")
    for label, score in top_predictions(scores):
        print(f"{label}: {score:.4f}")

    if window_idx == 19:
        break
