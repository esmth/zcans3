/*
 * Copyright (c) 2018 Alexander Wachter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/byteorder.h>

#include "u8g2port.h"

//#define COUNTER_MSG_ID 0x12345
#define COUNTER_MSG_ID 0x345

const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

void canrx(const struct device *dev, struct can_frame *frame, void *user_data){
	//printf("canrx(): Counter received: %u\n", sys_be16_to_cpu(UNALIGNED_GET((uint16_t *)&frame->data)));
	printf("canrx(): id:%u dlc:%u flags:%u data:", frame->id, frame->dlc, frame->flags);
	for(int i=0; i<frame->dlc; i++){
		printf("%02x ", frame->data[i]);
	}
	printf("\n");
}

struct node {
	uint16_t d1, d2;
	struct node *next, *prev;
};
struct node *head, *tail;
unsigned int nodelen;

void node_del(){
	struct node *del = tail;
	tail = del->prev;
	k_free(del);
	nodelen--;
}
void node_ins(uint16_t d1, uint16_t d2){
	struct node *new = k_malloc(sizeof(struct node));
	new->d1 = d1;
	new->d2 = d2;
	new->next = head;
	head = new;
	if(nodelen==0) tail = new;
	nodelen++;
	if(nodelen > 144){
		node_del();
/*
		// delete node
		struct node *del = head;
		while(del->next->next != NULL){
			del = del->next;
		}
		k_free(del->next);
		del->next = NULL;
		nodelen--;
*/
	}
}

uint32_t mapu(uint32_t in, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max){
	return (in-in_min)*(out_max-out_min)/(in_max-in_min)+out_min;
}

volatile uint16_t lambda, temp, status;
volatile uint32_t spartan3rxcount;
void canrxlambda(const struct device *dev, struct can_frame *frame, void *user_data){
	//printf("canrxlambda(): id:%u dlc:%u flags:%u data: %02x %02x %02x %02x\n", frame->id, frame->dlc, frame->flags, frame->data[0],frame->data[1],frame->data[2],frame->data[3]);
	lambda = (uint16_t)frame->data[0]<<8 | (uint16_t)frame->data[1];
	temp = frame->data[2];
	status = frame->data[3];
	//printf("lambda:%u temp:%u status:%u\n", lambda, temp*10, status);
	//printf("lambda:%u temp:%u\n", lambda, temp*10);
	//printf("lambda:%f temp:%u\n", (float)lambda/1000.0, temp*10);
	spartan3rxcount++;
	node_ins(lambda, mapu(lambda, 800, 1200, 0, 167));
}


int main(void){
	int ret;
	printf("ethan hello\n");
	nodelen=0;
	head = NULL;

	if (!device_is_ready(can_dev)) {
		printf("CAN: Device %s not ready.\n", can_dev->name);
		return 0;
	}
	//ret = can_set_mode(can_dev, CAN_MODE_LOOPBACK);
	ret = can_set_mode(can_dev, CAN_MODE_NORMAL);
	if (ret != 0) {
		printf("Error setting CAN mode [%d]", ret);
		return 0;
	}
	ret = can_start(can_dev);
	if (ret != 0) {
		printf("Error starting CAN controller [%d]", ret);
		return 0;
	}
	//spartan 3 can filter
	const struct can_filter filter2 = { .flags=CAN_FILTER_DATA, .id=1024, .mask=CAN_STD_ID_MASK };
	can_add_rx_filter(can_dev, canrxlambda, NULL, &filter2);

	// u8g2
	u8x8initz();
	u8g2_t u8g2;
	//u8g2_Setup_ssd1309_128x64_noname0_f(&u8g2, U8G2_R0, u8x8bytez, u8x8gpioz);
	//u8g2_Setup_ssd1309_128x64_noname2_f(&u8g2, U8G2_R0, u8x8bytez, u8x8gpioz);
	u8g2_Setup_ls013b7dh05_144x168_f(&u8g2, U8G2_R0, u8x8bytez, u8x8gpioz);
	u8g2_InitDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 0);
	u8g2_SetFontMode(&u8g2, 1);
	u8g2_SetDrawColor(&u8g2, 2);


	char msg_lambda[20], msg_count[20], msg_avg[20];
	unsigned int count=0;
	struct node *cur;
	uint16_t tmp;
	uint32_t avg;

	while (1) {
/*
		enum can_state state;
		struct can_bus_err_cnt err_cnt = {0, 0};
		ret = can_get_state(can_dev, &state, &err_cnt);
		if (ret == 0) {
			printf("can state %d txe %d rxe %d\n", state, err_cnt.tx_err_cnt, err_cnt.rx_err_cnt);
		}
*/
/*
		UNALIGNED_PUT(sys_cpu_to_be16(counter), (uint16_t *)&counter_frame.data[0]);
		counter++;
		// This sending call is blocking until the message is sent.
		//can_send(can_dev, &counter_frame, K_MSEC(100), NULL, NULL);
*/

		sprintf(msg_lambda, "%04u", lambda);
		sprintf(msg_count, "%u %u %u %u", temp, count++, spartan3rxcount, nodelen);

		u8g2_ClearBuffer(&u8g2);

		//u8g2_SetFont(&u8g2, u8g2_font_logisoso38_tr);
		//u8g2_DrawStr(&u8g2, -3, 38, msg_lambda);
		u8g2_SetFont(&u8g2, u8g2_font_logisoso32_tr);
		u8g2_DrawStr(&u8g2, -2, 32, msg_lambda);

		u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
		u8g2_DrawStr(&u8g2, 0, 168, msg_count);

		u8g2_DrawHLine(&u8g2, 0, 83, 144);
		u8g2_DrawHLine(&u8g2, 0, 84, 144);

		avg=0;
		cur = head;
		for(int i=0; i<nodelen; i++){
			avg += cur->d1;
			u8g2_DrawPixel(&u8g2, i, cur->d2);
			u8g2_DrawPixel(&u8g2, i, cur->d2+1);
			cur = cur->next;
		}
		avg /= nodelen;

		sprintf(msg_avg, "%04u", avg);
		u8g2_SetFont(&u8g2, u8g2_font_logisoso22_tr);
		u8g2_DrawStr(&u8g2, 89, 22, msg_avg);

		u8g2_SendBuffer(&u8g2);

		k_sleep(K_MSEC(50));
	}
}
