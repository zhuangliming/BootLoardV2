/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : common.c
* Author             : MCD Application Team
* Version            : V2.0.1
* Date               : 06/13/2008
* Description        : This file provides all the common functions.
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
pFunction Jump_To_Application;
u32 JumpAddress;
u32 BlockNbr = 0, UserMemoryMask = 0;
bool FlashProtection = FALSE;
extern u32 FlashDestination;

void delay_msp(void);
void delay_msp(void)
{
    u32 i=1000;
    while(i--);
}
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Int2Str
* Description    : Convert an Integer to a string
* Input          : - str: The string
*                  - intnum: The intger to be converted
* Output         : None
* Return         : None
*******************************************************************************/
void Int2Str(u8* str, s32 intnum)
{
    u32 i, Div = 1000000000, j = 0, Status = 0;

    for (i = 0; i < 10; i++)
    {
        str[j++] = (intnum / Div) + 48;

        intnum = intnum % Div;
        Div /= 10;
        if ((str[j-1] == '0') & (Status == 0))
        {
            j = 0;
        }
        else
        {
            Status++;
        }
    }
}

/*******************************************************************************
* Function Name  : Str2Int
* Description    : Convert a string to an integer
* Input 1        : - inputstr: The string to be converted
*                  - intnum: The intger value
* Output         : None
* Return         : 1: Correct
*                  0: Error
*******************************************************************************/
u32 Str2Int(u8 *inputstr, s32 *intnum)
{
    u32 i = 0, res = 0;
    u32 val = 0;

    if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X'))
    {
        if (inputstr[2] == '\0')
        {
            return 0;
        }
        for (i = 2; i < 11; i++)
        {
            if (inputstr[i] == '\0')
            {
                *intnum = val;
                /* return 1; */
                res = 1;
                break;
            }
            if (ISVALIDHEX(inputstr[i]))
            {
                val = (val << 4) + CONVERTHEX(inputstr[i]);
            }
            else
            {
                /* return 0, Invalid input */
                res = 0;
                break;
            }
        }
        /* over 8 digit hex --invalid */
        if (i >= 11)
        {
            res = 0;
        }
    }
    else     /* max 10-digit decimal input */
    {
        for (i = 0; i < 11; i++)
        {
            if (inputstr[i] == '\0')
            {
                *intnum = val;
                /* return 1 */
                res = 1;
                break;
            }
            else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0))
            {
                val = val << 10;
                *intnum = val;
                res = 1;
                break;
            }
            else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0))
            {
                val = val << 20;
                *intnum = val;
                res = 1;
                break;
            }
            else if (ISVALIDDEC(inputstr[i]))
            {
                val = val * 10 + CONVERTDEC(inputstr[i]);
            }
            else
            {
                /* return 0, Invalid input */
                res = 0;
                break;
            }
        }
        /* Over 10 digit decimal --invalid */
        if (i >= 11)
        {
            res = 0;
        }
    }

    return res;
}

/*******************************************************************************
* Function Name  : GetIntegerInput
* Description    : Get an integer from the HyperTerminal
* Input          : - num: The inetger
* Output         : None
* Return         : 1: Correct
*                  0: Error
*******************************************************************************/
u32 GetIntegerInput(s32 * num)
{
    u8 inputstr[16];

    while (1)
    {
        GetInputString(inputstr);
        if (inputstr[0] == '\0') continue;
        if ((inputstr[0] == 'a' || inputstr[0] == 'A') && inputstr[1] == '\0')
        {
            SerialPutString("User stop \r\n");
            return 0;
        }

        if (Str2Int(inputstr, num) == 0)
        {
            SerialPutString("Error,Please put again! \r\n");
        }
        else
        {
            return 1;
        }
    }
}

