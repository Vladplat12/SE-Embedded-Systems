#include "RTG.h"
#include "periphery.h"

int data_Recevied_flag = 0;
client_address result_buffer;
data_to_client data_to_clients;
data recieved_data;
void rtg_main() {

	printf("Start of program\n\r");

	udpServer_init();
	while (1) {
		if (data_Recevied_flag == 0) {
			ethernetif_input(&gnetif);
			sys_check_timeouts();
		} else {
			data_Recevied_flag = 0;
			bool result = testing_programm();
			data_to_clients.Test_ID = recieved_data.Test_ID;
			if (result == true)
				memcpy(data_to_clients.Bit_pattern, "test succeeded\n\r",sizeof("Test succeeded"));
			else
				memcpy(data_to_clients.Bit_pattern, "Test failed\n\r",sizeof("Test failed"));
			send_to_client();
		}
	}
}

