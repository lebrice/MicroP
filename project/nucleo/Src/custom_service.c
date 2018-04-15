#include "custom_service.h"

uint8_t sample_char_value;
volatile int connected = FALSE;
volatile uint8_t set_connectable = 1;
volatile uint16_t connection_handle = 0;
volatile uint8_t notification_enabled = FALSE;
extern uint8_t bnrg_expansion_board;

// Bluetooth Service Handle
uint16_t custom_service_handle;
// Bluetooth Service Characteristic Handles
uint16_t acc_char_handle, mic_char_handle, digit_char_handle;



#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
		uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
		uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
		uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

// We will use the same UUID as the 'ACC_SERVICE' from the BLE_SampleProject for our custom service.
//#define COPY_ACC_SERVICE_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x01,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_CUSTOM_SERVICE_UUID(uuid_struct)  		COPY_UUID_128(uuid_struct,0x01,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)


// We will use the same UUID as the 'ACC_CHAR' from the BLE_SampleProject for our 'ACC' characteristic.
//#define COPY_ACC_UUID(uuid_struct)          COPY_UUID_128(uuid_struct,0x03,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_ACC_CHAR_UUID(uuid_struct)        		COPY_UUID_128(uuid_struct,0x03,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)


// We will use the same UUID as the 'ACC_CHAR' from the BLE_SampleProject for our 'MIC' characteristic.
//#define COPY_FREE_FALL_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0xe2,0x3e,0x78,0xa0, 0xcf,0x4a, 0x11,0xe1, 0x8f,0xfc, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
  #define COPY_MIC_CHAR_UUID(uuid_struct)       COPY_UUID_128(uuid_struct,0xe2,0x3e,0x78,0xa0, 0xcf,0x4a, 0x11,0xe1, 0x8f,0xfc, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)


// We are using the same UUID as the 'LED_UUID' of the BLE_SampleProject for our 'DIGIT' characteristic.
//#define COPY_LED_UUID(uuid_struct)          COPY_UUID_128(uuid_struct,0x0c,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_DIGIT_CHAR_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x05,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)



//#define COPY_SAMPLE_SERVICE_UUID(uuid_struct)			COPY_UUID_128(uuid_struct,0x02,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
//#define COPY_SAMPLE_CHAR_UUID(uuid_struct)				COPY_UUID_128(uuid_struct,0xe2,0x3e,0x78,0xa0, 0xcf,0x4a, 0x11,0xe1, 0x8f,0xfc, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)

/* Store Value into a buffer in Little Endian Format */
#define STORE_LE_16(buf, val)    ( ((buf)[0] =  (uint8_t) (val)    ) , \
                                   ((buf)[1] =  (uint8_t) (val>>8) ) )


tBleStatus Add_Custom_Acc_Service(void)
{
  tBleStatus ret;

  uint8_t uuid[16];
  
	

	
	// ----------- Adding the service.
  COPY_CUSTOM_SERVICE_UUID(uuid);
	// TODO: figure out if we have the right UUID.
  ret = aci_gatt_add_serv(
		UUID_TYPE_128, 
		uuid, 
		PRIMARY_SERVICE, 
		41,
		&custom_service_handle
	);
  if (ret != BLE_STATUS_SUCCESS){
		printf("Error while adding the Custom Service.\n");
		goto fail;    
	}  
	
	/**List of the various Return codes for aci_gatt_add_char 
	 *
	 *0x00: Success
	 *0x47: Error
	 *0x1F: Out of Memory
	 *0x60: Invalid handle
	 *0x61: Invalid parameter
	 *0x62: Out of handle
	 *0x64: Insufficient resources
	 *0x66: Character Already Exists
	 */
	
  COPY_ACC_CHAR_UUID(uuid);  
  ret =  aci_gatt_add_char(
		custom_service_handle,
		UUID_TYPE_128, 
		uuid,
		20,
		CHAR_PROP_NOTIFY|CHAR_PROP_READ,
		ATTR_PERMISSION_NONE,
		GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
		16, 
		0,
		&acc_char_handle
	);
	
  if (ret != BLE_STATUS_SUCCESS){
		printf("Error while adding the ACC characteristic. (Errorcode %X)\n", ret);
		goto fail;    
	} 
	COPY_MIC_CHAR_UUID(uuid);  
  ret =  aci_gatt_add_char(
		custom_service_handle,
		UUID_TYPE_128, 
		uuid,
		20,
		CHAR_PROP_NOTIFY|CHAR_PROP_READ,
		ATTR_PERMISSION_NONE,
		GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
		16, 
		0,
		&mic_char_handle
	);
	if (ret != BLE_STATUS_SUCCESS){
		printf("Error while adding the MIC characteristic. (Errorcode %X)\n", ret);
		goto fail;    
	} 
	COPY_DIGIT_CHAR_UUID(uuid); 
  ret =  aci_gatt_add_char(
		custom_service_handle,
		UUID_TYPE_128, 
		uuid,
		1,
		CHAR_PROP_NOTIFY|CHAR_PROP_WRITE,
		ATTR_PERMISSION_NONE,
		//GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
		GATT_NOTIFY_ATTRIBUTE_WRITE, // I think this is better.
		16, 
		0,
		&digit_char_handle
	);
  if (ret != BLE_STATUS_SUCCESS){
		printf("Error while adding the DIGIT characteristic. (Errorcode %X)\n", ret);
		goto fail;    
	} 
  PRINTF("Service CUSTOM_ACC added. Handle 0x%04X, Acc Charac handle: 0x%04X, Mic Charac handle: 0x%04X\n",
		custom_service_handle,
		acc_char_handle,
		mic_char_handle
	);	
  return BLE_STATUS_SUCCESS; 
  
fail:
  PRINTF("Error while adding ACC service.\n");
  return BLE_STATUS_ERROR ;
    
}




