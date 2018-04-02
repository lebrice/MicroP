package com.project.ecse426.microp.client;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
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
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelUuid;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import com.project.ecse426.microp.R;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

public class ClientActivity extends AppCompatActivity {

    // Required UUIDs
    private final String TAG = ClientActivity.this.getLocalClassName();
    private final UUID SERVICE_UUID = UUID.fromString(""); // TODO DEFINE UUID FOR NUCLEO BOARD
    private int REQUEST_ENABLE_BT = 1;
    private int REQUEST_FINE_LOCATION = 1;
    private Map<String, BluetoothDevice> mScanResults;
    private BtleScanCallback mScanCallback;
    private BluetoothLeScanner mBluetoothLeScanner;
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothManager bluetoothManager;
    private boolean mScanning;
    private Handler mHandler;
    private long SCAN_PERIOD = 10000;
    private boolean mConnected;
    private BluetoothGatt mGatt;
    private Handler mLogHandler;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device_control);

        mLogHandler = new Handler(Looper.getMainLooper());

        // Setup BLE in Activity
        bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();

        // TODO On click listeners for starting and stopping scanning
        // TODO on click listeners for connecting and disconnecting from device
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
        // TODO start the scan
        List<ScanFilter> filters = new ArrayList<>();
        ScanFilter filter = new ScanFilter.Builder()
                .setServiceUuid(new ParcelUuid(SERVICE_UUID))
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

        // Handler delays scanning to save power, otherwise scans forever
        mHandler = new Handler();
        mHandler.postDelayed(this::stopScan, SCAN_PERIOD);
    }
    
    private void stopScan() {
        if (mScanning && mBluetoothAdapter != null && mBluetoothAdapter.isEnabled() && mBluetoothLeScanner != null) {
            mBluetoothLeScanner.stopScan(mScanCallback);
            scanComplete();
        }

        mScanCallback = null;
        mScanning = false;
        mHandler = null;
    }

    // Performs any actions using the results, for now simply logs
    private void scanComplete() {
        if (mScanResults.isEmpty()) {
            return;
        }
        for (String deviceAddress : mScanResults.keySet()) {
            Log.d(TAG, "Found device: " + deviceAddress);
        }
    }

    // To check for permissions, and ask to enable them if disabled
    private boolean hasPermissions() {
        if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
            requestBluetoothEnable();
            return false;
        } else if (!hasLocationPermissions()) {
            requestLocationPermission();
            return false;
        }
        return true;
    }

    // Requesting enabling ble
    private void requestBluetoothEnable() {
        Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        Log.d(TAG, "Requested user enables Bluetooth. Try starting the scan again.");
    }
    // For location purposes
    private boolean hasLocationPermissions() {
        return checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) == PackageManager.PERMISSION_GRANTED;
    }
    private void requestLocationPermission() {
        requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, REQUEST_FINE_LOCATION);
    }

    // To fully disconnect from server
    public void disconnectGattServer() {
        mConnected = false;
        if (mGatt != null) {
            mGatt.disconnect();
            mGatt.close();
        }
    }

    /*
    We pass in a Context, false for autoconnect and a BluetoothGattCallback. Be careful when using autoconnect, as it could run rampant.
    An active BLE interface with uncontrolled connection attempts will drain the battery and choke up the CPU, so its best to handle reconnections manually.
     */
    private void connectDevice(BluetoothDevice device) {
        GattClientCallback gattClientCallback = new GattClientCallback();
        mGatt = device.connectGatt(this, false, gattClientCallback);
    }

    /**
     * Handles BLE scanning callbacks
     * When finding devices, saves each with an appropriate address
     */
    private class BtleScanCallback extends ScanCallback {

        // A scan result contains the device address and the device itself
        private Map<String, BluetoothDevice> mScanResults;

        public BtleScanCallback(Map<String, BluetoothDevice> scanResults) {
            mScanResults = scanResults;
        }

        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            addScanResult(result);
        }
        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            for (ScanResult result : results) {
                addScanResult(result);
            }
        }
        @Override
        public void onScanFailed(int errorCode) {
            Log.e(TAG, "BLE Scan Failed with code " + errorCode);
        }
        private void addScanResult(ScanResult result) {
            BluetoothDevice device = result.getDevice();
            String deviceAddress = device.getAddress();
            mScanResults.put(deviceAddress, device);
            connectDevice(device);
        }

    };

    /**
     * Handles callbacks from the GATT server
     * Will either go into a CONNECTED/DISCONNECTED STATE upon GATT_SUCCESS/GATT_FAILURE
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
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                disconnectGattServer();
            }
        }
    }
}
