/**
  ******************************************************************************
  * @file    usb_desc.h
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    09-September-2013
  * @brief   Descriptor Header for RHID Demo
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DESC_H
#define __USB_DESC_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define HID_DESCRIPTOR_TYPE                     0x21
#define HID_REPORT_DESCRIPTOR_TYPE				0x22

#define RHID_SIZ_HID_DESC                   0x09
#define RHID_OFF_HID_DESC                   0x12

#define RHID_SIZ_DEVICE_DESC                18
#define RHID_SIZ_CONFIG_DESC                41
#define RHID_SIZ_REPORT_DESC                115
#define RHID_SIZ_STRING_LANGID              4
#define RHID_SIZ_STRING_VENDOR              10
#define RHID_SIZ_STRING_PRODUCT             22
#define RHID_SIZ_STRING_SERIAL              26

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define DEVICE_VER_H 0x01
#define DEVICE_VER_L 0x00

//HID Maximum packet size in bytes
#define wMaxPacketSize  0x40
#define EP1TxCount wMaxPacketSize
#define EP1RxCount 2

#define RPT3_COUNT 0x01 //PC->STM32
#define RPT4_COUNT 0x04 //STM32->PC

/* Exported functions ------------------------------------------------------- */
extern const uint8_t RHID_DeviceDescriptor[RHID_SIZ_DEVICE_DESC];
extern const uint8_t RHID_ConfigDescriptor[RHID_SIZ_CONFIG_DESC];
extern const uint8_t RHID_ReportDescriptor[RHID_SIZ_REPORT_DESC];
extern const uint8_t RHID_StringLangID[RHID_SIZ_STRING_LANGID];
extern const uint8_t RHID_StringVendor[RHID_SIZ_STRING_VENDOR];
extern const uint8_t RHID_StringProduct[RHID_SIZ_STRING_PRODUCT];
extern uint8_t RHID_StringSerial[RHID_SIZ_STRING_SERIAL];

#endif /* __USB_DESC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
