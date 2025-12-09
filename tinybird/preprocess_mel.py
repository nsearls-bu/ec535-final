import librosa
import numpy as np
import soundfile as sf
from pathlib import Path
import csv
import subprocess

SLICE_DURATION_MS = 1000            
SLICE_SAMPLES = 16000                 
SAMPLE_RATE = 16000
N_MELS = 64
N_FFT = 256
HOP_LENGTH = 128
TARGET_FRAMES = 64


# This code is a rewrite of birdnet tiny forge preprocesssing


def extract_loudest_slice(y, sr, slice_duration_ms):
    slice_n_samples = int(slice_duration_ms / 1000 * sr)
    audio_n_samples = len(y)

    # if shorter → pad
    if audio_n_samples < slice_n_samples:
        padded = np.pad(y, (0, slice_n_samples - audio_n_samples), mode="constant")
        return padded[:slice_n_samples]

    left_edge = slice_n_samples // 2
    right_edge = slice_n_samples - left_edge

    max_index = np.argmax(np.abs(y))
    start = max(max_index - left_edge, 0)
    end = min(max_index + right_edge, audio_n_samples)

    if end - start < slice_n_samples:
        if start == 0:
            end = slice_n_samples
        else:
            start = audio_n_samples - slice_n_samples
            end = audio_n_samples

    return y[start:end]


def make_mel(seg):
    S = librosa.feature.melspectrogram(
        y=seg,
        sr=SAMPLE_RATE,
        n_fft=N_FFT,
        hop_length=HOP_LENGTH,
        n_mels=N_MELS,
        fmin=0,
        fmax=SAMPLE_RATE // 2,
        power=2.0
    )

    M = librosa.power_to_db(S, ref=np.max)
    M = (M - M.min()) / (M.max() - M.min() + 1e-9)
    M = librosa.util.fix_length(M, size=TARGET_FRAMES, axis=1)

    return M.astype(np.float32)
def convert_to_wav(root):

    for species in root.iterdir():
        if not species.is_dir():
            continue

        for mp3 in species.glob("*.mp3"):
            wav = species / (mp3.stem + ".wav")  
            subprocess.run([
                "ffmpeg", "-y",       
                "-i", str(mp3),
                "-ac", "1",                 
                "-ar", str(SAMPLE_RATE),
                str(wav)
            ], check=True)

def main():
    raw = Path("audio_clips")
    out = Path("mels")
    out.mkdir(exist_ok=True)
    convert_to_wav(raw)
    csv_path = raw / "train.csv"
    with open(csv_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["file", "label"])

        for species in raw.iterdir():
            if not species.is_dir():
                continue

            label = species.name

            for wav in species.glob("*.wav"):
                y, sr = sf.read(wav)
                y = np.array(y).reshape(-1)

                # canonical format
                if sr != SAMPLE_RATE:
                    y = librosa.resample(y, orig_sr=sr, target_sr=SAMPLE_RATE)

                slice_seg = extract_loudest_slice(y, SAMPLE_RATE, SLICE_DURATION_MS)

                if len(slice_seg) < SLICE_SAMPLES:
                    continue

                mel = make_mel(slice_seg)

                outname = f"{label}_{wav.stem}.npy"
                np.save(out / outname, mel)
                writer.writerow([outname, label])

    labels = sorted([d.name for d in raw.iterdir() if d.is_dir()])
    with open("tiny_bird_labels.txt", "w") as f:
        for l in labels:
            f.write(l + "\n")

    print("Preprocessing complete.")
    print(f"Saved mels to: {out}")
    print(f"Training CSV: {csv_path}")
    print("Labels written to tiny_bird_labels.txt")


if __name__ == "__main__":
    main()
