/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma location=0x30000000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30000200
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30000000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30000200))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
#endif

ETH_TxPacketConfig TxConfig;

ETH_HandleTypeDef heth;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ETH_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


// from tutorial https://youtu.be/sPzQ5CniWtw?si=RY3PwykBVtkKE4jr
int _write(int file, char *ptr, int len) {
	int i = 0;
	for (i = 0; i < len; i++) {
		ITM_SendChar((*ptr++));
	}
	return len;
}

typedef struct INPUT {
    int target_index;
    double value;
    struct INPUT* next;
    struct INPUT* prev;
} INPUT;

typedef struct INPUT_QUEUE {
    INPUT* head;
    INPUT* tail;
    int size;
    int max_size;
} INPUT_QUEUE;

INPUT_QUEUE* create_input_queue(int max_size) {
    INPUT_QUEUE* input_queue = (INPUT_QUEUE*)malloc(sizeof(INPUT_QUEUE));
    input_queue->head = NULL;
    input_queue->tail = NULL;
    input_queue->size = 0;
    input_queue->max_size = max_size;
    return input_queue;
}

void insert_input(INPUT_QUEUE* input_queue, int target_index, double value) {

    if (input_queue->max_size -1 < target_index) {
        printf("Error: target index is out of range\n");
        return;
    }

    INPUT* new_input = (INPUT*)malloc(sizeof(INPUT));
    new_input->target_index = target_index;
    new_input->value = value;
    new_input->next = NULL;
    new_input->prev = NULL;

    if (input_queue->size == 0) {
        input_queue->head = new_input;
        input_queue->tail = new_input;
    } else {
        INPUT* cur_input = input_queue->head;
        while (cur_input != NULL) {
            if (cur_input->target_index == target_index) {
                cur_input->value += value;
                return;
            } else {
                cur_input = cur_input->next;
            }
        }
        input_queue->tail->next = new_input;
        new_input->prev = input_queue->tail;
        input_queue->tail = new_input;

    }
    input_queue->size++;
}

void clear_input_queue(INPUT_QUEUE* input_queue) {
    INPUT* cur_input = input_queue->head;
    while (cur_input != NULL) {
        INPUT* next_input = cur_input->next;
        free(cur_input);
        cur_input = next_input;
    }
    input_queue->head = NULL;
    input_queue->tail = NULL;
    input_queue->size = 0;
}


////////////////////// GLOBAL CONSTANTS //////////////////////////////
const double RP = 0.1;   // resting potential
const double WTH = 0.01; // threshold for weights to count as connection
const double ATH = 1.0;  // threshold for membrane potential to spike
const double DT = 0.1;   // time step
const double MAX_TIME = 7.0; // maximum time for simulation
const double TAU = 2.0;  // time constant
const double TIME_TOLERANCE = 0.0001; // tolerance for time comparisons
//////////////////////////////////////////////////////////////////////