/** Interleave pitch and roll into the serialized buffer.
*
*	Original:	[ p1, p2, p3, (...) ] <-- (Pitch)
*						[ r1, r2, r3, (...) ] <-- (Roll)
*
*	Result: [ p1, r1, p2, r2, p3, r3, (...) ] <-- (interleaved)
*
* NOTE: WE USE BIG ENDIAN.
*/
void serialize_acc_batch(AccBatch* batch, uint8_t serialized[BLE_MAX_DATA_BYTES]){
	for(int i=0; i<ACC_SAMPLES_PER_BATCH; i++){
		uint8_t* target;
		uint8_t* source; 
		
		source = (uint8_t*)(&batch->pitch[i]);
		target = &serialized[8*i];
		target[0] = source[3];
		target[1] = source[2];
		target[2] = source[1];
		target[3] = source[0];
		
		source = (uint8_t*)(&batch->roll[i]);
		target = &serialized[8*i + 4];
		target[0] = source[3];
		target[1] = source[2];
		target[2] = source[1];
		target[3] = source[0];
	}
}

void serialize_mic_batch(MicBatch* batch, uint8_t* serialized){
	byte* source;
	byte* target;
	for(int i=0; i<MIC_SAMPLES_PER_BATCH; i++){
		source = (byte*) &batch->data[i];
		target = &serialized[2*i];
		target[0] = source[1];
		target[1] = source[0];
	}
}


tBleStatus acc_update(AccBatch* acc_batch){
	uint8_t ret;
	uint8_t serialized[ACC_BYTES_PER_BATCH];
	
	
	if(set_connectable){
		setConnectable();
		set_connectable = FALSE;
	}
	
	serialize_acc_batch(acc_batch, serialized);	
	
	ret = aci_gatt_update_char_value(custom_service_handle, acc_char_handle, 0, 20, serialized);
	
  if (ret != BLE_STATUS_SUCCESS){
    PRINTF("Error while updating ACC characteristic.\n") ;
    return BLE_STATUS_ERROR ;
  }
  return BLE_STATUS_SUCCESS;	
}


tBleStatus mic_update(MicBatch* mic_batch){
	
	uint8_t ret;
	uint8_t serialized[ACC_BYTES_PER_BATCH];
	
	
	if(set_connectable){
		setConnectable();
		set_connectable = FALSE;
	}
	
	
	serialize_mic_batch(mic_batch, serialized);	
	
	ret = aci_gatt_update_char_value(custom_service_handle, acc_char_handle, 0, 20, serialized);
	
  if (ret != BLE_STATUS_SUCCESS){
    PRINTF("Error while updating ACC characteristic.\n") ;
    return BLE_STATUS_ERROR ;
  }
  return BLE_STATUS_SUCCESS;	
}


/**
 * @brief  Puts the device in connectable mode.
 *         If you want to specify a UUID list in the advertising data, those data can
 *         be specified as a parameter in aci_gap_set_discoverable().
 *         For manufacture data, aci_gap_update_adv_data must be called.
 * @param  None 
 * @retval None
 */
