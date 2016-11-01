/**
  ******************************************************************************
  * @file    usb_prop.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   All processing related to HID Demo
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "hw_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t ProtocolValue;
__IO uint8_t EXTI_Enable;
__IO uint8_t Request = 0;
uint8_t Report_Buf[wMaxPacketSize];
extern uint8_t Buffer[RPT4_COUNT+1]; //defined in hw_config.c

/* -------------------------------------------------------------------------- */
/*  Structures initializations */
/* -------------------------------------------------------------------------- */

DEVICE Device_Table =
  {
    EP_NUM,
    1
  };

DEVICE_PROP Device_Property =
  {
    HID_init,
    HID_Reset,
    HID_Status_In,
    HID_Status_Out,
    HID_Data_Setup,
    HID_NoData_Setup,
    HID_Get_Interface_Setting,
    HID_GetDeviceDescriptor,
    HID_GetConfigDescriptor,
    HID_GetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/
  };
USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    HID_GetConfiguration,
    HID_SetConfiguration,
    HID_GetInterface,
    HID_SetInterface,
    HID_GetStatus,
    HID_ClearFeature,
    HID_SetEndPointFeature,
    HID_SetDeviceFeature,
    HID_SetDeviceAddress
  };

ONE_DESCRIPTOR Device_Descriptor =
  {
    (uint8_t*)RHID_DeviceDescriptor,
    RHID_SIZ_DEVICE_DESC
  };

ONE_DESCRIPTOR Config_Descriptor =
  {
    (uint8_t*)RHID_ConfigDescriptor,
    RHID_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR RHID_Report_Descriptor =
  {
    (uint8_t *)RHID_ReportDescriptor,
    RHID_SIZ_REPORT_DESC
  };

ONE_DESCRIPTOR RHID_Hid_Descriptor =
  {
    (uint8_t*)RHID_ConfigDescriptor + RHID_OFF_HID_DESC,
    RHID_SIZ_HID_DESC
  };

ONE_DESCRIPTOR String_Descriptor[4] =
  {
    {(uint8_t*)RHID_StringLangID, RHID_SIZ_STRING_LANGID},
    {(uint8_t*)RHID_StringVendor, RHID_SIZ_STRING_VENDOR},
    {(uint8_t*)RHID_StringProduct, RHID_SIZ_STRING_PRODUCT},
    {(uint8_t*)RHID_StringSerial, RHID_SIZ_STRING_SERIAL}
  };

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*CustomHID_SetReport_Feature function prototypes*/
uint8_t *HID_SetReport_Feature(uint16_t Length);

/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : HID_init.
* Description    : HID init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void HID_init(void)
{

  /* Update the serial number string descriptor with the data from the unique
  ID*/
  Get_SerialNum();

  pInformation->Current_Configuration = 0;
  /* Connect the device */
  PowerOn();

  /* Perform basic device initialization operations */
  USB_SIL_Init();

  bDeviceState = UNCONNECTED;
}

/*******************************************************************************
* Function Name  : HID_Reset.
* Description    : HID reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void HID_Reset(void)
{
  /* Set HID_DEVICE as not configured */
  pInformation->Current_Configuration = 0;
  pInformation->Current_Interface = 0;/*the default Interface*/

  /* Current Feature initialization */
  pInformation->Current_Feature = RHID_ConfigDescriptor[7];
  SetBTABLE(BTABLE_ADDRESS);
  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_STALL);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_INTERRUPT);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPRxAddr(ENDP1, ENDP1_RXADDR);
  SetEPTxCount(ENDP1, EP1TxCount);
  SetEPRxCount(ENDP1, EP1RxCount);
  SetEPRxStatus(ENDP1, EP_RX_VALID);
  SetEPTxStatus(ENDP1, EP_TX_NAK);

  /* Set this device to response on default address */
  SetDeviceAddress(0);
  bDeviceState = ATTACHED;
}
/*******************************************************************************
* Function Name  : HID_SetConfiguration.
* Description    : Update the device state to configured.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void HID_SetConfiguration(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
}
/*******************************************************************************
* Function Name  : HID_SetConfiguration.
* Description    : Update the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void HID_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}
/*******************************************************************************
* Function Name  : HID_Status_In.
* Description    : HID status IN routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void HID_Status_In(void)
{
  BitAction Led_State;

  if (Report_Buf[1] == 0)
  {
    Led_State = Bit_RESET;
  }
  else
  {
    Led_State = Bit_SET;
  }

  switch (Report_Buf[0])
  {
    case 1: /* Led 1 */
     if (Led_State != Bit_RESET)
     {
       /////GPIO_SetBits(LED_PORT,LED1_PIN);
     }
     else
     {
       /////GPIO_ResetBits(LED_PORT,LED1_PIN);
     }
     break;
    case 2: /* Led 2 */
     if (Led_State != Bit_RESET)
     {
       /////GPIO_SetBits(LED_PORT,LED2_PIN);
     }
     else
     {
       /////GPIO_ResetBits(LED_PORT,LED2_PIN);
     }
      break;
    case 3: /* Led 1&2 */
       Buffer[4]=Report_Buf[1];
     break;
  }
}