////////////////////// LIFNeuron struct //////////////////////////////
typedef struct {
    double mp; // membrane potential
    double ts; // time-stamp of last spike
} LIFNeuron;
//////////////////////////////////////////////////////////////////////


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  // Define the number of neurons in each layer
  int layer_sizes[] = {16, 20, 20, 100, 20, 4};
  int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);




  // Allocate memory for the array of layers
  LIFNeuron** layers = (LIFNeuron**)malloc(num_layers * sizeof(LIFNeuron*));

  // Allocate memory for the array of weight matrices
  double*** weights = (double***)malloc((num_layers - 1) * sizeof(double**));

  // Allocate memory for and initialize weights
  for (int i = 0; i < num_layers - 1; i++) {
      weights[i] = (double**)malloc(layer_sizes[i] * sizeof(double*));
      for (int j = 0; j < layer_sizes[i]; j++) {
          weights[i][j] = (double*)malloc(layer_sizes[i + 1] * sizeof(double));
          for (int k = 0; k < layer_sizes[i + 1]; k++) {
              weights[i][j][k] = 0.0; // You can initialize weights as needed
          }
      }
  }

  // manually setting some weights
  weights[0][0][0] = 0.9566007951763376;
  weights[0][1][0] = 0.8124301063990329;
  weights[0][2][1] = 0.7366283239748567;
  weights[0][3][3] = 0.796525367655762;
  weights[0][4][4] = 0.9873246941500416;
  weights[0][5][16] = 0.7745873263742765;
  weights[0][6][19] = 0.8891613941035854;
  weights[0][7][3] = 0.9161894407462406;
  weights[0][8][0] = 0.7530420773023409;
  weights[0][9][1] = 0.6401370211460118;
  weights[0][10][7] = 0.9542160048036938;
  weights[0][11][10] = 0.7385716940068183;
  weights[0][12][11] = 0.8147650047705076;
  weights[0][13][8] = 0.6105148952250679;
  weights[0][14][8] = 0.9158494226216416;
  weights[0][15][9] = 0.7945255311358245;

  weights[1][0][0] = 0.8;
  weights[1][0][1] = 0.5;
  weights[1][0][0] = 0.2;

  weights[2][0][0] = 0.3;
  weights[2][0][1] = 0.98;
  weights[2][0][0] = 0.44;

  weights[3][0][0] = 0.8;
  weights[3][0][1] = 0.3;
  weights[3][0][0] = 0.5;

  weights[3][0][0] = 0.4;
  weights[3][19][2] = 0.8;
  weights[3][0][2] = 0.9;



  // Allocate memory for and initialize neurons
  for (int i = 0; i < num_layers; i++) {
      layers[i] = (LIFNeuron*)malloc(layer_sizes[i] * sizeof(LIFNeuron));
      for (int j = 0; j < layer_sizes[i]; j++) {
          layers[i][j].mp = RP;
          layers[i][j].ts = 0.0;
      }
  }



  // Create a queue of input queues for each layer
  INPUT_QUEUE** input_queues = (INPUT_QUEUE**)malloc(num_layers * sizeof(INPUT_QUEUE*));
  for (int i = 0; i < num_layers; i++) {
      input_queues[i] = create_input_queue(layer_sizes[i]);
  }




  ////////////////////////////////////// SETUP //////////////////////////////////////
  long double time = 0.0;
  int counter = 0;

  int output_buffer[layer_sizes[num_layers - 1]];
  for (int i = 0; i < layer_sizes[num_layers - 1]; i++) {
      output_buffer[i] = 0;
  }

  /////////////////////////////////// MAIN LOOP //////////////////////////////////////
  while (time < MAX_TIME) {



      if (counter % 10 == 0) {
          insert_input(input_queues[0], 0, 1.0);
      }




      // printf("\nPerforming updates...\n");

      for (int layer_idx = 0; layer_idx < num_layers; layer_idx++) {
          INPUT* current = input_queues[layer_idx]->head;
          while (current != NULL) {

              int to_update_index = current->target_index; // The index of the neuron to update
              double to_update_value = current->value; // The input value to the neuron

              LIFNeuron* neuron_to_update = &layers[layer_idx][to_update_index];

              double time_since_last_update = time - neuron_to_update->ts;



              // Calculate the new membrane potential after it has been leaking since the last update
              neuron_to_update->mp = RP + (neuron_to_update->mp - RP) * exp(-time_since_last_update / TAU);
              // Add the input and update the time stamp
              neuron_to_update->mp += to_update_value;
              neuron_to_update->ts = time;



              // Check for spike
              if (neuron_to_update->mp > ATH) {



                  neuron_to_update->mp = RP; // Reset the membrane potential



                  // If this is not the last layer, update the inputs to the next layer
                  if (layer_idx < num_layers - 1) {
                      for (int weight_idx = 0; weight_idx < layer_sizes[layer_idx + 1]; weight_idx++) {
                          double weight = weights[layer_idx][to_update_index][weight_idx];
                          if (weight > WTH) {

                              insert_input(input_queues[layer_idx + 1], weight_idx, weight);
                          }
                      }
                  } else {
                      output_buffer[to_update_index] = 1;
                  }
              }
              current = current->next; // Move to the next input in the queue
          }
          // Reset this layer's input queue
          clear_input_queue(input_queues[layer_idx]);
      }

      // Check the output buffer
      for (int i = 0; i < layer_sizes[num_layers - 1]; i++) {
          // printf("Output %d: %d\n", i, output_buffer[i]);
          output_buffer[i] = 0; // Reset this output
      }

      // Advance time
      time += DT;
      counter++;
  }



  //////////////////////////////////////////// CLEANUP ////////////////////////////////////////////


  // Free layers list
  for (int i = 0; i < num_layers; i++) {
      free(layers[i]);
  }

  // Free input lists
  for (int i = 0; i < num_layers; i++) {
      free(input_queues[i]);
  }

  // Free weights list
  for (int i = 0; i < num_layers - 1; i++) {
      for (int j = 0; j < layer_sizes[i]; j++) {
          free(weights[i][j]);
      }
      free(weights[i]);
  }

  // Free layer sizes OBS double check this
  free(layers);
  free(input_queues);
  free(weights);


  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  HAL_GPIO_TogglePin(MyGreenLed_GPIO_Port, MyGreenLed_Pin);
	  printf("Hello!\n");
	  HAL_Delay(400);

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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 24;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 9;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.battery_charging_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, MyGreenLed_Pin|LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_OTG_FS_PWR_EN_GPIO_Port, USB_OTG_FS_PWR_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : MyGreenLed_Pin LD3_Pin */
  GPIO_InitStruct.Pin = MyGreenLed_Pin|LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OTG_FS_PWR_EN_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_OTG_FS_PWR_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OTG_FS_OVCR_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_OVCR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OTG_FS_OVCR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/*
int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}
*/

/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
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
