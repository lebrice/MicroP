#ifndef _PIPELINE_H
#define _PIPELINE_H

#include <stdio.h>
#include <stdint.h>

// The delay between consecutive BLE packets.
#define BLE_DELAY_BETWEEN_PACKETS_MS 10

#define PIN(i) i##_Pin
#define PORT(i) i##_GPIO_Port

#define SET_PIN(i) HAL_GPIO_WritePin(PORT(i), PIN(i), GPIO_PIN_SET)
#define RESET_PIN(i) HAL_GPIO_WritePin(PORT(i), PIN(i), GPIO_PIN_RESET)
#define TOGGLE_PIN(i) HAL_GPIO_TogglePin(PORT(i), PIN(i))
#define READ_PIN(i) HAL_GPIO_ReadPin(PORT(i), PIN(i))


#define MIC_SAMPLE_COUNT 10000
#define ACC_SAMPLE_COUNT 1000
#define BLE_MAX_DATA_BYTES 20


#define ACC_CHANNELS 2
#define ACC_SAMPLE_BYTES 4
#define ACC_SAMPLES_PER_BATCH (BLE_MAX_DATA_BYTES / (ACC_SAMPLE_BYTES * ACC_CHANNELS))
#define ACC_BATCH_COUNT (ACC_SAMPLE_COUNT / ACC_SAMPLES_PER_BATCH)
#define ACC_BYTES_PER_BATCH (ACC_SAMPLES_PER_BATCH * ACC_SAMPLE_BYTES * ACC_CHANNELS)
#define ACC_TOTAL_BYTES (ACC_BYTES_PER_BATCH * ACC_BATCH_COUNT)

#define MIC_CHANNELS 1
#define MIC_SAMPLE_BYTES 2
#define MIC_SAMPLES_PER_BATCH (BLE_MAX_DATA_BYTES / (MIC_SAMPLE_BYTES * MIC_CHANNELS))
#define MIC_BATCH_COUNT (MIC_SAMPLE_COUNT / MIC_SAMPLES_PER_BATCH)
#define MIC_BYTES_PER_BATCH (MIC_SAMPLES_PER_BATCH * MIC_SAMPLE_BYTES * MIC_CHANNELS)
#define MIC_TOTAL_BYTES (MIC_BYTES_PER_BATCH * MIC_BATCH_COUNT)




typedef struct{
	uint16_t data[MIC_SAMPLE_COUNT];
} MicBuffer;

typedef struct {
	uint16_t data[MIC_SAMPLES_PER_BATCH];
} MicBatch;


typedef struct{
	float pitch[ACC_SAMPLE_COUNT];
	float roll[ACC_SAMPLE_COUNT];
} AccBuffer;

typedef struct {
	float pitch[ACC_SAMPLES_PER_BATCH];
	float roll[ACC_SAMPLES_PER_BATCH];
} AccBatch;


#endif
