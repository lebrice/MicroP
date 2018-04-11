/**
  ******************************************************************************
  * @file    main.c 
  * @author  CL
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   This application contains an example which shows how implementing
  *          a proprietary Bluetooth Low Energy profile: the sensor profile.
  *          The communication is done using a Nucleo board and a Smartphone
  *          with BTLE.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cube_hal.h"

#include "osal.h"
#include "sensor_service.h"
#include "debug.h"
#include "stm32_bluenrg_ble.h"
#include "bluenrg_utils.h"
#include "stm32f4xx_hal.h"

#include "main.h"

#include <string.h>
#include <stdio.h>


/* Private defines -----------------------------------------------------------*/
#define BDADDR_SIZE 6

/**
 * @}
 */
 

/** @defgroup MAIN_Private_Variables
 * @{
 */
/* Private variables ---------------------------------------------------------*/
extern volatile uint8_t set_connectable;
extern volatile int connected;
extern AxesRaw_t axes_data;


extern uint16_t accCharHandle, customAccServHandle, customAccCharHandle, customVoiceServHandle;


uint8_t bnrg_expansion_board = IDB04A1; /* at startup, suppose the X-NUCLEO-IDB04A1 is used */

void UART_Init(void);
void GPIO_Init(void);
UART_HandleTypeDef huart2;

#define MIC_SAMPLE_COUNT 16000
#define ACC_SAMPLE_COUNT 10000
#define VOICE_DATA_SAMPLE_LENGTH 16000
#define BLUETOOTH_BATCH_SIZE 200

typedef struct {
	float pitch[ACC_SAMPLE_COUNT];
	float roll[ACC_SAMPLE_COUNT];
} AccData;

typedef struct{
	uint16_t data[VOICE_DATA_SAMPLE_LENGTH];
} MicData;



void UART_Receiver(void);

/**
 * @}
 */

/** @defgroup MAIN_Private_Function_Prototypes
 * @{
 */
/* Private function prototypes -----------------------------------------------*/
void User_Process(AxesRaw_t* p_axes);
/**
 * @}
 */

/**
 * @brief  Main function to show how to use the BlueNRG Bluetooth Low Energy
 *         expansion board to send data from a Nucleo board to a smartphone
 *         with the support BLE and the "BlueNRG" app freely available on both
 *         GooglePlay and iTunes.
 *         The URL to the iTunes for the "BlueNRG" app is
 *         http://itunes.apple.com/app/bluenrg/id705873549?uo=5
 *         The URL to the GooglePlay is
 *         https://play.google.com/store/apps/details?id=com.st.bluenrg
 *         The source code of the "BlueNRG" app, both for iOS and Android, is
 *         freely downloadable from the developer website at
 *         http://software.g-maps.it/
 *         The board will act as Server-Peripheral.
 *
 *         After connection has been established:
 *          - by pressing the USER button on the board, the cube showed by
 *            the app on the smartphone will rotate.
 *          
 *         The communication is done using a vendor specific profile.
 *
 * @param  None
 * @retval None
 */
