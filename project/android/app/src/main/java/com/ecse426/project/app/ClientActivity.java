package com.ecse426.project.app;

import android.Manifest;
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
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.ecse426.project.microp.R;
import com.ecse426.project.utils.GattUtils;

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
    private String message = "message";

    private final UUID CUSTOM_SERVICE_023_UUID = GattUtils.CUSTOM_SERVICE_023_UUID;
    private final UUID CUSTOM_SERVICE_428_UUID = GattUtils.CUSTOM_SERVICE_428_UUID;
    private final UUID CHARACTERISTIC1_023_UUID = GattUtils.CHARACTERISTIC1_023_UUID;
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
    private EditText textMessage;
    private Button sendButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.client_activity);

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
        textMessage = findViewById(R.id.text_message);
        sendButton = findViewById(R.id.send_button);

        String textSet = "Nucleo MAC Address: " + NUCLEO_MAC_ADDRESS;
        textNucleoAddress.setText(textSet);
        textNucleoAddress.invalidate();

        // Hid progress bar by default
        progressBar.setVisibility(View.INVISIBLE);

        // Click Listeners for start and stop scan buttons
        startScanButton.setOnClickListener(v -> {
            startScan();
            Log.d(TAG, "Start Scan button clicked");
        });
        stopScanButton.setOnClickListener(v -> {
            stopScan();
            Log.d(TAG, "Stop Scan button clicked");
        });
        sendButton.setOnClickListener(view -> {
            message = textMessage.getText().toString();
            sendMessage(message);
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
                .setServiceUuid(new ParcelUuid(CUSTOM_SERVICE_023_UUID))
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
        if (mConnected && mInitialized) {
            return;
        }
        BluetoothGattService service = mGatt.getService(CUSTOM_SERVICE_023_UUID);
        BluetoothGattCharacteristic characteristic = service.getCharacteristic(CHARACTERISTIC1_023_UUID);
        byte[] messageBytes = new byte[0];
        // convert message to byte array
        try {
            messageBytes = message.getBytes("UTF-8");
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "Failed to convert message string to byte array");
        }
        // Send message
        characteristic.setValue(messageBytes);
        boolean messageSuccess = mGatt.writeCharacteristic(characteristic);
        if (messageSuccess) {
            Log.i(TAG, "Message sent successfully!");
        }
    }


    /**
     * Handles callbacks from the GATT server
     * This is where to get characteristics (data) from services
     * Each service has its own UUID, to get a specific category of data. Have to specify the service that contains
     * it and the characteristics
     */
    private class GattClientCallback extends BluetoothGattCallback {
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
                Log.d(TAG, "Bluetooth discover services failed");
                return;
            }
            BluetoothGattService service = gatt.getService(CUSTOM_SERVICE_023_UUID);
            BluetoothGattCharacteristic characteristic = service.getCharacteristic(CHARACTERISTIC1_023_UUID);
            characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
            // signifies that our characteristic is fully ready to use
            mInitialized = gatt.setCharacteristicNotification(characteristic, true);
            Log.i(TAG, "Bluetooth services discovered! Successfully connected!");

            // Enable notification value descriptor for the characteristic
//            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);
//            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
//            gatt.writeDescriptor(descriptor);
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
                    gatt.getService(CUSTOM_SERVICE_023_UUID)
                            .getCharacteristic(CHARACTERISTIC1_023_UUID);

            characteristic.setValue(new byte[]{1, 1});
            gatt.writeCharacteristic(characteristic);

        }

        /**
         * Reading notification from server
         * All updates from the sensor on characteristic value changes will be posted on this next callback
         * @param gatt Gatt session
         * @param characteristic Characteristic of service
         */
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
            byte[] messageBytes = characteristic.getValue();
            String messageString = null;
            try {
                messageString = new String(messageBytes, "UTF-8");
            } catch (UnsupportedEncodingException e) {
                Log.e(TAG, "Unable to convert message bytes to string");
            }
            Log.d(TAG, "Received message: " + messageString);
        }
    }
}