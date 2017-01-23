/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : mass_mal.h
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : Header for mass_mal.c file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MASS_MAL_H
#define __MASS_MAL_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MAL_OK   0
#define MAL_FAIL 1
#define MAX_LUN  1

#define     FLASH_DISK_START_ADDRESS		          0x08002800     /* Flash start address (10K for this bootloader) */
#define     FLASH_DISK_SIZE              				  55296
#define     FLASH_PAGE_SIZE                       1024         /* 1K per page */
#define     WAIT_TIMEOUT                		      100000 

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

uint16_t MAL_Init (uint8_t lun);
uint16_t MAL_GetStatus (uint8_t lun);
uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length);
uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length);
#endif /* __MASS_MAL_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