/* Ex.:
 *
 *  tBleStatus ret;    
 *  const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'G','X','X'};    
 *  const uint8_t serviceUUIDList[] = {AD_TYPE_16_BIT_SERV_UUID,0x34,0x12};    
 *  const uint8_t manuf_data[] = {4, AD_TYPE_MANUFACTURER_SPECIFIC_DATA, 0x05, 0x02, 0x01};
 *  
 *  ret = aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE,
 *                                 8, local_name, 3, serviceUUIDList, 0, 0);    
 *  ret = aci_gap_update_adv_data(5, manuf_data);
 *
 */
void setConnectable(void)
{  
  tBleStatus ret;
  // (0x09) 'G', 'X', 'X'
  const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'B','l','u','e','N','R','G'};
  
  /* disable scan response */
  hci_le_set_scan_resp_data(0,NULL);
  PRINTF("General Discoverable Mode.\n");
  
  ret = aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                 sizeof(local_name), local_name, 0, NULL, 0, 0);
  if (ret != BLE_STATUS_SUCCESS) {
    printf("Error while setting discoverable mode (%d)\n", ret);    
  }  
}
/**
 * @brief  This function is called when there is a LE Connection Complete event.
 * @param  uint8_t Address of peer device
 * @param  uint16_t Connection handle
 * @retval None
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle)
{  
  connected = TRUE;
  connection_handle = handle;
  
  PRINTF("Connected to device:");
  for(int i = 5; i > 0; i--){
    PRINTF("%02X-", addr[i]);
  }
  PRINTF("%02X\n", addr[0]);
}

/**
 * @brief  This function is called when the peer device gets disconnected.
 * @param  None 
 * @retval None
 */
void GAP_DisconnectionComplete_CB(void)
{
  connected = FALSE;
  PRINTF("Disconnected\n");
  /* Make the device connectable again. */
  set_connectable = TRUE;
  notification_enabled = FALSE;
}

/**
 * @brief  Read request callback.
 * @param  uint16_t Handle of the attribute
 * @retval None
 */
void Read_Request_CB(uint16_t handle)
{  
//  if(handle == sampleCharHandle + 1){
//		Sample_Characteristic_Update(sample_char_value);
//		PRINTF("Reading Sample Characteristic\n");
//	}  
  
  //EXIT:
  if(connection_handle != 0)
    aci_gatt_allow_read(connection_handle);
}

/**
 * @brief  Callback processing the ACI events.
 * @note   Inside this function each event must be identified and correctly
 *         parsed.
 * @param  void* Pointer to the ACI packet
 * @retval None
 */
void HCI_Event_CB(void *pckt)
{
	extern void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data);
		
  hci_uart_pckt *hci_pckt = pckt;
  /* obtain event packet */
  hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;
  
  if(hci_pckt->type != HCI_EVENT_PKT)
    return;
  
  switch(event_pckt->evt){
    
  case EVT_DISCONN_COMPLETE:
    {
      GAP_DisconnectionComplete_CB();
    }
    break;
    
  case EVT_LE_META_EVENT:
    {
      evt_le_meta_event *evt = (void *)event_pckt->data;
      
      switch(evt->subevent){
      case EVT_LE_CONN_COMPLETE:
        {
          evt_le_connection_complete *cc = (void *)evt->data;
          GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
        }
        break;
      }
    }
    break;
    
  case EVT_VENDOR:
    {
      evt_blue_aci *blue_evt = (void*)event_pckt->data;
      switch(blue_evt->ecode){
				
			case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:         
        {
          /* this callback is invoked when a GATT attribute is modified
          extract callback data and pass to suitable handler function */
          if (bnrg_expansion_board == IDB05A1) {
            evt_gatt_attr_modified_IDB05A1 *evt = (evt_gatt_attr_modified_IDB05A1*)blue_evt->data;
            Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data); 
          }
          else {
            evt_gatt_attr_modified_IDB04A1 *evt = (evt_gatt_attr_modified_IDB04A1*)blue_evt->data;
            Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data); 
          }                       
        }
        break; 
				
      case EVT_BLUE_GATT_READ_PERMIT_REQ:
        {
          evt_gatt_read_permit_req *pr = (void*)blue_evt->data;                    
          Read_Request_CB(pr->attr_handle);                    
        }
        break;
      }
    }
    break;
  }    
}

