/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "oled.h"
#include "font.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
volatile int m_iOutputtype = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern volatile uint8_t rx_flag2; //禁止编译程序优化
extern volatile uint8_t tx_flag2;
extern volatile uint8_t TIM_flag;
uint8_t message[1]={0};
uint8_t JustFloatTial[4] = {0x00,0x00,0x80,0x7F};//每次数据发送完毕，必须发送Tial结束控制符
//波形参数
float A = 2.0;
float startx=-4.0;
float endx=4.0;
int N = 100;//每个周期的输出点数，数据10ms间隔时间
float T = 100*10.0;//ms
float stepx=6.28318530718f/100;//每点相位步进=2π/N，连续回绕不再折返
float fx=0;
float fy=0;
int m_iDir = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  m_iOutputtype = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(20); // 单片机启动比OLED上电快,需要延迟等待一下
  OLED_Init();
  HAL_UART_Receive_IT(&huart2,message,1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    //1、处理串口2接收中断回调
    if(rx_flag2==1)
    {
      if(message[0]=='1')
      {
        m_iOutputtype=0;
      }
      else if(message[0]=='2')
      {
        m_iOutputtype=1;
      }
      else if(message[0]=='3')
      {
        m_iOutputtype=2;
      }
      else{}
      rx_flag2=0;
      HAL_UART_Receive_IT(&huart2,message,1);
      HAL_UART_Transmit_IT(&huart2,message,1);
    }
     // 中英文字符串混合显示
     // 2.oled显示菜单
    OLED_NewFrame();
    switch (m_iOutputtype)
    {
    case 0:
      OLED_PrintString(0, 0, "Sine", &font16x16, OLED_COLOR_NORMAL);
      break;
    case 1:
      OLED_PrintString(0, 0, "Sawtooth", &font16x16, OLED_COLOR_NORMAL);
      break;
    case 2:
      OLED_PrintString(0, 0, "Square", &font16x16, OLED_COLOR_NORMAL);
      break;
    default:
      break;
    }
    

    OLED_ShowFrame();
    // 3.增加stepx,计算fy
    switch(m_iOutputtype)
    {
      case 0:
        fx+=stepx;
        if(fx>6.28318530718f){
          fx-=6.28318530718f;
        }
        fy=A*sin(fx);
        break;
        case 1:
          fx+=stepx;
          if(fx>6.28318530718f){
            fx-=6.28318530718f;
          }
          fy=A*(fx/3.14159265359f - 1.0f);
          break;
        case 2:

        fx+=stepx;
          if(fx<=(startx+endx)/2.0)
          {
            fy=A*(-1);
          }
          else
          {
            fy = A;
          }
          if(fx>=endx)
          {
            fx=startx;
          }
          break;
    }
    

    //4.串口2发送fy
    HAL_UART_Transmit_IT(&huart2,(unsigned char *)&fy,4);//串口2发送字符串,1个浮点数4个字节
    HAL_Delay(5);
    HAL_UART_Transmit_IT(&huart2,JustFloatTial,4);//串口2发送结束控制符
    //5.串口2发送结束控制符 0x00 0x00 0x80 0x7A
    HAL_Delay(5);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
