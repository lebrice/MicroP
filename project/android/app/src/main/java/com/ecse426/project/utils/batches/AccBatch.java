package com.ecse426.project.utils.batches;

import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static com.ecse426.project.app.AppController.ACC_BYTES_PER_BATCH;
import static com.ecse426.project.app.AppController.ACC_SAMPLES_PER_BATCH;
import static com.ecse426.project.app.AppController.BLE_MAX_DATA_BYTES;

public class AccBatch extends Object {

    private static final String TAG = "AccBatch";
    public static boolean BIG_ENDIAN = true;

    public float[] pitch;
    public float[] roll;


    public AccBatch(float[] pitch, float[] roll) {
        this.pitch = pitch;
        this.roll = roll;
    }


    public static AccBatch fromBytes(byte[] bytes) {
        ByteBuffer buffer = ByteBuffer.allocate(BLE_MAX_DATA_BYTES);
        buffer.order(BIG_ENDIAN ? ByteOrder.BIG_ENDIAN : ByteOrder.LITTLE_ENDIAN);

        if(bytes.length >= BLE_MAX_DATA_BYTES){
            Log.e(TAG, "Too many bytes to buffer!");
        }
        // add all the bytes in source.
        buffer.put(bytes);

        // make a float array from these bytes.
        float[] tempFloats = buffer.asFloatBuffer().array();

        float[] pitch = new float[ACC_SAMPLES_PER_BATCH];
        float[] roll = new float[ACC_SAMPLES_PER_BATCH];

        for (int sourceIndex = 0, targetIndex = 0; targetIndex < ACC_SAMPLES_PER_BATCH; targetIndex++, sourceIndex += 2) {
            pitch[targetIndex] = tempFloats[sourceIndex];
            roll[targetIndex] = tempFloats[sourceIndex + 1];
        }

        return new AccBatch(pitch, roll);
    }


    @Override
    public String toString() {
        StringBuilder result = new StringBuilder();
        for (int i = 0; (i < this.pitch.length) && (i < this.roll.length); i++) {
            result.append(this.pitch[i] + "," + this.roll[i] + "\n");
        }
        return result.toString();
    }
}