int main(void)
{
  const char *name = "B123NRG";
  uint8_t SERVER_BDADDR[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x03};
  uint8_t bdaddr[BDADDR_SIZE];
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  
  uint8_t  hwVersion;
  uint16_t fwVersion;
  
  int ret;  
  
  /* STM32Cube HAL library initialization:
   *  - Configure the Flash prefetch, Flash preread and Buffer caches
   *  - Systick timer is configured by default as source of time base, but user 
   *    can eventually implement his proper time base source (a general purpose 
   *    timer for example or other time source), keeping in mind that Time base 
   *    duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
   *    handled in milliseconds basis.
   *  - Low Level Initialization
   */
  HAL_Init();
  
#if NEW_SERVICES
  /* Configure LED2 */
  BSP_LED_Init(LED2); 
#endif
  
  /* Configure the User Button in GPIO Mode */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
  
  /* Configure the system clock */
	/* SYSTEM CLOCK = 32 MHz */
  SystemClock_Config();

  /* Initialize the BlueNRG SPI driver */
  BNRG_SPI_Init();
  
  /* Initialize the BlueNRG HCI */
  HCI_Init();
	UART_Init();

  /* Reset BlueNRG hardware */
  BlueNRG_RST();
    
  /* get the BlueNRG HW and FW versions */
  getBlueNRGVersion(&hwVersion, &fwVersion);

  /* 
   * Reset BlueNRG again otherwise we won't
   * be able to change its MAC address.
   * aci_hal_write_config_data() must be the first
   * command after reset otherwise it will fail.
   */
  BlueNRG_RST();
  
  PRINTF("HWver %d, FWver %d", hwVersion, fwVersion);
	PRINTF("\n\n");
  
  if (hwVersion > 0x30) { /* X-NUCLEO-IDB05A1 expansion board is used */
    bnrg_expansion_board = IDB05A1; 
    /*
     * Change the MAC address to avoid issues with Android cache:
     * if different boards have the same MAC address, Android
     * applications unless you restart Bluetooth on tablet/phone
     */
    SERVER_BDADDR[5] = 0x02;
  }

  /* The Nucleo board must be configured as SERVER */
  Osal_MemCpy(bdaddr, SERVER_BDADDR, sizeof(SERVER_BDADDR));
  
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                  CONFIG_DATA_PUBADDR_LEN,
                                  bdaddr);
  if(ret){
    PRINTF("Setting BD_ADDR failed.\n");
  }
  
  ret = aci_gatt_init();    
  if(ret){
    PRINTF("GATT_Init failed.\n");
  }

  if (bnrg_expansion_board == IDB05A1) {
    ret = aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x03, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  }
  else {
    ret = aci_gap_init_IDB04A1(GAP_PERIPHERAL_ROLE_IDB04A1, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  }

  if(ret != BLE_STATUS_SUCCESS){
    PRINTF("GAP_Init failed.\n");
  }

  ret = aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0,
                                   strlen(name), (uint8_t *)name);

  if(ret){
    PRINTF("aci_gatt_update_char_value failed.\n");            
    while(1);
  }
  
  ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
                                     OOB_AUTH_DATA_ABSENT,
                                     NULL,
                                     7,
                                     16,
                                     USE_FIXED_PIN_FOR_PAIRING,
                                     123456,
                                     BONDING);
  if (ret == BLE_STATUS_SUCCESS) {
    PRINTF("BLE Stack Initialized.\n");
  }
  
  PRINTF("SERVER: BLE Stack Initialized\n");
  
  ret = Add_Acc_Service();
  
  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("Acc service added successfully.\n");
  else
    PRINTF("Error while adding Acc service.\n");
  
	
	// ------------ OUR CUSTOM SERVICE ----------------
	ret = Add_Custom_Acc_Service();
  
  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("CUSTOM Acc service added successfully.\n");
  else
    PRINTF("Error while adding CUSTOM Acc service.\n");
	
	// ------------------------------------------------
  
	
  ret = Add_Environmental_Sensor_Service();
  
  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("Environmental Sensor service added successfully.\n");
  else
    PRINTF("Error while adding Environmental Sensor service.\n");

#if NEW_SERVICES
  /* Instantiate Timer Service with two characteristics:
   * - seconds characteristic (Readable only)
   * - minutes characteristics (Readable and Notifiable )
   */
  ret = Add_Time_Service(); 
  
  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("Time service added successfully.\n");
  else
    PRINTF("Error while adding Time service.\n");  
  
  /* Instantiate LED Button Service with one characteristic:
   * - LED characteristic (Readable and Writable)
   */  
  ret = Add_LED_Service();

  if(ret == BLE_STATUS_SUCCESS)
    PRINTF("LED service added successfully.\n");
  else
    PRINTF("Error while adding LED service.\n");  
#endif

  /* Set output power level */
  ret = aci_hal_set_tx_power_level(1,4);

  while(1)
  {
    HCI_Process();
    User_Process(&axes_data);
		//UART_Receiver();
		
#if NEW_SERVICES
    Update_Time_Characteristics();
#endif
  }
}

/**
 * @brief  Process user input (i.e. pressing the USER button on Nucleo board)
 *         and send the updated acceleration data to the remote client.
 *
 * @param  AxesRaw_t* p_axes
 * @retval None
 */
void User_Process(AxesRaw_t* p_axes)
{
	uint8_t data[3];
	data[0] = 0;
	data[1] = 1;
	data[2] = 0;
  if(set_connectable){
    setConnectable();
    set_connectable = FALSE;
  }

  /* Check if the user has pushed the button */
  if(BSP_PB_GetState(BUTTON_KEY) == RESET)
  {
    while (BSP_PB_GetState(BUTTON_KEY) == RESET);
    
    //BSP_LED_Toggle(LED2); //used for debugging (BSP_LED_Init() above must be also enabled)
    
    if(connected)
    {
			HAL_UART_Transmit(&huart2, data, 3, 1000);
      /* Update acceleration data */
      p_axes->AXIS_X += 1;
      p_axes->AXIS_Y -= 1;
      p_axes->AXIS_Z += 2;
      //PRINTF("ACC: X=%6d Y=%6d Z=%6d\r\n", p_axes->AXIS_X, p_axes->AXIS_Y, p_axes->AXIS_Z);
      Acc_Update(p_axes);
    }
  }
}



