"""
Fabrice Normandin
McGill ID 260636800

A simple CNN-based model for speech recognition.


INPUT: 64x64 spectrogram image

OUTPUT: label, representing one of : [
    "unknown": 0
    "one", 
    "two", 
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    etc.
]
"""


import os
currdir = ""
try:
	currdir = os.path.dirname(__file__)
except:
	currdir = os.getcwd()

saved_model_dir = f"{currdir}/saved_model/"
import tensorflow as tf


def get_speech_model():
    x = tf.placeholder.
    x = tf.placeholder(tf.int32, name="input_x")
    x2 = tf.square(x)
    graph = tf.get_default_graph()
    return graph, x, x2

def get_model():
    classifier = tf.estimator.Estimator(
        model_fn=speech_model_function,
        model_dir=saved_model_dir
    )

def dataset_loader():
    spectrograms_dir = f"{current_dir}/spectrograms/"
    wav_files = os.listdir(audio_dir)
    file_labels = ["_".split(filename)[0] for filename in wav_files]

    file_names = tf.constant(wav_files, name="input file names")
    file_labels = tf.constant(file_labels, name="input file labels")

    filename_queue = tf.train.string_input_producer([file_names, file_labels])

def read_spectro_image_file(filename_queue):
    reader = tf.WholeFileReader()
    key, file_str = reader.read(filename_queue)
    example, label = tf.image.decode_png(filename_queue)


    


def speech_model_function(features, labels, mode):
    """
    Model function for CNN.
    Taken and adapted from https://www.tensorflow.org/tutorials/layers
    """
    # Input Layer
    input_layer = tf.reshape(features["x"], [-1, 64, 64, 1])

    # Convolutional Layer #1
    conv1 = tf.layers.conv2d(
        inputs=input_layer,
        filters=64,
        kernel_size=[5, 5],
        padding="same",
        activation=tf.nn.relu)

    # Pooling Layer #1
    pool1 = tf.layers.max_pooling2d(inputs=conv1, pool_size=[2, 2], strides=2)

    # Convolutional Layer #2 and Pooling Layer #2
    conv2 = tf.layers.conv2d(
        inputs=pool1,
        filters=128,
        kernel_size=[5, 5],
        padding="same",
        activation=tf.nn.relu)
    pool2 = tf.layers.max_pooling2d(inputs=conv2, pool_size=[2, 2], strides=2)

    # Dense Layer
    pool2_flat = tf.reshape(pool2, [-1, 7 * 7 * 64])
    dense = tf.layers.dense(
        inputs=pool2_flat,
        units=1024,
        activation=tf.nn.relu
    )
    dropout = tf.layers.dropout(
        inputs=dense,
        rate=0.4,
        training=mode == tf.estimator.ModeKeys.TRAIN
    )

    # Logits Layer
    logits = tf.layers.dense(inputs=dropout, units=10)

    predictions = {
        # Generate predictions (for PREDICT and EVAL mode)
        "classes": tf.argmax(input=logits, axis=1),
        # Add `softmax_tensor` to the graph. It is used for PREDICT and by the
        # `logging_hook`.
        "probabilities": tf.nn.softmax(logits, name="softmax_tensor")
    }

    if mode == tf.estimator.ModeKeys.PREDICT:
        return tf.estimator.EstimatorSpec(mode=mode, predictions=predictions)

    # Calculate Loss (for both TRAIN and EVAL modes)
    loss = tf.losses.sparse_softmax_cross_entropy(labels=labels, logits=logits)

    # Configure the Training Op (for TRAIN mode)
    if mode == tf.estimator.ModeKeys.TRAIN:
        optimizer = tf.train.AdamOptimizer(learning_rate=0.001)
        train_op = optimizer.minimize(
            loss=loss,
            global_step=tf.train.get_global_step())
        return tf.estimator.EstimatorSpec(mode=mode, loss=loss, train_op=train_op)

    # Add evaluation metrics (for EVAL mode)
    eval_metric_ops = {
        "accuracy": tf.metrics.accuracy(
            labels=labels, predictions=predictions["classes"])}
    return tf.estimator.EstimatorSpec(
        mode=mode, loss=loss, eval_metric_ops=eval_metric_ops)
