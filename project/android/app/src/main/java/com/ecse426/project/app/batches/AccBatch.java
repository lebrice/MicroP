package com.ecse426.project.app.batches;

public class AccBatch extends Object {
    public float[] pitch;
    public float[] roll;

    public static AccBatch fromBytes(byte[] bytes) {
        return null;
    }


    public AccBatch(float[] pitch, float[] roll){
        this.pitch = pitch;
        this.roll = roll;
    }
}
