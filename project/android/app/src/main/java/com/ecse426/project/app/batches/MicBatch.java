package com.ecse426.project.app.batches;

public class MicBatch extends Object {
    public short data[];
    public static MicBatch fromBytes(byte[] bytes){
        return null;
    }

    public MicBatch(short[] values){
        this.data = values;
    }


}
