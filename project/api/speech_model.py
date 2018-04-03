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
    tf.logging.set_verbosity("INFO")

    hooks = [
        tf.train.SummarySaverHook(
            save_steps = 100,
            output_dir=classifier.model_dir,
            scaffold=tf.train.Scaffold(),
            summary_op=tf.summary.merge_all()
        )
    ]
    # tf.estimator.train_and_evaluate(
    #     classifier,
    #     train_spec=tf.estimator.TrainSpec(
    #         input_fn=train_input_fn,
    #         max_steps=2000,
    #         hooks=hooks
    #     ),
    #     eval_spec=tf.estimator.EvalSpec(
    #         input_fn=valid_input_fn,
    #         steps=1000,
    #         hooks=hooks,
    #         name="Validation",
    #         start_delay_secs=30,
    #         throttle_secs=30
    #     )   
    # )
    classifier.train(
        input_fn=train_input_fn,
        max_steps=2000,
        hooks=hooks
    )
    classifier.export_savedmodel(
        f"{current_dir}/ml_model",
        serving_input_receiver_fn=serving_input_receiver_fn()
    )
    classifier.evaluate(
        input_fn=valid_input_fn,
        steps=100,
        hooks=hooks,
        name="Validation"
    )

#  feature_spec = {
#         'x': tf.FixedLenFeature(dtype=tf.string, shape=[64,64,1])
#     }

#     serialized_tf_example = tf.placeholder(dtype=tf.string,
#                                             shape=[BATCH_SIZE],
#                                             name='input_example_tensor')
#     receiver_tensors = serialized_tf_example
#     features = tf.parse_example(serialized_tf_example, feature_spec)
#     return tf.estimator.export.ServingInputReceiver(features, receiver_tensors)


def serving_input_receiver_fn():
    """An input receiver that expects a serialized tf.Example."""
    image_feature_column = tf.feature_column.numeric_column("x", shape=(64,64,1))
    # image_feature = tf.FixedLenFeature(dtype=tf.uint8, shape=[64,64,1])

    image_spec = tf.feature_column.make_parse_example_spec([image_feature_column])
    export_input_fn = tf.estimator.export.build_parsing_serving_input_receiver_fn(image_spec)
    return export_input_fn



def train_input_fn():
    return my_input_pipeline(f"{current_dir}/spectrograms/train")

def valid_input_fn():
    return my_input_pipeline(f"{current_dir}/spectrograms/valid")


def get_dummy_model():
    x = tf.placeholder(tf.int32, name="input_x")
    x2 = tf.square(x)
    graph = tf.get_default_graph()
    return graph, x, x2

def get_model():
    classifier = tf.estimator.Estimator(
        model_fn=speech_model_function,
        model_dir=saved_model_dir,
        config=tf.estimator.RunConfig(
            save_checkpoints_steps=500,
            keep_checkpoint_max=3
        )
    )
    return classifier


def my_input_pipeline(spectrograms_dir, batch_size=BATCH_SIZE):
    with tf.name_scope("input_pipeline"):
            
        i_files = os.listdir(spectrograms_dir)
        input_files = [f"{spectrograms_dir}/{filename}" for filename in i_files]

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
        return {"x": image_batch}, label_batch

def read_image_file(filename_queue):
    with tf.name_scope("image_reader"):
        reader = tf.WholeFileReader()
        file_path, file_str = reader.read(filename_queue)

        parts = tf.string_split([file_path], delimiter="/", skip_empty=True)
        # file_name =  tf.sparse_tensor_to_dense(parts, default_value="0")
        file_name =  parts.values[-1]
        label = tf.string_split([file_name], delimiter="_", skip_empty=True)
        label = tf.string_to_number(label.values[0], tf.int32)
        
        image = tf.image.decode_png(file_str, channels=1)
        reshaped = tf.reshape(image, [64, 64])
        return tf.to_float(reshaped), label