void UART_Init(void) {
	huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
}

void GPIO_Init(void) {
	GPIO_InitTypeDef hgpio;
	hgpio.Pin = DATA_INTERRUPT_Pin|IS_MIC_DATA_Pin;
	hgpio.Mode = GPIO_MODE_IT_RISING;
	hgpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &hgpio);
	
	HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

void wait_for_accelerometer_data(AccData * acc_data){
	int bytes_to_receive = ACC_SAMPLE_COUNT * sizeof(float) / sizeof(uint8_t);
	HAL_UART_Receive(&huart2, (uint8_t*) acc_data->pitch, bytes_to_receive, HAL_MAX_DELAY);
	HAL_UART_Receive(&huart2, (uint8_t*) acc_data->roll, bytes_to_receive, HAL_MAX_DELAY);
}

void wait_for_mic_data(MicData * mic_data){
	
	uint32_t buffer[MIC_SAMPLE_COUNT];	
	
	// The bytes are sent with DMA from the discovery board, and it seems like it sends it as 32-bit words.
	// Therefore, we need to shrink down the data, to uint16_t, to save some bandwidth.
	
	int bytes_to_receive = MIC_SAMPLE_COUNT * sizeof(uint32_t) / sizeof(uint8_t);
	
	HAL_UART_Receive(&huart2, (uint8_t*) buffer, bytes_to_receive, HAL_MAX_DELAY);

	for(int i=0; i<MIC_SAMPLE_COUNT; i += 4){
		// for every four bytes we received in buffer, keep only the lowest two.
		mic_data->data[i] = 0x0000FFFF | buffer[i];
	}
}

void send_acc_data(AccData * data){
	// TODO: send the pitch and roll correctly via Bluetooth.
	// TODO: I HAVE NO CLUE WHAT I'M DOING.
  tBleStatus ret;
	
	if(set_connectable){
    setConnectable();
    set_connectable = FALSE;
  }
	if(connected)
	{
		const int bytes_to_send = ACC_SAMPLE_COUNT * sizeof(float) / sizeof(uint8_t);
		const int number_of_batches = bytes_to_send / BLUETOOTH_BATCH_SIZE;
		const int samples_per_batch = BLUETOOTH_BATCH_SIZE / sizeof(float);
		
		
		// TODO: Figure out how to send data in batches.
		// TODO: THIS IS WRONG, need to have numbers in LITTLE-ENDIAN.
		// TODO: SEND PITCH
		int offset = 0;
		for(int batch_i=0; batch_i<number_of_batches;){
			// Send one batch of data.
			ret = aci_gatt_update_char_value(customAccServHandle, customAccCharHandle, 0, BLUETOOTH_BATCH_SIZE, (uint8_t*) &data->pitch[offset]);
			if (ret == BLE_STATUS_SUCCESS){
				// We successfully (I think) sent data using bluetooth.
				batch_i += 1;
				offset += samples_per_batch;
			}else {
				PRINTF("Error while updating CUSTOM ACC characteristic.\n") ;
			}	
		}
		
		// TODO: SEND ROLL
		offset = 0;
		for(int batch_i=0; batch_i<number_of_batches;){
			// Send one batch of data.
			ret = aci_gatt_update_char_value(customAccServHandle, customAccCharHandle, 0, BLUETOOTH_BATCH_SIZE, (uint8_t*) &data->roll[offset]);
			if (ret == BLE_STATUS_SUCCESS){
				// We successfully (I think) sent data using bluetooth.
				batch_i += 1;
				offset += samples_per_batch;
			}else {
				PRINTF("Error while updating CUSTOM ACC characteristic.\n") ;
			}	
		}
		
	}
}

void send_mic_data(MicData * data){
	// TODO: send the mic data correctly via Bluetooth.
	if(set_connectable){
    setConnectable();
    set_connectable = FALSE;
  }
	
	// TODO:
	
	
	
}


void UART_Receiver(){
	
	// TODO: Read a GPIO pin which determines if the data is accelerometer data or microphone data.
	BOOL is_mic_data = FALSE;
	//	is_mic_data = HAL_GPIO_ReadPin(GPIOx, IS_MIC_DATA_Pin);
	
	
	if(is_mic_data){
		PRINTF("Received Microphone data!\n");
		MicData mic_data;
		wait_for_mic_data(&mic_data);
		send_mic_data(&mic_data);
	}else{
		printf("Received Accelerometer data!\n");
		AccData acc_data;
		wait_for_accelerometer_data(&acc_data);
		send_acc_data(&acc_data);
	}
}
	

void EXTI1_IRQHandler(void) {
	int x = 0;
}


/**
 * @}
 */
 
/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
