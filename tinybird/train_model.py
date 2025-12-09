import numpy as np
import pandas as pd
import tensorflow as tf
from pathlib import Path
from sklearn.model_selection import train_test_split
from keras.src.applications.mobilenet import _conv_block, _depthwise_conv_block
from keras.callbacks import ModelCheckpoint

import keras
import matplotlib.pyplot as plt

IMG_H = 64
IMG_W = 64

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
    def __init__(self, freq_mask=6, time_mask=6, n_freq=1, n_time=1):
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
        GaussianNoise(0.02),
        SpecAugment(6, 6, 1, 1)
    ])


def mobilenet_slimmed(input_shape, num_classes, dropout=0.25):
    inp = keras.Input(shape=input_shape)
    x = augmentation_pipeline()(inp)

    x = _conv_block(x, 32, 1, kernel=(10, 4), strides=(5, 2))
    x = _depthwise_conv_block(x, 64, 1, block_id=1)
    x = _depthwise_conv_block(x, 128, 1, block_id=2)
    x = _depthwise_conv_block(x, 128, 1, block_id=3)
    x = _depthwise_conv_block(x, 256, 1, block_id=4)

    x = keras.layers.GlobalAveragePooling2D()(x)
    x = keras.layers.Dropout(dropout)(x)
    out = keras.layers.Dense(num_classes, activation="softmax")(x)
    return keras.Model(inp, out)


def load_dataset(csv_path, mel_dir):
    df = pd.read_csv(csv_path)
    labels = sorted(df.label.unique())
    idx = {l: i for i, l in enumerate(labels)}
    
    groups = df.groupby('source_file')['label'].first()
    train_files, val_files = train_test_split(
        groups.index,
        test_size=0.15,
        stratify=groups.values,
        random_state=42
    )
    
    train_df = df[df.source_file.isin(train_files)]
    val_df = df[df.source_file.isin(val_files)]
    
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
    
    print(f"Train samples: {len(X_train)}, Val samples: {len(X_val)}")
    print(f"Classes ({len(labels)}): {labels}")
    print(f"Data range: [{X_train.min():.3f}, {X_train.max():.3f}]")
    print(f"Data mean: {X_train.mean():.3f}, std: {X_train.std():.3f}")
    print(f"Any NaN: {np.isnan(X_train).any()}")
    print(f"Train class dist: {np.sum(y_train, axis=0)}")
    print(f"Val class dist: {np.sum(y_val, axis=0)}")
    
    cw = class_weights(y_train)

    train = tf.data.Dataset.from_tensor_slices((X_train, y_train)).shuffle(2000).batch(64).prefetch(2)
    val = tf.data.Dataset.from_tensor_slices((X_val, y_val)).batch(64).prefetch(2)

    model = mobilenet_slimmed((IMG_H, IMG_W, 1), len(labels))
    model.compile(optimizer=keras.optimizers.Adam(5e-4),
                  loss="categorical_crossentropy",
                  metrics=["accuracy"])

    checkpoint = ModelCheckpoint(
        "checkpoint.h5",
        monitor="val_accuracy",
        save_best_only=True,
        save_weights_only=False,
        mode="max"
    )
    early_stopping = keras.callbacks.EarlyStopping(
        monitor="val_accuracy",
        patience=10,
        restore_best_weights=True,
        mode="max"
    )
    h = model.fit(train,
                  validation_data=val,
                  epochs=100,
                  class_weight=cw,
                  callbacks=[checkpoint,early_stopping],
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