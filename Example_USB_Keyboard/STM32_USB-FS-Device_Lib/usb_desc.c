/**
  ******************************************************************************
  * @file    usb_desc.c
  * @author  MCD Application Team
  * @version V3.4.0
  * @date    09-September-2013
  * @brief   Descriptors for RHID Demo
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


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* USB Standard Device Descriptor */
const uint8_t RHID_DeviceDescriptor[RHID_SIZ_DEVICE_DESC] =
  {
		    RHID_SIZ_DEVICE_DESC,         // общая длина дескриптора устройства в байтах
		    USB_DEVICE_DESCRIPTOR_TYPE, // bDescriptorType - показывает, что это за дескриптор. В данном случае - Device descriptor
		    0x00, 0x02,                 // bcdUSB - какую версию стандарта USB поддерживает устройство. 2.0

			// класс, подкласс устройства и протокол, по стандарту USB. У нас нули, означает каждый интерфейс сам за себя
		    0x00,                       //bDeviceClass
		    0x00,                       //bDeviceSubClass
		    0x00,                       //bDeviceProtocol

		    0x40,                       //bMaxPacketSize - максимальный размер пакетов для Endpoint 0 (при конфигурировании)

			// те самые пресловутые VID и PID,  по которым и определяется, что же это за устройство.
			// в реальных устройствах надо покупать VID, чтобы устройства можно было различать и подсовывать нужные драйвера
		    0x83, 0x04,                 //idVendor (0x0483)
		    0x10, 0x57,                 //idProduct (0x5711)

		    DEVICE_VER_L, DEVICE_VER_H,                 // bcdDevice rel. DEVICE_VER_H.DEVICE_VER_L  номер релиза устройства

			// дальше идут индексы строк, описывающих производителя, устройство и серийный номер.
			// Отображаются в свойствах устройства в диспетчере устройств
			// А по серийному номеру подключенные устройства с одинаковым VID/PID различаются системой.
		    1,                          //Index of string descriptor describing manufacturer
		    2,                          //Index of string descriptor describing product
		    3,                          //Index of string descriptor describing the device serial number
		    0x01                        // bNumConfigurations - количество возможных конфигураций. У нас одна.
  }
  ; /* CustomHID_DeviceDescriptor */


/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const uint8_t RHID_ConfigDescriptor[RHID_SIZ_CONFIG_DESC] =
  {
		    0x09, 			// bLength: длина дескриптора конфигурации
		    USB_CONFIGURATION_DESCRIPTOR_TYPE, // bDescriptorType: тип дескриптора - конфигурация
		    RHID_SIZ_CONFIG_DESC, 0x00, // wTotalLength: общий размер всего дерева под данной конфигурацией в байтах

		    0x01,         // bNumInterfaces: в конфигурации всего один интерфейс
		    0x01,         // bConfigurationValue: индекс данной конфигурации
		    0x00,         // iConfiguration: индекс строки, которая описывает эту конфигурацию
		    0xE0,         // bmAttributes: признак того, что устройство будет питаться от шины USB
		    0x32,         // MaxPower 100 mA: и ему хватит 100 мА

				/************** Дескриптор интерфейса ****************/
				0x09,         // bLength: размер дескриптора интерфейса
				USB_INTERFACE_DESCRIPTOR_TYPE, // bDescriptorType: тип дескриптора - интерфейс
				0x00,         // bInterfaceNumber: порядковый номер интерфейса - 0
				0x00,         // bAlternateSetting: признак альтернативного интерфейса, у нас не используется
				0x02,         // bNumEndpoints - количество эндпоинтов.

				0x03,         // bInterfaceClass: класс интерфеса - HID
				// если бы мы косили под стандартное устройство, например клавиатуру или мышь, то надо было бы указать правильно класс и подкласс
				// а так у нас общее HID-устройство
				0x00,         // bInterfaceSubClass : подкласс интерфейса.
				0x00,         // nInterfaceProtocol : протокол интерфейса

				0,            // iInterface: индекс строки, описывающей интерфейс

					// теперь отдельный дескриптор для уточнения того, что данный интерфейс - это HID устройство
					/******************** HID дескриптор ********************/
					0x09,         // bLength: длина HID-дескриптора
					HID_DESCRIPTOR_TYPE, // bDescriptorType: тип дескриптора - HID
					0x01, 0x01,   // bcdHID: номер версии HID 1.1
					0x00,         // bCountryCode: код страны (если нужен)
					0x01,         // bNumDescriptors: Сколько дальше будет report дескрипторов
						HID_REPORT_DESCRIPTOR_TYPE,         // bDescriptorType: Тип дескриптора - report
						RHID_SIZ_REPORT_DESC,	0x00, // wItemLength: длина report-дескриптора


					/******************** дескриптор конечных точек (endpoints) ********************/
					0x07,          // bLength: длина дескриптора
					USB_ENDPOINT_DESCRIPTOR_TYPE, // тип дескриптора - endpoints

					0x81,          // bEndpointAddress: адрес конечной точки и направление 1(IN)
					0x03,          // bmAttributes: тип конечной точки - Interrupt endpoint
					wMaxPacketSize, 0x00,    // wMaxPacketSize:  Bytes max
					0x20,          // bInterval: Polling Interval (32 ms)

          0x07,	/* bLength: Endpoint Descriptor size */
          USB_ENDPOINT_DESCRIPTOR_TYPE,	/* bDescriptorType: */
            /*	Endpoint descriptor type */
          0x01,	/* bEndpointAddress: */
            /*	Endpoint Address (OUT) */
          0x03,	/* bmAttributes: Interrupt endpoint */
          wMaxPacketSize,	/* wMaxPacketSize:  Bytes max  */
          0x00,
          0x20,	/* bInterval: Polling Interval (32 ms) */
}
  ; /* RHID_ConfigDescriptor */
