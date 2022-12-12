#ifndef INC_RTG_H_
#define INC_RTG_H_

#include "main.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwip.h"
#include "lwip/opt.h"
#include "udp.h"

#define MAX_PACKET_SIZE 266
#define Buff_size 255
#define SERVER_PORT 7
#define TESTING_NUM_FOR_ADC 1000
#define deviation_ADC 0.05
#define DEVICION_TO_10Parts 10
#define DEVICION_TO_2Parts 2
#define MULT_FROM10_Parts 100
#define MULT_FROM2_Parts 500
#define UART_DEBUG &huart3
#define UART_5 &huart5
#define UART_6 &huart6
#define I2C_4 &hi2c4
#define I2C_2 &hi2c2
#define TIM_6 &htim6
#define TIM_7 &htim7
#define TIM_10 &htim10

#define DAC_1 &hdac
#define ADC_1 &hadc1

#define I2C_PORT 44
#define TimeOut 100

extern UART_HandleTypeDef huart3;
extern struct netif gnetif;
extern int data_Recevied_flag;

typedef struct Data {
	uint32_t Test_ID;
	uint8_t peripherial_to_be_tested;
	uint8_t Iterations;
	uint8_t Bit_pattern_length;
	uint8_t Bit_pattern[Buff_size];
} data;

extern data recieved_data;

typedef struct Client_address {
	struct udp_pcb *upcb;
	ip_addr_t addr;
	u16_t port;
	uint8_t len;
} client_address;

typedef struct Data_to_client {
	uint32_t Test_ID;
	uint8_t Bit_pattern[Buff_size];
} data_to_client;

typedef enum {
	false, true
} bool;

extern client_address result_buffer;
extern data_to_client data_to_clients;

extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;
extern I2C_HandleTypeDef hi2c4;
extern I2C_HandleTypeDef hi2c2;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim10;
extern DAC_HandleTypeDef hdac;
extern ADC_HandleTypeDef hadc1;

bool I2C_testing(void);
bool UART_testing(void) ;
bool testing_programm(void);
bool ADC_testing(int iteration);

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
bool is_valid_data(void);
void send_to_client(void);
void udpServer_init(void);

void rtg_main();
#endif /* INC_RTG_H_ */