/*******************************************************************************
* Function Name  : HID_Status_Out
* Description    : HID status OUT routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void HID_Status_Out (void)
{}

/*******************************************************************************
* Function Name  : HID_Data_Setup
* Description    : Handle the data class specific requests.
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT HID_Data_Setup(uint8_t RequestNo)
{
  uint8_t *(*CopyRoutine)(uint16_t);

  CopyRoutine = NULL;
  if ((RequestNo == GET_DESCRIPTOR)
      && (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
      && (pInformation->USBwIndex0 == 0))
  {
    if (pInformation->USBwValue1 == REPORT_DESCRIPTOR)
    {
      CopyRoutine = HID_GetReportDescriptor;
    }
    else if (pInformation->USBwValue1 == HID_DESCRIPTOR_TYPE)
    {
      CopyRoutine = HID_GetHIDDescriptor;
    }

  } /* End of GET_DESCRIPTOR */

  /*** GET_PROTOCOL, GET_REPORT, SET_REPORT ***/
  else if ( (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) )
  {
    switch( RequestNo )
    {
    case GET_PROTOCOL:
      CopyRoutine = HID_GetProtocolValue;
      break;
    case SET_REPORT:
      CopyRoutine = HID_SetReport_Feature;
      Request = SET_REPORT;
      break;
    default:
      break;
    }
  }

  if (CopyRoutine == NULL)
  {
    return USB_UNSUPPORT;
  }
  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : HID_SetReport_Feature
* Description    : Set Feature request handling
* Input          : Length.
* Output         : None.
* Return         : Buffer
*******************************************************************************/
uint8_t *HID_SetReport_Feature(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = wMaxPacketSize;
    return NULL;
  }
  else
  {
    return Report_Buf;
  }
}

/*******************************************************************************
* Function Name  : HID_NoData_Setup
* Description    : handle the no data class specific requests
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT HID_NoData_Setup(uint8_t RequestNo)
{
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      && (RequestNo == SET_PROTOCOL))
  {
    return HID_SetProtocol();
  }

  else
  {
    return USB_UNSUPPORT;
  }
}

/*******************************************************************************
* Function Name  : HID_GetDeviceDescriptor.
* Description    : Gets the device descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the device descriptor.
*******************************************************************************/
uint8_t *HID_GetDeviceDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*******************************************************************************
* Function Name  : HID_GetConfigDescriptor.
* Description    : Gets the configuration descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *HID_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
* Function Name  : HID_GetStringDescriptor
* Description    : Gets the string descriptors according to the needed index
* Input          : Length
* Output         : None.
* Return         : The address of the string descriptors.
*******************************************************************************/
uint8_t *HID_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;
  if (wValue0 > 4)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
  }
}

/*******************************************************************************
* Function Name  : HID_GetReportDescriptor.
* Description    : Gets the HID report descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *HID_GetReportDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &RHID_Report_Descriptor);
}

/*******************************************************************************
* Function Name  : HID_GetHIDDescriptor.
* Description    : Gets the HID descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *HID_GetHIDDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &RHID_Hid_Descriptor);
}

/*******************************************************************************
* Function Name  : HID_Get_Interface_Setting.
* Description    : tests the interface and the alternate setting according to the
*                  supported one.
* Input          : - Interface : interface number.
*                  - AlternateSetting : Alternate Setting number.
* Output         : None.
* Return         : USB_SUCCESS or USB_UNSUPPORT.
*******************************************************************************/
RESULT HID_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;
  }
  else if (Interface > 0)
  {
    return USB_UNSUPPORT;
  }
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : HID_SetProtocol
* Description    : HID Set Protocol request routine.
* Input          : None.
* Output         : None.
* Return         : USB SUCCESS.
*******************************************************************************/
RESULT HID_SetProtocol(void)
{
  uint8_t wValue0 = pInformation->USBwValue0;
  ProtocolValue = wValue0;
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : HID_GetProtocolValue
* Description    : get the protocol value
* Input          : Length.
* Output         : None.
* Return         : address of the protocol value.
*******************************************************************************/
uint8_t *HID_GetProtocolValue(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = 1;
    return NULL;
  }
  else
  {
    return (uint8_t *)(&ProtocolValue);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
