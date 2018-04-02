package com.project.ecse426.microp.client;

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
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.project.ecse426.microp.R;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

public class ClientActivity extends AppCompatActivity {

    // Required UUIDs
    private final String TAG = this.getClass().getName();
    private Map<String, BluetoothDevice> mScanResults;
    private BtleScanCallback mScanCallback;
    private BluetoothLeScanner mBluetoothLeScanner;
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothManager bluetoothManager;
    private BluetoothGatt mGatt;
    public BluetoothDevice nucleoDevice;

    private Handler mHandler;
    private long SCAN_PERIOD = 10000;
    private boolean mConnected;
    private boolean mScanning;
    private boolean mInitialized;
    private char[] bytes;
    private boolean mEchoInitialized;
    private boolean enableConnection;
    private List<String> spinnerArray = new ArrayList<String>();

    private final UUID SERVICE_UUID = Utils.getServiceUuid();
    private final UUID CHARACTERISTIC_UUID = Utils.getCharacteristicUuid();
    private final UUID CONTROL_POINT_UUID = Utils.getControlPointCharUuid();
    private final UUID CLIENT_CHARACTERISTIC_CONFIG_UUID = Utils.getClientCharacteristicConfigUuid();
    private final String NUCLEO_MAC_ADDRESS = Utils.getNucleoMacAddress();
    private String selectedAddress;
    private BluetoothDevice deviceSelected;
    public static final String PREFS_NAME = "MAC_Address";

    // ==================== LIFECYCLE ====================
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.client_activity);

        // Setup BLE in Activity
        Log.d(TAG, "BLE Setting up");
        bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();
        Log.d(TAG, "BLE Set up");

        // Fancy progress bar lmao
        final ProgressBar progressBar = findViewById(R.id.progress_bar);
        progressBar.setVisibility(View.GONE);

        // Start and stop scanning buttons
        final Button start_scan_button = findViewById(R.id.start_scan_button);
        start_scan_button.setOnClickListener(v -> {
            startScan();
            Log.d(TAG, "Start Scan button clicked");
            progressBar.setVisibility(View.VISIBLE);
        });
        final Button stop_scan_button = findViewById(R.id.stop_scan_button);
        stop_scan_button.setOnClickListener(v -> {
            stopScan();
            Log.d(TAG, "Stop Scan button clicked");
            progressBar.setVisibility(View.GONE);
        });

        // Populate spinner with addresses
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, spinnerArray);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        Spinner spinnerAddress = findViewById(R.id.address_spinner);
        spinnerAddress.setAdapter(adapter);

        // selecting item from spinner
        try {
            TextView deviceAddress = findViewById(R.id.device_address);
            Map<String, BluetoothDevice> scanResults = mScanCallback.getmScanResults();
            spinnerAddress.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                                                 @Override
                                                 public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                                                     selectedAddress = spinnerAddress.getItemAtPosition(i).toString();
                                                     deviceAddress.setText(selectedAddress);
                                                     deviceAddress.invalidate();
                                                 }

                                                 @Override
                                                 public void onNothingSelected(AdapterView<?> adapterView) {
                                                 }
                                             }
            );
            if (selectedAddress != null) {
                deviceSelected = scanResults.get(selectedAddress);
            }
            else {
                // Default address
                String address = NUCLEO_MAC_ADDRESS;
                deviceSelected = scanResults.get(address);
            }

        } catch (NullPointerException e) {
            Log.e(TAG, e.getMessage());
        }

        TextView resultsText = findViewById(R.id.results_text);
        resultsText.setText("Results Found: " + spinnerArray.size());
        resultsText.invalidate();

        ToggleButton toggleConnection = (ToggleButton) findViewById(R.id.toggle_button);
        toggleConnection.setOnCheckedChangeListener((compoundButton, isChecked) -> {
            if (isChecked && deviceSelected != null) {
                connectDevice(deviceSelected);
            }
            else {
                disconnectGattServer();
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
        spinnerArray.clear();
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
        Toast.makeText(this, "Scanning", Toast.LENGTH_SHORT);

        // Handler delays scanning to save power, otherwise scans forever
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
        Toast.makeText(this, "Scanning Stopped", Toast.LENGTH_SHORT);
    }

    // Performs any actions using the results, for now simply logs
    private void scanComplete() {
        if (mScanResults.isEmpty()) {
            return;
        }
        for (String deviceAddress : mScanResults.keySet()) {
            Log.d(TAG, "=====SCAN COMPLETE=====");
            Log.d(TAG, "Found device: " + deviceAddress);
        }
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

        // A scan result contains the device address and the device itself
        private Map<String, BluetoothDevice> mScanResults;

        public BtleScanCallback(Map<String, BluetoothDevice> scanResults) {
            mScanResults = scanResults;
        }

        @Override
        public void onScanResult(int callbackType, ScanResult result) {
            // Add filter to connect to specific MAC address
            if (result.getDevice().getAddress() == NUCLEO_MAC_ADDRESS) {
                nucleoDevice = result.getDevice();
                Log.i(TAG, "=====NUCLEO DEVICE FOUND=====");
                connectDevice(nucleoDevice);
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
            if (!spinnerArray.contains(deviceAddress)) {
                spinnerArray.add(deviceAddress);
            }
            Log.i(TAG, "=====RESULT ADDED=====");
        }

        public Map<String, BluetoothDevice> getmScanResults() {
            return mScanResults;
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

    private void sendMessage() {
        if (!mConnected || !mEchoInitialized) {
            return;
        }
        BluetoothGattService service = mGatt.getService(SERVICE_UUID);
        BluetoothGattCharacteristic characteristic = service.getCharacteristic(CHARACTERISTIC_UUID);
        //String message = mBinding.messageEditText.getText().toString();
    }


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
                gatt.discoverServices();
                Log.i(TAG, "Connected to Bluetooth Device");
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                disconnectGattServer();
            }
        }

        /**
         * Handler for when services are discovered
         * @param gatt
         * @param status
         */
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);
            if (status != BluetoothGatt.GATT_SUCCESS) {
                return;
            }
            BluetoothGattService service = gatt.getService(SERVICE_UUID);
            BluetoothGattCharacteristic characteristic = service.getCharacteristic(CHARACTERISTIC_UUID);
            characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
            mInitialized = gatt.setCharacteristicNotification(characteristic, true);

            BluetoothGattDescriptor descriptor =
                    characteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIG_UUID);

            descriptor.setValue(
                    BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);

            gatt.writeDescriptor(descriptor);
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status){

            BluetoothGattCharacteristic characteristic =
                    gatt.getService(SERVICE_UUID)
                            .getCharacteristic(CONTROL_POINT_UUID);

            characteristic.setValue(new byte[]{1, 1});
            gatt.writeCharacteristic(characteristic);

        }

        /**
         * For reading data from the gatt server
         * @param gatt
         * @param characteristic
         */
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);
            byte[] messageBytes = characteristic.getValue();
            String messageString = null;
            messageString = new String(bytes);
            Log.d(TAG, "Received message: " + messageString);
        }
    }
}
