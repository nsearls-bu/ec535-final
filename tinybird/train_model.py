import numpy as np
import pandas as pd
import tensorflow as tf
from pathlib import Path
from sklearn.model_selection import train_test_split
from keras.src.applications.mobilenet import _conv_block, _depthwise_conv_block
import keras
import matplotlib.pyplot as plt

IMG_H = 64
IMG_W = 64
# Author is BirdNET-TinyForge. I rewrote their pipelines.

class GaussianNoise(keras.layers.Layer):
    def __init__(self, stddev=0.03):
        super().__init__()
        self.stddev = stddev

    def call(self, x, training=None):
        if training:
            n = tf.random.normal(tf.shape(x), 0.0, self.stddev)
            return tf.clip_by_value(x + n, 0.0, 1.0)
        return x


class SpecAugment(keras.layers.Layer):
    def __init__(self, freq_mask=8, time_mask=8, n_freq=2, n_time=2):
        super().__init__()
        self.fm = freq_mask
        self.tm = time_mask
        self.nf = n_freq
        self.nt = n_time

    def call(self, x, training=None):
        if not training:
            return x

        def a(m):
            for _ in range(self.nf):
                f = tf.random.uniform([], 0, self.fm, tf.int32)
                f0 = tf.random.uniform([], 0, IMG_H - f, tf.int32)
                idx = tf.range(IMG_H)
                mask = (idx < f0) | (idx >= f0 + f)
                m *= tf.reshape(tf.cast(mask, tf.float32), (IMG_H, 1, 1))

            for _ in range(self.nt):
                t = tf.random.uniform([], 0, self.tm, tf.int32)
                t0 = tf.random.uniform([], 0, IMG_W - t, tf.int32)
                idx = tf.range(IMG_W)
                mask = (idx < t0) | (idx >= t0 + t)
                m *= tf.reshape(tf.cast(mask, tf.float32), (1, IMG_W, 1))

            return m

        return tf.map_fn(a, x)


def augmentation_pipeline():
    return keras.Sequential([
        GaussianNoise(0.03),
        SpecAugment(8, 8, 2, 2)
    ])


def mobilenet_slimmed(input_shape, num_classes, dropout=0.15):
    inp = keras.Input(shape=input_shape)
    x = augmentation_pipeline()(inp)

    x = _conv_block(x, 16, 1, kernel=(10, 4), strides=(5, 2))
    x = _depthwise_conv_block(x, 16, 1, block_id=1)
    x = _depthwise_conv_block(x, 32, 1, block_id=2)
    x = _depthwise_conv_block(x, 48, 1, block_id=3)
    x = _depthwise_conv_block(x, 64, 1, block_id=4)
    x = _depthwise_conv_block(x, 64, 1, block_id=5)

    x = keras.layers.GlobalMaxPooling2D()(x)
    x = keras.layers.Dropout(dropout)(x)
    out = keras.layers.Dense(num_classes, activation="softmax")(x)
    return keras.Model(inp, out)


def load_dataset(csv_path, mel_dir):
    df = pd.read_csv(csv_path)
    labels = sorted(df.label.unique())
    idx = {l: i for i, l in enumerate(labels)}

    train_df, val_df = train_test_split(df, test_size=0.15, stratify=df.label, random_state=42)

    def load(d):
        X, y = [], []
        for _, r in d.iterrows():
            m = np.load(mel_dir / r.file).reshape(IMG_H, IMG_W, 1)
            X.append(m)
            y.append(idx[r.label])
        return np.array(X, np.float32), keras.utils.to_categorical(y, len(labels))

    return *load(train_df), *load(val_df), labels


def class_weights(y):
    c = np.sum(y, axis=0)
    t = len(y)
    return {i: t / (len(c) * n) for i, n in enumerate(c) if n > 0}


def main():
    mel_dir = Path("mels")
    csv_path = Path("audio_clips/train.csv")

    X_train, y_train, X_val, y_val, labels = load_dataset(csv_path, mel_dir)
    cw = class_weights(y_train)

    train = tf.data.Dataset.from_tensor_slices((X_train, y_train)).shuffle(2000).batch(32).prefetch(2)
    val = tf.data.Dataset.from_tensor_slices((X_val, y_val)).batch(32).prefetch(2)

    model = mobilenet_slimmed((IMG_H, IMG_W, 1), len(labels))
    model.compile(optimizer=keras.optimizers.Adam(1e-3),
                  loss="categorical_crossentropy",
                  metrics=["accuracy"])




    h = model.fit(train,
                  validation_data=val,
                  epochs=100,
                  class_weight=cw,
                  verbose=1)

    model.save("final_model.h5")

    conv = tf.lite.TFLiteConverter.from_keras_model(model)
    tfl = conv.convert()
    open("tinybird_fp32.tflite", "wb").write(tfl)

    plt.figure(figsize=(10, 4))
    plt.plot(h.history["accuracy"])
    plt.plot(h.history["val_accuracy"])
    plt.savefig("acc.png", dpi=150)


if __name__ == "__main__":
    main()
