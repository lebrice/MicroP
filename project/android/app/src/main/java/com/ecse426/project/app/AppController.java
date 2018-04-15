package com.ecse426.project.app;

/**
 * Created by matth on 09-Apr-18.
 */

import android.app.Application;
import android.media.AudioFormat;
import android.os.Environment;
import android.system.ErrnoException;
import android.system.Os;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.toolbox.ImageLoader;
import com.android.volley.toolbox.Volley;
import com.ecse426.project.utils.LruBitmapCache;
import com.ecse426.project.utils.WaveFileTools;
import com.ecse426.project.utils.batches.AccBatch;
import com.ecse426.project.utils.batches.MicBatch;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.nio.file.Files;
import java.nio.file.Paths;

public class AppController extends Application {
    // TODO: CHANGE ME.
    public static final String WEBSITE_ADDRESS = "12.34.56.78:5000";

    public static final String ACC_ENDPOINT_URL = WEBSITE_ADDRESS + "/accelerometer/";
    public static final String MIC_ENDPOINT_URL = WEBSITE_ADDRESS + "/speech/";


    public static final int ACC_SAMPLE_COUNT = 1000;
    public static final int MIC_SAMPLE_COUNT = 10000;
    public static final int MIC_SAMPLE_RATE = 10000;

    public static final int BLE_MAX_DATA_BYTES = 20;

    public static final int ACC_CHANNELS = 2;
    public static final int ACC_SAMPLE_BYTES = 4;
    public static final int ACC_SAMPLES_PER_BATCH = 2;
    public static final int ACC_BATCH_COUNT = ACC_SAMPLE_COUNT / ACC_SAMPLES_PER_BATCH;
    public static final int ACC_BYTES_PER_BATCH = ACC_SAMPLES_PER_BATCH * ACC_SAMPLE_BYTES * ACC_CHANNELS;
    public static final int ACC_TOTAL_BYTES = ACC_BYTES_PER_BATCH * ACC_BATCH_COUNT;

    public static final String ACC_FILE_PATH = Environment.getExternalStorageDirectory().getPath() + "/Documents/acc.csv";

    private int accSampleCount = 0;
    public int getAccSampleCount(){
        return accSampleCount;
    }


    private File accFile = null;
    private FileOutputStream accFileOutputStream;
    private OutputStreamWriter accStreamWriter;


    public static final int MIC_CHANNELS = 1;
    public static final int MIC_SAMPLE_BYTES = 2;
    public static final int MIC_SAMPLES_PER_BATCH = 10;
    public static final int MIC_BATCH_COUNT = MIC_SAMPLE_COUNT / MIC_SAMPLES_PER_BATCH;
    public static final int MIC_BYTES_PER_BATCH = MIC_SAMPLES_PER_BATCH * MIC_SAMPLE_BYTES * MIC_CHANNELS;
    public static final int MIC_TOTAL_BYTES = MIC_BYTES_PER_BATCH * MIC_BATCH_COUNT;

    public static final String MIC_FILE_PATH = Environment.getExternalStorageDirectory().getPath() + "Documents/audio.wav";

    private static int micSampleCount = 0;
    public int getMicSampleCount(){
        return micSampleCount;
    }

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


    private File newBlankFile(String path) throws IOException, ErrnoException {
        File file = new File(path);
        if(file.exists()){
            Os.remove(path);
        }
        file.createNewFile();
        return file;
    }

    private void createAccFile() {
        try {
            this.accFile = this.newBlankFile(ACC_FILE_PATH);
            this.accFileOutputStream = new FileOutputStream(this.accFile);
            this.accStreamWriter = new OutputStreamWriter(accFileOutputStream);
        } catch (ErrnoException | IOException e) {
            e.printStackTrace();
            Log.e(TAG, "File creation failure!" + e.getMessage());
        }
        Log.i(TAG, "Successfully created Acc file.");
    }

    private void createMicFile() {
        try {
            micFile = this.newBlankFile(ACC_FILE_PATH);
            micFileOutputStream = new FileOutputStream(micFile);
            micStreamWriter = new OutputStreamWriter(accFileOutputStream);
            WaveFileTools.writeWavHeader(
                    micFileOutputStream,
                    AudioFormat.CHANNEL_IN_MONO,
                    MIC_SAMPLE_RATE,
                    AudioFormat.ENCODING_PCM_16BIT
            );
        } catch (ErrnoException | IOException e) {
            e.printStackTrace();
            Log.e(TAG, "File creation failure!" + e.getMessage());
        }
        Log.i(TAG, "Successfully created Mic file.");
    }

    public boolean addAccBatch(AccBatch batch) throws IOException {
        Log.i(TAG, "Received ACC Batch!  (Sample count: " + this.accSampleCount + "/" + ACC_SAMPLE_COUNT + ").");

        if (this.accSampleCount == 0) {
            this.createAccFile();
        }
        // Write out the batch to the current acc file.
        this.accStreamWriter.write(batch.toString());

        this.accSampleCount += ACC_SAMPLES_PER_BATCH;

        // If we have all the samples we need
        if (this.accSampleCount >= ACC_SAMPLE_COUNT) {

            // Flush the output streams
            this.closeAccFile();

            // We let the Activity know that we have all the data for one sample.
            return true;
        }else {
            // We are still missing data.
            return false;
        }
    }


    /**
     * Adds a batch of data, and indicates if the
     * @param batch
     * @return
     */
    public boolean addMicBatch(MicBatch batch) throws IOException {
        Log.i(TAG, "Received ACC Batch!  (Sample count: " + this.accSampleCount + "/" + ACC_SAMPLE_COUNT + ").");

        if (micSampleCount == 0) {
            // if this is the first sample, create a new file for holding microphone data.
            this.createMicFile();
        }
        micFileOutputStream.write(batch.toBytes());
        micSampleCount += MIC_SAMPLES_PER_BATCH;

        if(micSampleCount >= MIC_SAMPLE_COUNT){
            // Flush the buffers
            this.closeMicFile();

            // Update the .wav file header, to indicate the right length.
            WaveFileTools.updateWavHeader(micFile);

            return true;
        }else{
            return false;
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
        this.getRequestQueue().add(req);
    }

    public void cancelPendingRequests(Object tag) {
        if (mRequestQueue != null) {
            mRequestQueue.cancelAll(tag);
        }
    }


    public String getEncodedFile(boolean isMicRequest) {
        String filepath = isMicRequest ? MIC_FILE_PATH : ACC_FILE_PATH;
        try {
            return Base64.encodeToString(Files.readAllBytes(Paths.get(filepath)), Base64.DEFAULT);

        } catch (IOException e) {
            e.printStackTrace();
            Log.e(TAG, "Error while retrieving encoded version of " + (isMicRequest? "Mic" : "Acc") + "file:\n" + e.getMessage());
            return "";
        }
    }


    public void closeMicFile() {
        try {
            micStreamWriter.close();
            micSampleCount = 0;
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    public void closeAccFile() {
        try {
            accStreamWriter.close();
            accSampleCount = 0;
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}