package com.project.ecse426.microp.client;

import java.util.UUID;

/**
 * Created by matth on 02-Apr-18.
 * Contains GATT service and characteristic UUIDs. Contains Nucleo board's MAC address
 */

public class Utils {
    private final static UUID SERVICE_UUID = UUID.randomUUID(); // TODO DEFINE UUID FOR NUCLEO BOARD
    private final static UUID CHARACTERISTIC_UUID = UUID.randomUUID();
    private final static UUID CONTROL_POINT_CHAR_UUID = UUID.randomUUID();
    private final static UUID CLIENT_CHARACTERISTIC_CONFIG_UUID = UUID.randomUUID();
    private final static String NUCLEO_MAC_ADDRESS = "1C:23:2C:DA:63:4E"; // TODO DEFINE MAC Address FOR NUCLEO BOARD

    public static UUID getCharacteristicUuid() {
        return CHARACTERISTIC_UUID;
    }

    public static UUID getControlPointCharUuid() {
        return CONTROL_POINT_CHAR_UUID;
    }

    public static UUID getServiceUuid() {
        return SERVICE_UUID;
    }

    public static String getNucleoMacAddress() {
        return NUCLEO_MAC_ADDRESS;
    }

    public static UUID getClientCharacteristicConfigUuid() {
        return CLIENT_CHARACTERISTIC_CONFIG_UUID;
    }
}
