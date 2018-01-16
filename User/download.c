/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : download.c
* Author             : MCD Application Team
* Version            : V2.0.1
* Date               : 06/13/2008
* Description        : This file provides the software which allows to download
*                      an image to internal Flash.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern u8 file_name[FILE_NAME_LENGTH];
u8 tab_1024[1024] =
{
    0
};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : SerialDownload
* Description    : Download a file via serial port
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SerialDownload(void)
{
    u8 Number[10] = "          ";
    s32 Size = 0;

    SerialPutString("Waiting for the new firmware file to be transferred (Press 'a' to stop)\n\r");

    Size = Ymodem_Receive(&tab_1024[0]);

    if (Size > 0)
    {
        u32 dly=1000000;
        while(dly--);
        SerialPutString("\r Firmware has been successfully upgraded!\n\r--------------------------------\r\n Edition: ");
        SerialPutString(file_name);
        Int2Str(Number, Size);
        SerialPutString("\n\r Size: ");
        SerialPutString(Number);
        SerialPutString(" Bytes");
        SerialPutString("\n\r--------------------------------");
        SerialPutString("\n\rPress '2' to jump to user app or '1' update again\n\r");
    }
    else if (Size == -1)
    {
        SerialPutString("\n\n\rThe image size is higher than the allowed space memory!\n\r");
    }
    else if (Size == -2)
    {
        SerialPutString("\n\n\rVerification failed!\n\r");
    }
    else if (Size == -3)
    {
        SerialPutString("\r\n\User termination!Can continue to enter!\n\r");
    }
    else
    {
        SerialPutString("\n\rFailed to receive the file!\n\r");
    }
}

/*******************(C)COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
