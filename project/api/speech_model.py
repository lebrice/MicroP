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
num_classes = 10

import os
current_dir = ""
try:
    current_dir = os.path.dirname(__file__)
except:
    current_dir = os.getcwd()

saved_model_dir = f"{current_dir}/saved_model/"
import tensorflow as tf


BATCH_SIZE = 100


def main():
    classifier = get_model()
    classifier.train(
        input_fn=train_input_fn
    )


def train_input_fn():
    return my_input_pipeline(f"{current_dir}/spectrograms")




def get_dummy_model():
    x = tf.placeholder(tf.int32, name="input_x")
    x2 = tf.square(x)
    graph = tf.get_default_graph()
    return graph, x, x2

def get_model():
    classifier = tf.estimator.Estimator(
        model_fn=speech_model_function,
        model_dir=saved_model_dir
    )
    return classifier


def my_input_pipeline(spectrograms_dir, batch_size=BATCH_SIZE):
    i_files = os.listdir(spectrograms_dir)
    input_files = [f"{spectrograms_dir}/{filename}" for filename in i_files]

    # file_labels = [int(filename.split("_")[0]) for filename in i_files]

    # import collections
    # print("Training Samples per label:")
    # print(dict(collections.Counter(file_labels)))

    file_names = tf.constant(input_files, name="file_names")
    # f_labels = tf.constant(file_labels, name="file_labels")
    filename_queue = tf.train.string_input_producer(file_names, shuffle=False)
    images, f_labels = read_image_file(filename_queue)

    min_after_dequeue = 200
    capacity = min_after_dequeue + 3 * batch_size
    image_batch, label_batch = tf.train.shuffle_batch(
        [images, f_labels],
        batch_size=batch_size,
        capacity=capacity,
        min_after_dequeue=min_after_dequeue
    )
    return image_batch, label_batch

def read_image_file(filename_queue):
    with tf.name_scope("image_reader"):
        reader = tf.WholeFileReader()
        file_name, file_str = reader.read(filename_queue)

        parts = tf.string_split([file_name], delimiter="_", skip_empty=True)
        # label = tf.sparse_tensor_to_dense(parts, default_value="0")

        label = tf.string_to_number(parts.values[0], tf.int32)
        
        image = tf.image.decode_png(file_str)
        reshaped = tf.reshape(image, [64, 64])
        return tf.to_float(reshaped), label


def speech_model_function(features, labels, mode):
    """
    Model function for CNN.
    Taken and adapted from https://www.tensorflow.org/tutorials/layers
    """
    # Input Layer
    input_layer = tf.reshape(features, [-1, 64, 64, 1])

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

 # Convolutional Layer #2 and Pooling Layer #2
    conv3 = tf.layers.conv2d(
        inputs=pool2,
        filters=128,
        kernel_size=[3, 3],
        padding="same",
        activation=tf.nn.relu)
    pool3 = tf.layers.max_pooling2d(inputs=conv3, pool_size=[2, 2], strides=2)

    # Dense Layer
    pool3_flat = tf.reshape(pool3, [-1, 8 * 8 * 128])
    dense = tf.layers.dense(
        inputs=pool3_flat,
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
    # loss = tf.nn.softmax_cross_entropy_with_logits(labels=labels, logits=logits)

    onehot_labels = tf.one_hot(indices=tf.reshape(labels, [-1]), depth=num_classes)
    loss = tf.losses.softmax_cross_entropy(
        onehot_labels=onehot_labels, logits=logits
    )

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


if __name__ == '__main__':
    main()
