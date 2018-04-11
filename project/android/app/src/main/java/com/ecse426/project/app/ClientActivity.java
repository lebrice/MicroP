package com.ecse426.project.app;

import android.Manifest;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.ParcelUuid;
import android.support.v7.app.AppCompatActivity;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;
import com.ecse426.project.microp.R;
import com.ecse426.project.utils.GattUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

public class ClientActivity extends AppCompatActivity {

    private final String TAG = this.getClass().getName();
    private Map<String, BluetoothDevice> mScanResults;
    private BtleScanCallback mScanCallback;
    private BluetoothLeScanner mBluetoothLeScanner;
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothManager bluetoothManager;
    private BluetoothGatt mGatt;

    private Handler mHandler;
    private long SCAN_PERIOD = 10000;
    private boolean mConnected;
    private boolean mScanning;
    private boolean mInitialized;
    private List<String> listAddress = new ArrayList<String>();

    private final UUID SERVICE_023_UUID = GattUtils.SERVICE_023_UUID;
    private final UUID SERVICE_428_UUID = GattUtils.SERVICE_428_UUID;
    private final UUID CHAR_AUDIO_023_UUID = GattUtils.CHAR_AUDIO_023_UUID;
    private final UUID CHAR_ACCEL_023_UUID = GattUtils.CHAR_ACCEL_023_UUID;
    private final UUID CONTROL_POINT_UUID = GattUtils.CONTROL_POINT_CHAR_UUID;
    private final UUID CLIENT_CHARACTERISTIC_CONFIG_UUID = GattUtils.CLIENT_CHARACTERISTIC_CONFIG_UUID;
    private final String NUCLEO_MAC_ADDRESS = GattUtils.NUCLEO_MAC_ADDRESS;

    private String selectedAddress;
    private BluetoothDevice selectedDevice;
    private ProgressBar progressBar;
    private ListView listView;
    private ArrayAdapter<String> arrayAdapter;
    private TextView textAddress;
    private ToggleButton toggleConnection;
    private Button startScanButton;
    private Button stopScanButton;
    private TextView textNucleoAddress;
//    private EditText textMessage;
    private Button sendButton;
    private Button uploadButton;

