package com.project.ecse426.microp.server;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.ParcelUuid;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

import com.project.ecse426.microp.R;

import java.util.UUID;

/**
 * Created by Matthew
 * Activity for Server, Used for debugging. Nucleo would be running its own server application
 */
public class ServerActivity extends AppCompatActivity {

    private final String TAG = this.getClass().getName();
    private BluetoothManager mBluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;
    private BluetoothLeAdvertiser mBluetoothLeAdvertiser;
    private BluetoothGattServer mGattServer;

    private final UUID SERVICE_UUID = UUID.fromString(""); // TODO DETERMINE A NUCLEO BOARD UUID

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_server);

        mBluetoothManager = (BluetoothManager) getSystemService(BLUETOOTH_SERVICE);
        mBluetoothAdapter = mBluetoothManager.getAdapter();
    }

    // We check these in onResume to ensure the user has not disabled BLE while the app was paused.
    protected void onResume() {
        super.onResume();
        if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivity(enableBtIntent);
            finish();
            return;
        }
        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            finish();
            return;
        }
        if (!mBluetoothAdapter.isMultipleAdvertisementSupported()) {
            finish();
            return;
        }
        mBluetoothLeAdvertiser = mBluetoothAdapter.getBluetoothLeAdvertiser();
        GattServerCallback gattServerCallback = new GattServerCallback();
        mGattServer = mBluetoothManager.openGattServer(this, gattServerCallback);
        setupServer();
        startAdvertising();
    }

    private void startAdvertising() {
        if (mBluetoothLeAdvertiser == null) {
            return;
        }
        /*
        AdvertiseMode refers to how frequently the server will send out an advertising packet. Higher frequency will consume more energy, and low frequency will use less.
        TxPowerLevel deals with the broadcast range, so a higher level will have a larger area in which devices will be able to find it.
         */
        AdvertiseSettings settings = new AdvertiseSettings.Builder()
                .setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_BALANCED)
                .setConnectable(true)
                .setTimeout(0)
                .setTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_LOW)
                .build();
        ParcelUuid parcelUuid = new ParcelUuid(SERVICE_UUID);
        AdvertiseData data = new AdvertiseData.Builder()
                .setIncludeDeviceName(true)
                .addServiceUuid(parcelUuid)
                .build();
        mBluetoothLeAdvertiser.startAdvertising(settings, data, mAdvertiseCallback);
    }

    private void setupServer() {
        BluetoothGattService service = new BluetoothGattService(SERVICE_UUID,
                BluetoothGattService.SERVICE_TYPE_PRIMARY);
        mGattServer.addService(service);
    }
    private AdvertiseCallback mAdvertiseCallback = new AdvertiseCallback() {
        @Override
        public void onStartSuccess(AdvertiseSettings settingsInEffect) {
            Log.d(TAG, "Peripheral advertising started.");
        }

        @Override
        public void onStartFailure(int errorCode) {
            Log.d(TAG, "Peripheral advertising failed: " + errorCode);
        }
    };

    // If server has to pause or stop
    protected void onPause() {
        super.onPause();
        stopAdvertising();
        stopServer();
    }
    private void stopServer() {
        if (mGattServer != null) {
            mGattServer.close();
        }
    }
    private void stopAdvertising() {
        if (mBluetoothLeAdvertiser != null) {
            mBluetoothLeAdvertiser.stopAdvertising(mAdvertiseCallback);
        }
    }

    private class GattServerCallback extends BluetoothGattServerCallback {

    }
}
