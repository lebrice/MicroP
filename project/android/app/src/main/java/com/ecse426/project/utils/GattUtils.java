package com.ecse426.project.utils;

import java.util.UUID;

/**
 * Created by matth on 02-Apr-18.
 * This class contains Service UUIDs and the Nucleo MAC Address
 */

public class GattUtils {
    private final static UUID SERVICE_UUID = convertFromInteger(0x2800); // TODO DEFINE UUID FOR NUCLEO BOARD
    private final static UUID CHARACTERISTIC_UUID = UUID.randomUUID();
    private final static UUID CONTROL_POINT_CHAR_UUID = UUID.randomUUID();
    private final static UUID CLIENT_CHARACTERISTIC_CONFIG_UUID = UUID.randomUUID();
    private final static String NUCLEO_MAC_ADDRESS = "03:80:E1:00:34:12";

    public static UUID convertFromInteger(int i) {

        final long MSB = 0x0000000000001000L;
        final long LSB = 0x800000805f9b34fbL;
        long value = i & 0xFFFFFFFF;
        return new UUID(MSB | (value << 32), LSB);

    }

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