    // Volley connectivity
    private RequestQueue queue;
    private String url = "http://httpbin.org/post";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.client_activity);
        Context mContext = getApplicationContext();
        queue = Volley.newRequestQueue(mContext);

        // Setup BLE in Activity
        Log.d(TAG, "BLE Setting up");
        bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager != null ? bluetoothManager.getAdapter() : null;
        Log.d(TAG, "BLE Set up");

        // Getting Views from Layout
        listView = findViewById(R.id.list_address);
        progressBar = findViewById(R.id.progress_bar);
        toggleConnection = findViewById(R.id.toggle_connection);
        startScanButton = findViewById(R.id.start_scan_button);
        stopScanButton = findViewById(R.id.stop_scan_button);
        textAddress = findViewById(R.id.device_address);
        textNucleoAddress = findViewById(R.id.nucleo_address);
        sendButton = findViewById(R.id.send_button);
        uploadButton = findViewById(R.id.upload_button);

        String textSet = "Nucleo MAC Address: " + NUCLEO_MAC_ADDRESS;
        textNucleoAddress.setText(textSet);
        textNucleoAddress.invalidate();

        // Hid progress bar by default
        progressBar.setVisibility(View.INVISIBLE);

        // Check for permissions
        hasPermissions();

        // Click Listeners for start and stop scan buttons
        startScanButton.setOnClickListener(v -> {
            startScan();
            Log.d(TAG, "Start Scan button clicked");
        });
        stopScanButton.setOnClickListener(v -> {
            stopScan();
            Log.d(TAG, "Stop Scan button clicked");
        });

        // Populate ListView
        arrayAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, listAddress);
        listView.setAdapter(arrayAdapter);

        // On Item Click Listener for list view
        listView.setOnItemClickListener((parent, view, position, id) -> {
            selectedAddress = listAddress.get(position);
            Toast.makeText(this, "Address " + selectedAddress + " selected", Toast.LENGTH_SHORT).show();
            String deviceText = "Device MAC Address: " + selectedAddress;
            textAddress.setText(deviceText);
            textAddress.invalidate();
            selectedDevice = mScanResults.get(selectedAddress);
        });

        // On Checked Listener for toggle connection button
        toggleConnection.setOnCheckedChangeListener((compoundButton, isChecked) -> {
            if (selectedDevice != null && selectedAddress != null) {
                if (isChecked) {
                    String deviceText = "Device MAC Address: " + selectedAddress;
                    textAddress.setText(deviceText);
                    textAddress.invalidate();
                    connectDevice(selectedDevice);
                }
                else {
                    disconnectGattServer();
                }
            }
            else {
                Toast.makeText(this, "Need to select address", Toast.LENGTH_SHORT).show();
            }
        });

        String testData = "no_data";
        String key = "audio";
        String filePath = "/mnt/sdcard/Documents/test.txt";
        uploadButton.setOnClickListener(view -> httpPostStringWeb(url, key, testData));
    }

    protected void onResume() {
        // Enable BLE if not Enabled
        super.onResume();
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            finish();
        }
    }

    private void startScan() {
        if (!hasPermissions() || mScanning) {
            return;
        }
        Log.d(TAG, "=====SCAN STARTED=====");
        listAddress.clear();
        List<ScanFilter> filters = new ArrayList<>();
        ScanFilter filter = new ScanFilter.Builder()
                .setServiceUuid(new ParcelUuid(SERVICE_023_UUID))
                .build();
        ScanSettings settings = new ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_POWER)
                .build();
        // Results of scan
        mScanResults = new HashMap<>();
        mScanCallback = new BtleScanCallback(mScanResults);

        // Start of scan
        mBluetoothLeScanner = mBluetoothAdapter.getBluetoothLeScanner();
        mBluetoothLeScanner.startScan(filters, settings, mScanCallback);
        mScanning = true;
        Toast.makeText(this, "Scanning", Toast.LENGTH_SHORT).show();
        progressBar.setVisibility(View.VISIBLE);

        // Handler stops the scan after 10 seconds in order to save power
        mHandler = new Handler();
        mHandler.postDelayed(this::stopScan, SCAN_PERIOD);
    }

    private void stopScan() {
        if (mScanning && mBluetoothAdapter != null && mBluetoothAdapter.isEnabled() && mBluetoothLeScanner != null) {
            mBluetoothLeScanner.stopScan(mScanCallback);
            scanComplete();
        }
        Log.d(TAG, "=====SCAN STOPPED=====");
        Log.d(TAG, "Scan Stopped!");
        mScanCallback = null;
        mScanning = false;
        mHandler = null;
        Toast.makeText(this, "Scanning Stopped", Toast.LENGTH_SHORT).show();
        progressBar.setVisibility(View.INVISIBLE);
    }

    // Performs any actions using the results, for now simply logs
    private void scanComplete() {
        if (mScanResults.isEmpty()) {
            return;
        }
        for (String deviceAddress : mScanResults.keySet()) {
            Log.d(TAG, "Found device: " + deviceAddress);
        }
        Log.d(TAG, "=====SCAN COMPLETE=====");
    }

    // To check for permissions, and ask to enable them if disabled
    private boolean hasPermissions() {
        if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
            Log.d(TAG, "=====PERMISSIONS REQUIRED=====");
            requestBluetoothEnable();
            return false;
        } else if (!hasLocationPermissions()) {
            Log.d(TAG, "=====PERMISSIONS REQUIRED=====");
            requestLocationPermission();
            return false;
        }
        return true;
    }
    private void requestBluetoothEnable() {
        Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        int REQUEST_ENABLE_BT = 1;
        startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        Log.d(TAG, "Requested user enables Bluetooth. Try starting the scan again.");
    }
    private boolean hasLocationPermissions() {
        return checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED;
    }
    private void requestLocationPermission() {
        int REQUEST_FINE_LOCATION = 1;
        requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, REQUEST_FINE_LOCATION);
    }

    /*
   We pass in a Context, false for autoconnect and a BluetoothGattCallback. Be careful when using autoconnect, as it could run rampant.
   An active BLE interface with uncontrolled connection attempts will drain the battery and choke up the CPU, so its best to handle reconnections manually.
    */
    private void connectDevice(BluetoothDevice device) {
        GattClientCallback gattClientCallback = new GattClientCallback();
        mGatt = device.connectGatt(this, false, gattClientCallback);
        Log.d(TAG, "=====GATT SERVER CONNECTED=====");
        Toast.makeText(this, "Connecting to " + selectedAddress, Toast.LENGTH_SHORT).show();
    }

    // To fully disconnect from server
    public void disconnectGattServer() {
        mConnected = false;
        if (mGatt != null) {
            mGatt.disconnect();
            mGatt.close();
            Log.d(TAG, "=====GATT SERVER DISCONNECTED=====");
        }
    }

    private void sendMessage(String message) {
        if (!mConnected && !mInitialized) {
            return;
        }
        BluetoothGattService service = mGatt.getService(SERVICE_023_UUID);
        BluetoothGattCharacteristic characteristic = service.getCharacteristic(CHAR_AUDIO_023_UUID);
        byte[] messageBytes = new byte[0];
        // convert message to byte array
        try {
            messageBytes = message.getBytes("UTF-8");
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "Failed to convert message string to byte array");
        }
        // Send message by writing to the characteristic
        characteristic.setValue(messageBytes);
        boolean messageSuccess = mGatt.writeCharacteristic(characteristic);
        if (messageSuccess) {
            Log.i(TAG, "=====Message sent successfully!=====");
        }
        else  {
            Log.i(TAG, "=====Message failed!=====");
        }
    }

    // Sending raw string
    private void httpPostStringWeb(String url, String key, String data){
        ProgressDialog pDialog = new ProgressDialog(this);
        pDialog.setMessage("Sending...");
        pDialog.show();
        StringRequest stringRequest = new StringRequest(Request.Method.POST, url, response -> {
            // Log the first 500 characters of the response string
            Log.i(TAG, response);
            pDialog.hide();
        }, error -> {
            // Log error
            Log.e(TAG, "Request failure!");
            pDialog.hide();
        }) {
            @Override
            protected Map<String, String> getParams() {
                Map<String, String> params = new HashMap<>();
                params.put(key,data);
                return params;
            }
        };
        AppController.getInstance().addToRequestQueue(stringRequest, AppController.TAG);
    }

    // Sending Json Object
    private void httpPostJsonWeb(String url, String key, String data){
        ProgressDialog pDialog = new ProgressDialog(this);
        pDialog.setMessage("Sending...");
        pDialog.show();
        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST, url, null,
                response -> {
                    Log.d(TAG, response.toString());
                    pDialog.hide();
            }, error -> {
                Log.e(TAG, error.getMessage());
                pDialog.hide();
        }){
            @Override
            protected Map<String, String> getParams() {
                Map<String, String> params = new HashMap<>();
                params.put(key, data);
                return params;
            }
        };
        AppController.getInstance().addToRequestQueue(jsonObjReq, AppController.TAG);
    }

