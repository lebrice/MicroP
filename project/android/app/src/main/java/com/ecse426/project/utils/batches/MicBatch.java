package com.ecse426.project.utils.batches;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import static com.ecse426.project.app.AppController.MIC_BYTES_PER_BATCH;

/**
 * Simple class for the MicBatch.
 */
public class MicBatch extends Object {
    public static boolean BIG_ENDIAN = true;

    private ByteBuffer buffer = ByteBuffer.allocate(MIC_BYTES_PER_BATCH);

    public MicBatch(){
        this.buffer.order(BIG_ENDIAN ? ByteOrder.BIG_ENDIAN : ByteOrder.LITTLE_ENDIAN);
    }

    /**
     * Factory method.
     * @param bytes a byte array to convert.
     * @return the MicBatch.
     */
    public static MicBatch fromBytes(byte[] bytes) {
        MicBatch result = new MicBatch();
        result.buffer.put(bytes);
        return result;
    }

    public byte[] toBytes(){
        return toBytes(ByteOrder.LITTLE_ENDIAN);
    }

    public byte[] toBytes(ByteOrder byteOrder){
        return this.buffer.order(byteOrder).array();
    }


    public short[] toShorts(){
        return this.buffer.asShortBuffer().array();
    }





}