const uint8_t RHID_ReportDescriptor[RHID_SIZ_REPORT_DESC] =
  {
		    0x05, 0x01, /* Usage Page (Generic Desktop)             */
		    0x09, 0x02, /* Usage (Mouse)                            */
		    0xA1, 0x01, /* Collection (Application)                 */
		    0x09, 0x01, /*  Usage (Pointer)                         */
		    0xA1, 0x00, /*  Collection (Physical)                   */
		    0x85, 0x01,  /*   Report ID  */
		    0x05, 0x09, /*      Usage Page (Buttons)                */
		    0x19, 0x01, /*      Usage Minimum (01)                  */
		    0x29, 0x03, /*      Usage Maximum (03)                  */
		    0x15, 0x00, /*      Logical Minimum (0)                 */
		    0x25, 0x01, /*      Logical Maximum (0)                 */
		    0x95, 0x03, /*      Report Count (3)                    */
		    0x75, 0x01, /*      Report Size (1)                     */
		    0x81, 0x02, /*      Input (Data, Variable, Absolute)    */
		    0x95, 0x01, /*      Report Count (1)                    */
		    0x75, 0x05, /*      Report Size (5)                     */
		    0x81, 0x01, /*      Input (Constant)    ;5 bit padding  */
		    0x05, 0x01, /*      Usage Page (Generic Desktop)        */
		    0x09, 0x30, /*      Usage (X)                           */
		    0x09, 0x31, /*      Usage (Y)                           */
		    0x15, 0x81, /*      Logical Minimum (-127)              */
		    0x25, 0x7F, /*      Logical Maximum (127)               */
		    0x75, 0x08, /*      Report Size (8)                     */
		    0x95, 0x02, /*      Report Count (2)                    */
		    0x81, 0x06, /*      Input (Data, Variable, Relative)    */
		    0xC0, 0xC0,/* End Collection,End Collection            */
		//
		    0x09, 0x06, /*		Usage (Keyboard)                    */
		    0xA1, 0x01, /*		Collection (Application)            */
		    0x85, 0x02,  /*   Report ID  */
		    0x05, 0x07, /*  	Usage (Key codes)                   */
		    0x19, 0xE0, /*      Usage Minimum (224)                 */
		    0x29, 0xE7, /*      Usage Maximum (231)                 */
		    0x15, 0x00, /*      Logical Minimum (0)                 */
		    0x25, 0x01, /*      Logical Maximum (1)                 */
		    0x75, 0x01, /*      Report Size (1)                     */
		    0x95, 0x08, /*      Report Count (8)                    */
		    0x81, 0x02, /*      Input (Data, Variable, Absolute)    */
		    0x95, 0x01, /*      Report Count (1)                    */
		    0x75, 0x08, /*      Report Size (8)                     */
		    0x81, 0x01, /*      Input (Constant)    ;5 bit padding  */
		    0x95, 0x05, /*      Report Count (5)                    */
		    0x75, 0x01, /*      Report Size (1)                     */
		    0x05, 0x08, /*      Usage Page (Page# for LEDs)         */
		    0x19, 0x01, /*      Usage Minimum (01)                  */
		    0x29, 0x05, /*      Usage Maximum (05)                  */
		    0x91, 0x02, /*      Output (Data, Variable, Absolute)   */
		    0x95, 0x01, /*      Report Count (1)                    */
		    0x75, 0x03, /*      Report Size (3)                     */
		    0x91, 0x01, /*      Output (Constant)                   */
		    0x95, 0x06, /*      Report Count (1)                    */
		    0x75, 0x08, /*      Report Size (3)                     */
		    0x15, 0x00, /*      Logical Minimum (0)                 */
		    0x25, 0x65, /*      Logical Maximum (101)               */
		    0x05, 0x07, /*  	Usage (Key codes)                   */
		    0x19, 0x00, /*      Usage Minimum (00)                  */
		    0x29, 0x65, /*      Usage Maximum (101)                 */
		    0x81, 0x00, /*      Input (Data, Array)                 */
		    0xC0 /* 		End Collection,End Collection       */
}
  ; /* RHID_ReportDescriptor */

/* USB String Descriptors (optional) */
const uint8_t RHID_StringLangID[RHID_SIZ_STRING_LANGID] =
  {
    RHID_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04
  }
  ; /* LangID = 0x0409: U.S. English */

const uint8_t RHID_StringVendor[RHID_SIZ_STRING_VENDOR] =
  {
    RHID_SIZ_STRING_VENDOR, /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
    /* Manufacturer: "STMicroelectronics" */
    'R', 0, 'a', 0, 'J', 0, 'a', 0
  };

const uint8_t RHID_StringProduct[RHID_SIZ_STRING_PRODUCT] =
  {
    RHID_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'R', 0, 'H', 0, 'I', 0, 'D', 0, ' ', 0, 'S', 0, 'T', 0,
    'M', 0, '3', 0, '2', 0
  };
uint8_t RHID_StringSerial[RHID_SIZ_STRING_SERIAL] =
  {
    RHID_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'R', 0, 'H', 0, '-', 0, '0', 0, '0', 0, '0', 0, '1', 0
  };

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