//    private void httpPostFileWeb(String url, String key, String filePath) {
//        File file = new File(filePath);
//        ProgressDialog pDialog = new ProgressDialog(this);
//        pDialog.setMessage("Sending...");
//        pDialog.show();
//        MultipartRequest request = new MultipartRequest(url, null, null, file, response -> {
//            Log.i(TAG, response.toString());
//            pDialog.hide();
//        }, error -> {
//            Log.e(TAG, error.getMessage());
//            pDialog.hide();
//        });
//        AppController.getInstance().addToRequestQueue(request, AppController.TAG);
//    }

    // Used for writing to file
    public void writeToFile(byte[] array, String pathName)
    {
        File file = new File(pathName);
        try
        {
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
            FileOutputStream stream = new FileOutputStream(pathName);
            stream.write(array);
            Log.i(TAG, "File written to!");
        } catch (Exception e)
        {
            e.printStackTrace();
        }
    }


    /**
     * Handles BLE scanning callbacks
     * When scanning for devices
     */
    private class BtleScanCallback extends ScanCallback {

        BtleScanCallback(Map<String, BluetoothDevice> scanResults) {
            mScanResults = scanResults;
        }

        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            String deviceAddress = device.getAddress();
            if (deviceAddress.equals(NUCLEO_MAC_ADDRESS)) {
                Log.i(TAG, "=====FOUND NUCLEO DEVICE=====");
                Log.i(TAG, "Connecting automatically to nucleo device");
                stopScan();
                selectedAddress = deviceAddress;
                selectedDevice = device;
                toggleConnection.setChecked(true);
            }
            addScanResult(result);
        }

        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            Log.i(TAG, "=====MULTIPLE DEVICES FOUND=====");
            for (ScanResult result : results) {
                addScanResult(result);
            }
        }

        @Override
        public void onScanFailed(int errorCode) {
            Log.e(TAG, "=====SCAN FAILED=====");
            Log.e(TAG, "BLE Scan Failed with code " + errorCode);
        }

        private void addScanResult(ScanResult result) {
            BluetoothDevice device = result.getDevice();
            String deviceAddress = device.getAddress();
            mScanResults.put(deviceAddress, device);
            if (!listAddress.contains(deviceAddress)) {
                listAddress.add(deviceAddress);
                arrayAdapter.notifyDataSetChanged();
            }
            Log.i(TAG, "=====RESULT ADDED=====");
            Log.i(TAG, "Device Address: " + deviceAddress);
        }
    }

    /**
     * Handles callbacks from the GATT server
     * This is where to get characteristics (data) from services
     * Each service has its own UUID, to get a specific category of data. Have to specify the service that contains
     * it and the characteristics
     */
    private class GattClientCallback extends BluetoothGattCallback {
        private String key = "audio"; //audio be default
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);
            // Any other status than Success should be handled as an error and the client should be disconnected.
            if (status == BluetoothGatt.GATT_FAILURE) {
                disconnectGattServer();
                return;
            } else if (status != BluetoothGatt.GATT_SUCCESS) {
                disconnectGattServer();
                return;
            }
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                mConnected = true;
                gatt.discoverServices();
                Log.i(TAG, "=====Connected to Bluetooth Device=====");
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                disconnectGattServer();
            }
        }

        /**
         * Handler for discovered services
         * @param gatt Gatt session
         * @param status Status
         */
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.d(TAG, "=====Bluetooth discover services failed=====");
                return;
            }

            Log.i(TAG, "=====Bluetooth GATT Success!=====");
            BluetoothGattService service = gatt.getService(SERVICE_023_UUID);
            for (BluetoothGattCharacteristic characteristic :service.getCharacteristics()) {
                Log.i(TAG, "Found characteristic: " + characteristic);
            }

            BluetoothGattCharacteristic audioCharacteristic = service.getCharacteristic(CHAR_AUDIO_023_UUID);
            audioCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);

            // Signifies that our audioCharacteristic is fully ready to use
            mInitialized = gatt.setCharacteristicNotification(audioCharacteristic, true);
            Log.i(TAG, "=====Bluetooth services discovered! Successfully connected!=====");

            // Enable notification value descriptor for the audioCharacteristic
            BluetoothGattDescriptor descriptor = audioCharacteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            gatt.writeDescriptor(descriptor);
        }

