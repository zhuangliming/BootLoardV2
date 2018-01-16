/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Main program body
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
extern pFunction Jump_To_Application;
extern u32 JumpAddress;
ErrorStatus HSEStartUpStatus;

void RCC_Configuration(void);
void IAP_Init(void);
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#define IAP_LED_ON   GPIO_SetBits(GPIOB, GPIO_Pin_0)
#define IAP_LED_OFF  GPIO_ResetBits(GPIOB, GPIO_Pin_0)

static void delay_ms(u16 time)
{
    u16 i=0,j;
    for(i=time; i>0; i--)
        for(j=1000; j>0; j--);
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
    u8 user_key=0,i;
#ifdef DEBUG
    debug();
#endif
    FLASH_Unlock();			// Flash unlock
    IAP_Init();
    SerialPutString("\r\n----------------------------------------------------------");
    SerialPutString("\r\n              Cortex-M3 System Update V1.0                ");
    SerialPutString("\r\n  Note: this operation will upgrade your system firmware! ");
    SerialPutString("\r\n                                           --ZhuangLiMing ");
    SerialPutString("\r\n----------------------------------------------------------");
    SerialPutString("\r\n\r\n");
    /* Infinite loop */
    for(i=0; i<100; i++)
    {
        if(SerialKeyPressed(&user_key))
        {
            if(user_key == 'z')
            {
                Main_Menu ();
                i=101;
            }
        }
        delay_ms(100);
    }
    if (((*(vu32*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
    {
        // Jump to user application
        IAP_LED_OFF;
        JumpAddress = *(vu32*) (ApplicationAddress + 4);
        Jump_To_Application = (pFunction) JumpAddress;
        __MSR_MSP(*(vu32*) ApplicationAddress);   // Initialize user application's Stack Pointer
        Jump_To_Application();
    }
    while (1) {}
}

/*******************************************************************************
* Function Name  : IAP_Init
* Description    : Initialize the IAP: Configure RCC, USART and GPIOs.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IAP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* Clock configuration -------------------------------------------------------*/
    RCC_Configuration();

    /* LED	   */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;//LED
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    IAP_LED_OFF;
    /* Configure the GPIO ports( USART2 Transmit and Receive Lines) */
    /* Configure the USART2_Tx as Alternate function Push-Pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure the USART2_Rx as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    IAP_LED_ON;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART2 configuration ------------------------------------------------------*/
    /* USART2 configured as follow:
          - BaudRate = 115200 baud
          - Word Length = 8 Bits
          - One Stop Bit
          - No parity
          - Hardware flow control disabled (RTS and CTS signals)
          - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = 19200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure the USART2 */
    USART_Init(USART1, &USART_InitStructure);

    /* Enable the USART2 */
    USART_Cmd(USART1, ENABLE);
}

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        /* Flash 2 wait state */
        FLASH_SetLatency(FLASH_Latency_2);

        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK */
        RCC_PCLK2Config(RCC_HCLK_Div1);

        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config(RCC_HCLK_Div2);

        /* PLLCLK = 8MHz * 9 = 72 MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

        /* Enable PLL */
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }

    /* Enable GPIOA, GPIOC and USART2 clock  */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