/*******************************************************************************
* Function Name  : SerialKeyPressed
* Description    : Test to see if a key has been pressed on the HyperTerminal
* Input          : - key: The key pressed
* Output         : None
* Return         : 1: Correct
*                  0: Error
*******************************************************************************/
u32 SerialKeyPressed(u8 *key)
{
    /*RS485 */
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
    {
        *key = (u8)USART1->DR;
        return 1;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
* Function Name  : GetKey
* Description    : Get a key from the HyperTerminal
* Input          : None
* Output         : None
* Return         : The Key Pressed
*******************************************************************************/
u8 GetKey(void)
{
    u8 key = 0;
    /* Waiting for user input */
    while (1)
    {

        if (SerialKeyPressed((u8*)&key)) break;
    }
    return key;
}


/*******************************************************************************
* Function Name  : SerialPutChar
* Description    : Print a character on the HyperTerminal
* Input          : - c: The character to be printed
* Output         : None
* Return         : None
*******************************************************************************/
void SerialPutChar(u8 c)
{
    /*RS485 */
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    delay_msp();
    USART_SendData(USART1, c);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
    delay_msp();
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

/*******************************************************************************
* Function Name  : Serial_PutString
* Description    : Print a string on the HyperTerminal
* Input          : - s: The string to be printed
* Output         : None
* Return         : None
*******************************************************************************/
void Serial_PutString(u8 *s)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_8);
    delay_msp();
    while (*s != '\0')
    {
        USART_SendData(USART1, *s);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET);
        s ++;
    }
    delay_msp();
    GPIO_ResetBits(GPIOA, GPIO_Pin_8);
}

/*******************************************************************************
* Function Name  : GetInputString
* Description    : Get Input string from the HyperTerminal
* Input          : - buffP: The input string
* Output         : None
* Return         : None
*******************************************************************************/
void GetInputString (u8 * buffP)
{
    u32 bytes_read = 0;
    u8 c = 0;
    do
    {
        c = GetKey();
        if (c == '\r')
            break;
        if (c == '\b')   /* Backspace */
        {
            if (bytes_read > 0)
            {
                SerialPutString("\b \b");
                bytes_read --;
            }
            continue;
        }
        if (bytes_read >= CMD_STRING_SIZE )
        {
            SerialPutString("Command string size overflow\r\n");
            bytes_read = 0;
            continue;
        }
        if (c >= 0x20 && c <= 0x7E)
        {
            buffP[bytes_read++] = c;
            SerialPutChar(c);
        }
    }
    while (1);
    SerialPutString(("\n\r"));
    buffP[bytes_read] = '\0';
}

/*******************************************************************************
* Function Name  : FLASH_PagesMask
* Description    : Calculate the number of pages
* Input          : - Size: The image size
* Output         : None
* Return         : The number of pages
*******************************************************************************/
u32 FLASH_PagesMask(vu32 Size)
{
    u32 pagenumber = 0x0;
    u32 size = Size;

    if ((size % PAGE_SIZE) != 0)
    {
        pagenumber = (size / PAGE_SIZE) + 1;
    }
    else
    {
        pagenumber = size / PAGE_SIZE;
    }
    return pagenumber;

}

