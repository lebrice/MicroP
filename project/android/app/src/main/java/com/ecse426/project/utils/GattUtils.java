package com.ecse426.project.utils;

import java.util.UUID;

/**
 * Created by matth on 02-Apr-18.
 * This class contains Gatt Service, Characteristic and Descriptor UUIDs and the Nucleo MAC Address
 * Service UUIDs used to identify GATT service, characteristic UUIDs used to identify data we would
 * like to read/notify/write, descriptor UUIDs used to identify permissions to read/write/notify a
 * characteristic
 */

public class GattUtils {
    public final static UUID CUSTOM_SERVICE_023_UUID = UUID.fromString("02366E80-CF3A-11E1-9AB4-0002A5D5C51B");
    public final static UUID CUSTOM_SERVICE_428_UUID = UUID.fromString("42821A40-E477-11E2-82D0-0002A4D5C51B");
    public final static UUID CHARACTERISTIC1_023_UUID = UUID.fromString("E23E78A0-CF4A-11E1-8FFC-0002A5D5C51B");
    public final static UUID CHARACTERISTIC2_023_UUID = UUID.fromString("340A1B80-CF4B-11E1-AC36-0002A5D5C51B"); // Read, Notify Permissions
    public final static UUID CONTROL_POINT_CHAR_UUID = UUID.randomUUID();
    public final static UUID CLIENT_CHARACTERISTIC_CONFIG_UUID = convertFromInteger(0x2902);
    public final static String NUCLEO_MAC_ADDRESS = "03:80:E1:00:34:12";

    // If you want to convert an integer to UUID. Some generic UUIDs on the web are represented in hex, so this may be helpful to decipher them
    public static UUID convertFromInteger(int i) {

        final long MSB = 0x0000000000001000L;
        final long LSB = 0x800000805f9b34fbL;
        long value = i & 0xFFFFFFFF;
        return new UUID(MSB | (value << 32), LSB);

    }
}
