/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : mass_mal.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : Medium Access Layer interface
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "mass_mal.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Mass_Memory_Size[2];
uint32_t Mass_Block_Size[2];
uint32_t Mass_Block_Count[2];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Init(uint8_t lun)
{
  switch (lun)
  {
    case 0:
			FLASH_Unlock(); 
      break;
    case 1:
	  return MAL_FAIL;
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}
/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{
  uint16_t i;
  switch (lun)
  {
    case 0:	
	   for(i = 0; i < Transfer_Length; i += FLASH_PAGE_SIZE)
	   { 
	    if (FLASH_WaitForLastOperation(WAIT_TIMEOUT) != FLASH_TIMEOUT)
		  {
		     FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		  } 	
		  FLASH_ErasePage(FLASH_DISK_START_ADDRESS + Memory_Offset + i); 
	   }			
		 
	   for(i = 0; i < Transfer_Length; i += 4)
	   { 
		   if(FLASH_WaitForLastOperation(WAIT_TIMEOUT) != FLASH_TIMEOUT)
		   {
		      FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
		   } 
		   FLASH_ProgramWord(FLASH_DISK_START_ADDRESS + Memory_Offset + i , Writebuff[i >> 2]); 
	   }

      break;
    case 1:
      break;
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{
  uint16_t i;
  switch (lun)
  {
    case 0:
  	  for(i = 0; i < Transfer_Length; i += 4)
			{
					 Readbuff[i >> 2] = ((vu32*)(FLASH_DISK_START_ADDRESS + Memory_Offset))[i >> 2]; 
			}
      break;
    case 1:
      break;
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_GetStatus (uint8_t lun)
{
  if (lun == 0)
  {		 
      Mass_Block_Count[0] = FLASH_DISK_SIZE / FLASH_PAGE_SIZE; 
      Mass_Block_Size[0] =  FLASH_PAGE_SIZE; 
      Mass_Memory_Size[0] = FLASH_DISK_SIZE; 
      return MAL_OK;
  }

  return MAL_FAIL;
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