def speech_model_function(features, labels, mode):
    """
    Model function for CNN.
    Taken and adapted from https://www.tensorflow.org/tutorials/layers
    """
    # Input Layer
    with tf.name_scope("input_layer"):
        input_layer = tf.reshape(features["x"], [-1, 64, 64, 1], name="x")
        tf.summary.image("input_image", input_layer)

    with tf.name_scope("conv_layer_1"):
        # Convolutional Layer #1
        conv1 = tf.layers.conv2d(
            inputs=input_layer,
            filters=64,
            kernel_size=[5, 5],
            padding="same",
            activation=tf.nn.relu
        )
        # Pooling Layer #1
        pool1 = tf.layers.max_pooling2d(inputs=conv1, pool_size=[2, 2], strides=2)
        tf.summary.image("out_1", pool1[...,:1])
    
    with tf.name_scope("conv_layer_2"):
        # Convolutional Layer #2 and Pooling Layer #2
        conv2 = tf.layers.conv2d(
            inputs=pool1,
            filters=128,
            kernel_size=[5, 5],
            padding="same",
            activation=tf.nn.relu
        )
        pool2 = tf.layers.max_pooling2d(inputs=conv2, pool_size=[2, 2], strides=2)
        tf.summary.image("out_2", pool2[...,:1])

    with tf.name_scope("conv_layer_3"):
        # Convolutional Layer #3 and Pooling Layer #3
        conv3 = tf.layers.conv2d(
            inputs=pool2,
            filters=128,
            kernel_size=[3, 3],
            padding="same",
            activation=tf.nn.relu
        )
        pool3 = tf.layers.max_pooling2d(
            inputs=conv3,
            pool_size=[2, 2],
            strides=2
        )
        tf.summary.image("out_3", pool3[...,:1])

    with tf.name_scope("flatten"):
        # Dense Layer
        pool3_flat = tf.reshape(pool3, [-1, 8 * 8 * 128])

    with tf.name_scope("dense_layer_1"):
        dense = tf.layers.dense(
            inputs=pool3_flat,
            units=1024,
            activation=tf.nn.relu
        )
    with tf.name_scope("dropout"):
        dropout = tf.layers.dropout(
            inputs=dense,
            rate=0.4,
            training=mode == tf.estimator.ModeKeys.TRAIN
        )

    with tf.name_scope("output_layer"):
        # Logits Layer
        logits = tf.layers.dense(inputs=dropout, units=10)
        
        classes = tf.argmax(input=logits, axis=1)
        tf.summary.histogram("classes", classes)
        probabilities = tf.nn.softmax(logits, name="softmax_tensor")
        predictions = {
            # Generate predictions (for PREDICT and EVAL mode)
            "classes": classes,
            # Add `softmax_tensor` to the graph. It is used for PREDICT and by the
            # `logging_hook`.
            "probabilities": probabilities
        }


    if mode == tf.estimator.ModeKeys.PREDICT:
        str_classes = tf.as_string(classes)
        output = tf.estimator.export.ClassificationOutput(
            scores=probabilities,
            classes=str_classes
        )

        return tf.estimator.EstimatorSpec(
            mode=mode,
            predictions=predictions,
            export_outputs={"prediction": output}
        )

    # Calculate Loss (for both TRAIN and EVAL modes)
    # loss = tf.nn.softmax_cross_entropy_with_logits(labels=labels, logits=logits)
    with tf.name_scope("Loss"):

        onehot_labels = tf.one_hot(indices=tf.reshape(labels, [-1]), depth=num_classes)
        loss = tf.losses.softmax_cross_entropy(
            onehot_labels=onehot_labels, logits=logits
        )
        tf.summary.scalar("Loss", loss)

    with tf.name_scope("Accuracy"):
        accuracy = tf.metrics.accuracy(labels=labels, predictions=predictions["classes"])
        tf.summary.scalar("accuracy", accuracy[0])

   

    # Configure the Training Op (for TRAIN mode)
    if mode == tf.estimator.ModeKeys.TRAIN:
        optimizer = tf.train.AdamOptimizer(learning_rate=0.001)
        train_op = optimizer.minimize(
            loss=loss,
            global_step=tf.train.get_global_step()
        )
        return tf.estimator.EstimatorSpec(
            mode=mode,
            loss=loss,
            train_op=train_op
        )

     # Add evaluation metrics
    eval_metric_ops = {
        "accuracy": accuracy
    }
    return tf.estimator.EstimatorSpec(
        mode=mode, loss=loss, eval_metric_ops=eval_metric_ops)


if __name__ == '__main__':
    main()
