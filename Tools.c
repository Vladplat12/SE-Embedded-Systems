#include "RTG.h"

uint8_t uart5_flag_calback;
uint8_t uart6_flag_calback;
uint8_t I2C2_flag_callback = 0;
uint8_t I2C4_flag_callback = 0;

uint8_t milli100Second;
uint8_t seconds;
uint8_t milli500Second;

uint8_t buffRead[Buff_size];
uint8_t buffWrite[Buff_size];
uint8_t buff_for_compering[Buff_size];

int adc_flag = 0;
void send_to_client(void) {

	struct pbuf *txBuf = pbuf_alloc(PBUF_TRANSPORT, sizeof(data_to_clients),
			PBUF_RAM);
	pbuf_take(txBuf, &data_to_clients, sizeof(data_to_clients));
	udp_connect(result_buffer.upcb, &result_buffer.addr, result_buffer.port);
	udp_send(result_buffer.upcb, txBuf);
	udp_disconnect(result_buffer.upcb);
	pbuf_free(txBuf);
}
void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p,
		const ip_addr_t *addr, u16_t port) {

	char buf[100];
	const int len = sprintf(buf, "%s\n", (char*) p->payload);
	memcpy(&recieved_data, p->payload, p->len);

	result_buffer.upcb = upcb;
	result_buffer.addr = *addr;
	result_buffer.port = port;

	if (is_valid_data() && (p->len < MAX_PACKET_SIZE) && (p->len > -1))
		data_Recevied_flag = 1;
	else {
		data_to_clients.Test_ID = recieved_data.Test_ID;
		memcpy(data_to_clients.Bit_pattern, "Error", sizeof("Error"));
		send_to_client();
	}
	struct pbuf *txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
	pbuf_take(txBuf, buf, len);
	udp_connect(upcb, addr, port);
	udp_disconnect(upcb);
	pbuf_free(txBuf);
	pbuf_free(p);
}

bool is_valid_data(void) {
	if (recieved_data.Bit_pattern_length > Buff_size)
		return false;
	else if (recieved_data.Iterations < 1)
		return false;
	else if (recieved_data.Bit_pattern_length < 1)
		return false;
	else if (recieved_data.Test_ID < 1)
		return false;
	return true;
}
void init_timer(void) {
	milli100Second = 0;
	seconds = 0;
	milli500Second = 0;
}

bool testing_programm(void) {
	bool result = false;
	init_timer();
	switch (recieved_data.peripherial_to_be_tested) {
	case 2:
		HAL_TIM_Base_Start_IT(TIM_6);
		result = UART_testing();
		HAL_TIM_Base_Stop_IT(TIM_6);
		//printf("milliSecond=%d  Seconds=%d\n\r", milli100Second * MULT_FROM10_Parts, seconds);
		init_timer();
		HAL_TIM_Base_Start_IT(TIM_7);
		result = UART_testing();
		HAL_TIM_Base_Stop_IT(TIM_7);
		//printf("milliSecond=%d  Seconds=%d\n\r", milli500Second * MULT_FROM2_Parts, seconds);
		break;

	case 8:
		HAL_TIM_Base_Start_IT(TIM_6);
		HAL_TIM_Base_Stop_IT(TIM_6);
		result = I2C_testing();
		//printf("milliSecond=%d  Seconds=%d\n\r", milli100Second * MULT_FROM10_Parts, seconds);
		init_timer();
		HAL_TIM_Base_Start_IT(TIM_7);
		result = I2C_testing();
		HAL_TIM_Base_Stop_IT(TIM_7);
		//printf("milliSecond=%d  Seconds=%d\n\r", milli500Second *MULT_FROM2_Parts, seconds);
		break;
	case 16:
		HAL_DAC_SetValue(DAC_1, DAC_CHANNEL_1, DAC_ALIGN_12B_R,
		TESTING_NUM_FOR_ADC);
		HAL_DAC_Start(DAC_1, DAC_CHANNEL_1);
		result = ADC_testing(recieved_data.Iterations);
		break;
	}
	return result;
}

bool ADC_testing(int iteration) {
	int Standard_deviation = TESTING_NUM_FOR_ADC * deviation_ADC;
	HAL_TIM_Base_Start_IT(TIM_10);

	for (int i = 0; i < iteration; i++) {
		if (adc_flag) {
			printf("DAC value=%d >> ADC value =%ld \n\r", TESTING_NUM_FOR_ADC,
					HAL_ADC_GetValue(ADC_1)); //Sample the DAC value using the ADC
			adc_flag = 0;
			if (HAL_ADC_GetValue(ADC_1)
					< TESTING_NUM_FOR_ADC - Standard_deviation
					|| TESTING_NUM_FOR_ADC + Standard_deviation
							< HAL_ADC_GetValue(ADC_1)) {
				HAL_TIM_Base_Stop_IT(TIM_10);
				return false;
			}
		}
	}
	HAL_TIM_Base_Stop_IT(TIM_10);
	return true;
}

