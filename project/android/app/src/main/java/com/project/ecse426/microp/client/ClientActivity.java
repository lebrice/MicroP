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
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ProgressBar;
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
    private List<String> listAddress = new ArrayList<String>();

    private final UUID SERVICE_UUID = Utils.getServiceUuid();
    private final UUID CHARACTERISTIC_UUID = Utils.getCharacteristicUuid();
    private final UUID CONTROL_POINT_UUID = Utils.getControlPointCharUuid();
    private final UUID CLIENT_CHARACTERISTIC_CONFIG_UUID = Utils.getClientCharacteristicConfigUuid();
    private final String NUCLEO_MAC_ADDRESS = Utils.getNucleoMacAddress();
    private String selectedAddress;
    private BluetoothDevice selectedDevice;
    public static final String PREFS_NAME = "MAC_Address";
    private ProgressBar progressBar;
    private ListView listView;
    private ArrayAdapter<String> arrayAdapter;
    private TextView textAddress;
    private ToggleButton toggleConnection;
    private Button startScanButton;
    private Button stopScanButton;

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

        // Getting Views from Layout
        listView = findViewById(R.id.list_address);
        progressBar = findViewById(R.id.progress_bar);
        toggleConnection = findViewById(R.id.toggle_connection);
        startScanButton = findViewById(R.id.start_scan_button);
        stopScanButton = findViewById(R.id.stop_scan_button);
        textAddress = findViewById(R.id.device_address);

        // Hid progress bar by default
        progressBar.setVisibility(View.GONE);

        // Click Listeners for start and stop scan buttons
        startScanButton.setOnClickListener(v -> {
            startScan();
            Log.d(TAG, "Start Scan button clicked");
            progressBar.setVisibility(View.VISIBLE);
        });
        stopScanButton.setOnClickListener(v -> {
            stopScan();
            Log.d(TAG, "Stop Scan button clicked");
            progressBar.setVisibility(View.GONE);
        });

        // Populate ListView
        arrayAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, listAddress);
        listView.setAdapter(arrayAdapter);

        // On Item Click Listener for list view
        listView.setOnItemClickListener((parent, view, position, id) -> {
            selectedAddress = listAddress.get(position);
            Toast.makeText(this, "Address " + selectedAddress + " selected", Toast.LENGTH_SHORT).show();
            textAddress.setText("Device Address: " + selectedAddress);
            textAddress.invalidate();
            selectedDevice = mScanResults.get(selectedAddress);
        });

        // On Checked Listener for toggle connection button
        toggleConnection.setOnCheckedChangeListener((compoundButton, isChecked) -> {
            if (selectedDevice != null && selectedAddress != null) {
                if (isChecked) {
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
        Toast.makeText(this, "Scanning", Toast.LENGTH_SHORT).show();

        // Handler delays scanning to save power, otherwise scans forever4
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
        progressBar.setVisibility(View.GONE);
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
            if (!listAddress.contains(deviceAddress)) {
                listAddress.add(deviceAddress);
                arrayAdapter.notifyDataSetChanged();
            }
            Log.i(TAG, "=====RESULT ADDED=====");
//            if (deviceAddress == NUCLEO_MAC_ADDRESS) {
//                connectDevice(device);
//                stopScan();
//            }
            //connectDevice(device);
        }

//        public Map<String, BluetoothDevice> getmScanResults() {
//            return mScanResults;
//        }
    }

    /*
   We pass in a Context, false for autoconnect and a BluetoothGattCallback. Be careful when using autoconnect, as it could run rampant.
   An active BLE interface with uncontrolled connection attempts will drain the battery and choke up the CPU, so its best to handle reconnections manually.
    */
    private void connectDevice(BluetoothDevice device) {
        GattClientCallback gattClientCallback = new GattClientCallback();
        mGatt = device.connectGatt(this, false, gattClientCallback);
        Log.d(TAG, "=====GATT SERVER CONNECTED=====");
        Toast.makeText(this, "Connected to " + selectedAddress, Toast.LENGTH_SHORT).show();
    }

    // To fully disconnect from server
    public void disconnectGattServer() {
        mConnected = false;
        if (mGatt != null) {
            mGatt.disconnect();
            mGatt.close();
            Log.d(TAG, "=====GATT SERVER DISCONNECTED=====");
            Toast.makeText(this, "Disconnected from " + selectedAddress, Toast.LENGTH_SHORT).show();
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
