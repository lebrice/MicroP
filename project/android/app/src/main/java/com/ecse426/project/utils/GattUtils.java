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
    // 0x01,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b
    public final static UUID CUSTOM_SERVICE_UUID =  UUID.fromString("01366E80-CF3A-11E1-9AB4-0002A5D5C51B");

    // 0xe2,0x3e,0x78,0xa0, 0xcf,0x4a, 0x11,0xe1, 0x8f,0xfc, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
    public final static UUID CHAR_AUDIO_UUID =      UUID.fromString("E23E78A0-CF4A-11E1-8FFC-0002A5D5C51B");

    // 0x03,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
    public final static UUID CHAR_ACCEL_UUID =      UUID.fromString("03366E80-CF3A-11E1-9AB4-0002A5D5C51B");

    // 0x05,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
    public final static UUID CHAR_DIGIT_UUID =      UUID.fromString("05366E80-CF3A-11E1-9AB4-0002A5D5C51B");
    public final static UUID CLIENT_CHARACTERISTIC_CONFIG_UUID = convertFromInteger(0x2902);
    //public final static String NUCLEO_MAC_ADDRESS = "03:80:E1:00:34:12";
    public final static String NUCLEO_MAC_ADDRESS = "11:22:33:44:55:66";

    // If you want to convert an integer to UUID. Some generic UUIDs on the web are represented in hex, so this may be helpful to decipher them
    public static UUID convertFromInteger(int i) {

        final long MSB = 0x0000000000001000L;
        final long LSB = 0x800000805f9b34fbL;
        long value = i & 0xFFFFFFFF;
        return new UUID(MSB | (value << 32), LSB);

    }
}