// ADC interrupt Callback (convert data finish)
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	adc_flag = 1;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == TIM_10)
		HAL_ADC_Start_IT(&hadc1);
	if (htim == TIM_6) {
		milli100Second++;
		if (milli100Second >= DEVICION_TO_10Parts) {
			milli100Second = 0;
			seconds++;
		}
	}
	if (htim == TIM_7) {
		milli500Second++;
		if (milli500Second >= DEVICION_TO_2Parts) {
			milli500Second = 0;
			seconds++;
		}
	}
}

bool I2C_testing(void) {
	//init_timer();
	HAL_TIM_Base_Start_IT(TIM_6);
	HAL_I2C_Slave_Receive_DMA(I2C_2, buffRead,
			recieved_data.Bit_pattern_length);
	HAL_I2C_Master_Transmit(I2C_4, I2C_PORT, recieved_data.Bit_pattern,
			recieved_data.Bit_pattern_length, TimeOut);
	for (int i = 0; i < recieved_data.Iterations; i++) {
		if (I2C2_flag_callback == 1) {
			//printf("In first I2C: %s\n\r", buffRead);
			//init_timer();
			if (memcmp(recieved_data.Bit_pattern, buffRead,
					recieved_data.Bit_pattern_length))
				return false;
			memset(buffWrite, 0, Buff_size);
			memcpy(buffWrite, buffRead, Buff_size);
			HAL_I2C_Master_Receive_DMA(I2C_4, I2C_PORT, buffRead,
					recieved_data.Bit_pattern_length);
			HAL_I2C_Slave_Transmit(I2C_2, buffWrite,
					recieved_data.Bit_pattern_length,
					TimeOut);
			I2C2_flag_callback = 0;
		}
		if (I2C4_flag_callback == 1) {
			//init_timer();
			//printf("In second I2C: %s\n\r", buffRead);
			HAL_I2C_Slave_Receive_DMA(I2C_2, buffRead,
					recieved_data.Bit_pattern_length);
			memset(buffWrite, 0, Buff_size);
			memcpy(buffWrite, buffRead, recieved_data.Bit_pattern_length);
			HAL_I2C_Master_Transmit(I2C_4, I2C_PORT, buffWrite,
					recieved_data.Bit_pattern_length,
					TimeOut);
			I2C4_flag_callback = 0;
		}
	}
	return true;
}

bool UART_testing(void) {
	uart5_flag_calback = 0;
	uart6_flag_calback = 0;
	HAL_UART_Receive_DMA(UART_5, buffRead, recieved_data.Bit_pattern_length);
	HAL_UART_Transmit(UART_6, recieved_data.Bit_pattern,
			recieved_data.Bit_pattern_length, TimeOut);
	memcpy(buff_for_compering, buffRead, Buff_size);

	for (int i = 0; i < recieved_data.Iterations; i++) {
		if (uart5_flag_calback) {
			uart5_flag_calback = 0;
			//printf("In First UART: %s\n\r", buffRead);
			if (memcmp(buffRead, buff_for_compering,
					recieved_data.Bit_pattern_length))
				return false;
			HAL_UART_Receive_DMA(UART_6, buffRead,
					recieved_data.Bit_pattern_length);
			memset(buffWrite, 0, Buff_size);
			memcpy(buffWrite, buffRead, recieved_data.Bit_pattern_length);
			HAL_UART_Transmit(UART_5, buffWrite,
					recieved_data.Bit_pattern_length,
					TimeOut);
		}
		if (uart6_flag_calback) {
			//printf("In second UART: %s\n\r", buffRead);
			HAL_UART_Receive_DMA(UART_5, buffRead,
					recieved_data.Bit_pattern_length);
			uart6_flag_calback = 0;
			memset(buffWrite, 0, recieved_data.Bit_pattern_length);
			memcpy(buffWrite, buffRead, recieved_data.Bit_pattern_length);
			HAL_UART_Transmit(UART_6, buffWrite,
					recieved_data.Bit_pattern_length,
					TimeOut);
		}
	}
	return true;
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	if (hi2c == I2C_2) {
		I2C2_flag_callback = 1;
	}
}
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	if (hi2c == I2C_4) {
		I2C4_flag_callback = 1;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == UART_6)
		uart6_flag_calback = 1;
	if (huart == UART_5)
		uart5_flag_calback = 1;
}

// printf
int __io_putchar(int ch) {
	HAL_UART_Transmit(UART_DEBUG, (uint8_t*) &ch, 1, 0xFFFF);
	return ch;
}

int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(UART_DEBUG, (uint8_t*) ptr, len, 0xFFFF);
	return len;
}

// scanf
int _read(int file, char *ptr, int len) {
	int ch = 0;
	HAL_UART_Receive(UART_DEBUG, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	HAL_UART_Transmit(UART_DEBUG, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	if (ch == 13) {
		ch = 10;
		HAL_UART_Transmit(UART_DEBUG, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	} else if (ch == 8) {
		ch = 0x30;
		HAL_UART_Transmit(UART_DEBUG, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	}
	*ptr = ch;
	return 1;
}
void udpServer_init(void) {
	struct udp_pcb *upcb = udp_new();
	err_t err = udp_bind(upcb, IP_ADDR_ANY, SERVER_PORT);
	if (err == ERR_OK) {
		udp_recv(upcb, udp_receive_callback, NULL);
	} else {
		udp_remove(upcb);
	}
}
