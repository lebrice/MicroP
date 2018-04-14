package com.ecse426.project.app;

/**
 * Created by matth on 09-Apr-18.
 */

import android.app.Application;
import android.media.AudioFormat;
import android.os.Build;
import android.os.Environment;
import android.support.annotation.RequiresApi;
import android.system.ErrnoException;
import android.system.Os;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;

import com.android.internal.http.multipart.FilePart;
import com.android.internal.http.multipart.MultipartEntity;
import com.android.internal.http.multipart.Part;
import com.android.volley.AuthFailureError;
import com.android.volley.DefaultRetryPolicy;
import com.android.volley.NetworkResponse;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.toolbox.ImageLoader;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.Volley;
import com.ecse426.project.utils.LruBitmapCache;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOError;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.CharBuffer;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;

import com.ecse426.project.app.batches.AccBatch;
import com.ecse426.project.app.batches.MicBatch;

import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.mime.HttpMultipartMode;
import org.apache.http.entity.mime.content.FileBody;
import org.apache.http.entity.mime.content.StringBody;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.entity.mime.MultipartEntityBuilder;
import org.apache.http.protocol.HTTP;
import org.json.JSONException;
import org.json.JSONObject;

public class AppController extends Application {
    // TODO: CHANGE ME.
    public static final String WEBSITE_ADDRESS = "128.0.0.1:5000";

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

    private boolean accFileCreated = false;
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

    private static boolean micFileCreated = false;
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
        boolean created = file.createNewFile();
        if (!created) {
            Os.remove(path);
        }
        return file;
    }

    private void createAccFile() {
        try {
            this.accFile = this.newBlankFile(ACC_FILE_PATH);
            this.accFileOutputStream = new FileOutputStream(this.accFile);
            this.accStreamWriter = new OutputStreamWriter(accFileOutputStream);
        } catch (FileNotFoundException | ErrnoException e) {
            e.printStackTrace();
            Log.e(TAG, "File creation failure!" + e.getMessage());
            e.printStackTrace();
            Log.e(TAG, "File creation failure!" + e.getMessage());
        } catch (IOException e) {
            e.printStackTrace();
            Log.e(TAG, "File creation failure!" + e.getMessage());
        }
        Log.i(TAG, "Successfully created Acc file.");
        this.accFileCreated = true;
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
        } catch (FileNotFoundException | ErrnoException e) {
            e.printStackTrace();
            Log.e(TAG, "File creation failure!" + e.getMessage());
        } catch (IOException e) {
            e.printStackTrace();
            Log.e(TAG, "File creation failure!" + e.getMessage());
        }
        Log.i(TAG, "Successfully created Mic file.");

        micFileCreated = true;
    }

    public void addAccBatch(AccBatch batch) {
        Log.i(TAG, "Received ACC Batch!  (Sample count: " + this.accSampleCount + "/" + ACC_SAMPLE_COUNT + ").");

        if (this.accSampleCount == 0) {
            if (this.accFileCreated) throw new AssertionError();
            this.createAccFile();
        }
        try {
            this.accStreamWriter.write(batch.toString());
            this.accSampleCount += ACC_SAMPLES_PER_BATCH;

            // If we have all the samples we need
            if (this.accSampleCount >= ACC_SAMPLE_COUNT) {

                // Flush the output streams
                this.closeAccFile();

                // Send the data over to the API.
                this.sendFileToWebsite(false);
            }

        } catch (IOException e) {
            e.printStackTrace();
            Log.e(TAG, "File creation failure!" + e.getMessage());
        } catch (JSONException e) {
            e.printStackTrace();
            Log.e(TAG, "Error while Attempting to send file:" + e.getMessage());
        }
    }


    private void returnSpokenDigit(int digit) {
        // TODO: Figure out a way to send this back through BLE. ATM the BLE classes are in ClientActivity.
    }


    public void addMicBatch(MicBatch batch) {
        if (micSampleCount == 0) {
            if (micFileCreated) throw new AssertionError();
            this.createMicFile();
        }
        try {
            micFileOutputStream.write(batch.toBytes());
            this.micSampleCount += MIC_SAMPLES_PER_BATCH;

            if(this.micSampleCount >= MIC_SAMPLE_COUNT){
                WaveFileTools.updateWavHeader(this.micFile);
                this.closeMicFile();

            }


        } catch (IOException e) {
            e.printStackTrace();
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


    private void sendFileToWebsite(boolean isMicRequest) throws JSONException, IOException {
        String filepath = isMicRequest ? MIC_FILE_PATH : ACC_FILE_PATH;


        // TODO: Its actually simpler, given that Volley wasn't made for 'big' files, to just send the file as Base64 encoded.

        String encodedFile = encodedFile = Base64.encodeToString(Files.readAllBytes(Paths.get(filepath)), Base64.DEFAULT);

        JSONObject requestJson = new JSONObject();
        requestJson.put("accelerometer", encodedFile);

        // Create the request.
        JsonObjectRequest jsonObjReq = new JsonObjectRequest(
                Request.Method.POST,
                isMicRequest ? MIC_ENDPOINT_URL : ACC_ENDPOINT_URL,
                requestJson,
                response -> {
                    Log.d(TAG, "SUCCESS: " + response.toString());
                    if (isMicRequest) {
                        int digit = response.optInt("result", 0);
                        this.closeMicFile();
                        this.returnSpokenDigit(digit);

                    } else {
                        this.closeAccFile();
                    }

                },
                error -> {
                    Log.e(TAG, "ERROR: " + error.getMessage());
                    if (isMicRequest) {
                        this.returnSpokenDigit(0);
                    }
                });

        // Change the default timeout.
        jsonObjReq.setRetryPolicy(new DefaultRetryPolicy(
                30 * 1000, // 30 secs.
                DefaultRetryPolicy.DEFAULT_MAX_RETRIES,
                DefaultRetryPolicy.DEFAULT_BACKOFF_MULT));

        AppController.getInstance().addToRequestQueue(jsonObjReq, AppController.TAG);
    }

    private void closeMicFile() {
        try {
            micStreamWriter.flush();
            micStreamWriter.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private void closeAccFile() {
        try {
            accStreamWriter.flush();
            accStreamWriter.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }


    // TODO figure out a way to write to wav file
    public void writeToWaveFile(byte[] array, String pathname) {

    }
}