//        /**
//         *
//         * @param gatt
//         * @param characteristic
//         */
//        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
//            super.onCharacteristicChanged(gatt, characteristic);
//            byte[] messageBytes = characteristic.getValue();
//            String messageString = null;
//            messageString = new String(bytes);
//        }

        /**
         * Writing to descriptor of characteristic. Need to write to Characteristic to tell the sensor to start
         * streaming data. Write a simple byte array that contains {1,1} that serves as a data streaming command.
         * @param gatt Gatt session
         * @param descriptor Descriptor written to
         * @param status Status
         */
        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status){

            BluetoothGattCharacteristic characteristic =
                    gatt.getService(SERVICE_023_UUID)
                            .getCharacteristic(CHAR_AUDIO_023_UUID);

            characteristic.setValue(new byte[]{1, 1});
            gatt.writeCharacteristic(characteristic);

        }

        /**
         * Reading notification from server
         * All updates from the sensor on characteristic value changes will be posted on this next callback
         * @param gatt Gatt session
         * @param characteristic Characteristic of service
         */
        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
            byte[] messageBytes = characteristic.getValue();
            String messageString = null;
            String key = null;
            String filePath = null;
            String encoding = null;

            if (characteristic.getUuid().equals(CHAR_AUDIO_023_UUID)) {
                filePath = "/mnt/sdcard/Documents/audio.wav";
                key = "audio";
                writeToFile(messageBytes, filePath);
            }
            else if (characteristic.getUuid().equals(CHAR_ACCEL_023_UUID)) {
                filePath = "/mnt/sdcard/Documents/accel.csv";
                key = "accel";
                writeToFile(messageBytes, filePath);
            }
            else {
                Log.e(TAG, "Unrecognized characteristic!");
            }

            try {
                messageString = new String(messageBytes, "UTF-8");
                encoding = Base64.encodeToString(messageBytes, Base64.DEFAULT);
            } catch (UnsupportedEncodingException e) {
                Log.e(TAG, "Unable to convert message bytes to string");
            }
            Log.d(TAG, "Received message: " + messageString);

            Log.d(TAG, "Sending to web server!");
            //httpPostFileWeb(url, key, filePath);
            httpPostJsonWeb(url, key, encoding);
        }


    }
}