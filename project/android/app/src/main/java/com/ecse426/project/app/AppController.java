package com.ecse426.project.app;

/**
 * Created by matth on 09-Apr-18.
 */

import android.app.Application;
import android.os.Environment;
import android.text.TextUtils;
import android.util.Log;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.toolbox.ImageLoader;
import com.android.volley.toolbox.Volley;
import com.ecse426.project.utils.LruBitmapCache;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;

import com.ecse426.project.app.batches.AccBatch;
import com.ecse426.project.app.batches.MicBatch;

public class AppController extends Application {
    public static final int ACC_SAMPLE_COUNT = 1000;
    public static final int MIC_SAMPLE_COUNT = 10000;
    public static final int BLE_MAX_DATA_BYTES = 20;

    public static final int ACC_CHANNELS = 2;
    public static final int ACC_SAMPLE_BYTES = 4;
    public static final int ACC_SAMPLES_PER_BATCH = 2;
    public static final int ACC_BATCH_COUNT = ACC_SAMPLE_COUNT / ACC_SAMPLES_PER_BATCH;
    public static final int ACC_BYTES_PER_BATCH = ACC_SAMPLES_PER_BATCH * ACC_SAMPLE_BYTES * ACC_CHANNELS;
    public static final int ACC_TOTAL_BYTES = ACC_BYTES_PER_BATCH * ACC_BATCH_COUNT;

    public static final String ACC_FILE_PATH = Environment.getExternalStorageDirectory().getPath() +"/Documents/acc.csv";

    private static int accSampleCount;
    private static File accFile = null;
    private static FileOutputStream accFileOutputStream;
    private static OutputStreamWriter accStreamWriter;


    public static final int MIC_CHANNELS = 1;
    public static final int MIC_SAMPLE_BYTES = 2;
    public static final int MIC_SAMPLES_PER_BATCH = 10;
    public static final int MIC_BATCH_COUNT = MIC_SAMPLE_COUNT / MIC_SAMPLES_PER_BATCH;
    public static final int MIC_BYTES_PER_BATCH = MIC_SAMPLES_PER_BATCH * MIC_SAMPLE_BYTES * MIC_CHANNELS;
    public static final int MIC_TOTAL_BYTES = MIC_BYTES_PER_BATCH * MIC_BATCH_COUNT;

    public static final String MIC_FILE_PATH = Environment.getExternalStorageDirectory().getPath() +"Documents/audio.wav";

    private static int micSampleCount;
    private static File micFile = null;
    private static FileOutputStream micFileOutputStream;
    private static OutputStreamWriter micStreamWriter;


    public static final String TAG = AppController.class
            .getSimpleName();

    private RequestQueue mRequestQueue;
    private ImageLoader mImageLoader;

    private static AppController mInstance;

    @Override
    public void onCreate() {
        super.onCreate();
        mInstance = this;

    }

    private void createFiles(){
        this.micFile = new File(MIC_FILE_PATH);
        // If file doesn't exist, create new file and add columns pitch and roll
        if (!this.accFile.exists()) {
            Log.i(TAG, "File doesn't exist, creating new one!");
            boolean success = this.accFile.createNewFile();
            if (success) {
                fileOutputStream = new FileOutputStream(this.accFile);
                streamWriter = new OutputStreamWriter(fileOutputStream);
                Log.i(TAG, "File created!");
                streamWriter.append("pitch,roll\n");
            }
            else {
                Log.e(TAG, "File creation failure!");
            }
        }
    }

    public static synchronized AppController getInstance() {
        return mInstance;
    }

    public RequestQueue getRequestQueue() {
        if (mRequestQueue == null) {
            mRequestQueue = Volley.newRequestQueue(getApplicationContext());
        }

        return mRequestQueue;
    }

    public ImageLoader getImageLoader() {
        getRequestQueue();
        if (mImageLoader == null) {
            mImageLoader = new ImageLoader(this.mRequestQueue,
                    new LruBitmapCache());
        }
        return this.mImageLoader;
    }

    public <T> void addToRequestQueue(Request<T> req, String tag) {
        // set the default tag if tag is empty
        req.setTag(TextUtils.isEmpty(tag) ? TAG : tag);
        getRequestQueue().add(req);
    }

    public <T> void addToRequestQueue(Request<T> req) {
        req.setTag(TAG);
        getRequestQueue().add(req);
    }

    public void cancelPendingRequests(Object tag) {
        if (mRequestQueue != null) {
            mRequestQueue.cancelAll(tag);
        }
    }


    public void addAccBatch(AccBatch batch) {
        Log.i(TAG, "Received ACC Batch!  (Sample count: " + this.accSampleCount+ "/" + ACC_SAMPLE_COUNT + ").");


    }

    public void addMicBatch(MicBatch batch) {
        return;
    }

    /*
        A note on serialization and deserialization
        This method expects that the serialization of the byte array is of the following format:
        [pitch_msr0, roll_msr0, pitch_msr1, roll_msr1, ...]
        Deserialization is done by assuming that the array is of even length and that a pitch
        measurement is followed by a roll measurement, followed by a pitch measurement, etc.
     */
    public void writeToAccFile(byte[] array, String pathName)
    {
        filePath = "/mnt/sdcard/Documents/acc.csv";
        key = "acc";
        AccBatch batch = AccBatch.fromBytes(array);
        AppController.getInstance().addAccBatch(batch);

        try
        {
            OutputStreamWriter streamWriter;
            // If file doesn't exist, create new file and add columns pitch and roll
            if (!this.accFile.exists()) {
                Log.i(TAG, "File doesn't exist, creating new one!");
                boolean success = this.accFile.createNewFile();
                if (success) {
                    fileOutputStream = new FileOutputStream(this.accFile);
                    streamWriter = new OutputStreamWriter(fileOutputStream);
                    Log.i(TAG, "File created!");
                    streamWriter.append("pitch,roll\n");
                }
                else {
                    Log.e(TAG, "File creation failure!");
                }
            }
            // Output stream for file and stream writer to write to file
            fileOutputStream = new FileOutputStream(this.accFile);
            streamWriter = new OutputStreamWriter(fileOutputStream);

            // Iterate through the array by 2 (pitch and roll), comma seperate the bytes and insert
            // the row to the file
            for (int i = 0; i < array.length - 1; i += 2) {
                int pitchIndex = i;
                int rollIndex = i + 1;

                String pitchMeasurement = Byte.toString(array[pitchIndex]);
                String rollMeasurement = Byte.toString(array[rollIndex]);

                // Comma separate the values and add new line at the end
                String row = pitchMeasurement + "," + rollMeasurement + "\n";
                streamWriter.append(row);
                Log.i(TAG,"Row " + row + " added.");
            }
        } catch (Exception e)
        {
            e.printStackTrace();
        }
    }


    // TODO figure out a way to write to wav file
    public void writeToWaveFile(byte[] array, String pathname) {
        filePath = "/mnt/sdcard/Documents/audio.wav";
        File file = new File(pathname);
        try {
            if (!file.exists()) {
                Log.i(TAG, "File doesn't exist, creating new one!");
                boolean success = file.createNewFile();
                if (success) {
                    Log.i(TAG, "File created!");
                }
                else {
                    Log.e(TAG, "File creation failure!");
                }
            }
//            AudioStream
//
//
//            streamWriter.append(Arrays.toString(array));
//            streamWriter.append("\n");
            Log.i(TAG, "File written to!");
        } catch (Exception e) {
            Log.e(TAG, e.getMessage());
        }
    }


}