/*******************************************************************************
* Function Name  : FLASH_DisableWriteProtectionPages
* Description    : Disable the write protection of desired pages
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_DisableWriteProtectionPages(void)
{
    u32 useroptionbyte = 0, WRPR = 0;
    u16 var1 = OB_IWDG_SW, var2 = OB_STOP_NoRST, var3 = OB_STDBY_NoRST;
    FLASH_Status status = FLASH_BUSY;

    WRPR = FLASH_GetWriteProtectionOptionByte();

    /* Test if user memory is write protected */
    if ((WRPR & UserMemoryMask) != UserMemoryMask)
    {
        useroptionbyte = FLASH_GetUserOptionByte();

        UserMemoryMask |= WRPR;

        status = FLASH_EraseOptionBytes();

        if (UserMemoryMask != 0xFFFFFFFF)
        {
            status = FLASH_EnableWriteProtection((u32)~UserMemoryMask);
        }

        /* Test if user Option Bytes are programmed */
        if ((useroptionbyte & 0x07) != 0x07)
        {
            /* Restore user Option Bytes */
            if ((useroptionbyte & 0x01) == 0x0)
            {
                var1 = OB_IWDG_HW;
            }
            if ((useroptionbyte & 0x02) == 0x0)
            {
                var2 = OB_STOP_RST;
            }
            if ((useroptionbyte & 0x04) == 0x0)
            {
                var3 = OB_STDBY_RST;
            }

            FLASH_UserOptionByteConfig(var1, var2, var3);
        }

        if (status == FLASH_COMPLETE)
        {
            SerialPutString("Write Protection disabled...\r\n");

            SerialPutString("...and a System Reset will be generated to re-load the new option bytes\r\n");

            /* Enable WWDG clock */
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

            /* Generate a system Reset to re-load the new option bytes: enable WWDG and set
              counter value to 0x4F, as T6 bit is cleared this will generate a WWDG reset */
            WWDG_Enable(0x4F);
        }
        else
        {
            SerialPutString("Error: Flash write unprotection failed...\r\n");
        }
    }
    else
    {
        SerialPutString("Flash memory not write protected\r\n");
    }
}

/*******************************************************************************
* Function Name  : Main_Menu
* Description    : Display the Main Menu on to HyperTerminal
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Main_Menu(void)
{
    u8 key = 0;

    /* Get the number of block (4 or 2 pages) from where the user program will be loaded */
    BlockNbr = (FlashDestination - 0x08000000) >> 12;

    /* Compute the mask to test if the Flash memory, where the user program will be
       loaded, is write protected */
#ifdef USE_STM3210B_EVAL
    UserMemoryMask = ((u32)~((1 << BlockNbr) - 1));
#else /* USE_STM3210E_EVAL */
    if (BlockNbr < 62)
    {
        UserMemoryMask = ((u32)~((1 << BlockNbr) - 1));
    }
    else
    {
        UserMemoryMask = ((u32)0x80000000);
    }
#endif /* USE_STM3210B_EVAL */


    /* Test if any page of Flash memory where program user will be loaded is write protected */
    if ((FLASH_GetWriteProtectionOptionByte() & UserMemoryMask) != UserMemoryMask)
    {
        FlashProtection = TRUE;
        SerialPutString("\r\n==================== Main Menu ===========================\r\n\n");
        SerialPutString("  Download Image To the STM32F10x Internal Flash ------- 1\r\n\n");
        SerialPutString("  Execute The New Program ------------------------------ 2\r\n\n");
        SerialPutString("  Disable the write protection ------------------------- 3\r\n\n");
        SerialPutString("==========================================================\r\n\n");
    }
    else
    {
        FlashProtection = FALSE;
        SerialPutString("\r\n==================== Main Menu ===========================\r\n\n");
        SerialPutString("  Execute The New Program ------------------------------ 1\r\n\n");
        SerialPutString("  Disable the write protection ------------------------- 2\r\n\n");
        SerialPutString("==========================================================\r\n\n");
    }

    while (1)
    {
        key = GetKey();

        if (key == 0x31)
        {
            /* Download user application in the Flash */
            SerialDownload();
        }
        else if (key == 0x32)
        {
            JumpAddress = *(vu32*) (ApplicationAddress + 4);

            /* Jump to user application */
            Jump_To_Application = (pFunction) JumpAddress;
            /* Initialize user application's Stack Pointer */
            __MSR_MSP(*(vu32*) ApplicationAddress);
            Jump_To_Application();
        }
        else if ((key == 0x33) && (FlashProtection == TRUE))
        {
            /* Disable the write protection of desired pages */
            FLASH_DisableWriteProtectionPages();
        }
        else
        {
            if (FlashProtection == FALSE)
            {
                SerialPutString("Error input, please enter 1 or 2\r");
            }
            else
            {
                SerialPutString("Error input, please enter 1,2 or 3\r");
            }
        }
    }
}

/*******************(C)COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
