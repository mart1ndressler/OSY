// **************************************************************************
//
//               Demo program for OSY labs
//
// Subject:      Operating systems
// Author:       Petr Olivka, petr.olivka@vsb.cz, 02/2022
// Organization: Department of Computer Science, FEECS,
//               VSB-Technical University of Ostrava, CZ
//
// File:         Task control demo program.
//
// **************************************************************************
//
// All mapped LEDs and switches and their PINs and GPIOs:
// See schema in APPS syllabus.
//
// Switches:
// Name		PIN				GPIO
// PTC9		SW_PTC9_PIN		SW_PTC9_GPIO
// PTC10	SW_PTC10_PIN	SW_PTC10_GPIO
// PTC11	SW_PTC11_PIN	SW_PTC11_GPIO
// PTC12	SW_PTC12_PIN	SW_PTC12_GPIO
//
// LEDs:
// Name		PIN				GPIO
// PTA1		LED_PTA1_PIN   LED_PTA1_GPIO
// PTA2		LED_PTA2_PIN   LED_PTA2_GPIO
//
// PTC0		LED_PTC0_PIN   LED_PTC0_GPIO
// PTC1		LED_PTC1_PIN   LED_PTC1_GPIO
// PTC2		LED_PTC2_PIN   LED_PTC2_GPIO
// PTC3		LED_PTC3_PIN   LED_PTC3_GPIO
// PTC4		LED_PTC4_PIN   LED_PTC4_GPIO
// PTC5		LED_PTC5_PIN   LED_PTC5_GPIO
// PTC7		LED_PTC7_PIN   LED_PTC7_GPIO
// PTC8		LED_PTC8_PIN   LED_PTC8_GPIO
//
// PTB2		LED_PTB2_PIN   LED_PTB2_GPIO
// PTB3		LED_PTB3_PIN   LED_PTB3_GPIO
// PTB9		LED_PTB9_PIN   LED_PTB9_GPIO
// PTB10	LED_PTB10_PIN  LED_PTB10_GPIO
// PTB11	LED_PTB11_PIN  LED_PTB11_GPIO
// PTB18	LED_PTB18_PIN  LED_PTB18_GPIO
// PTB19	LED_PTB19_PIN  LED_PTB19_GPIO
// PTB20	LED_PTB20_PIN  LED_PTB20_GPIO
// PTB23	LED_PTB23_PIN  LED_PTB23_GPIO

// FreeRTOS kernel includes.
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

// System includes.
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"

// Task priorities.
#define LOW_TASK_PRIORITY 		(configMAX_PRIORITIES - 2)
#define NORMAL_TASK_PRIORITY 	(configMAX_PRIORITIES - 1)
#define HIGH_TASK_PRIORITY 		(configMAX_PRIORITIES)

#define TASK_NAME_SWITCHES		"switches"
#define TASK_NAME_LED_PTA		"led_pta"
//#define TASK_NAME_LED_SNAKE_L	"led_snake_l"
//#define TASK_NAME_LED_SNAKE_R	"led_snake_r"
#define TASK_NAME_LED_BRIGHTNESS "led_brightness"
#define TASK_NAME_SWITCH_ALL "swith_all"
#define TASK_NAME_SWITCH_ALL_OFF "swith_all_off"

#define LED_PTA_NUM 	2
#define LED_PTC_NUM		8
#define LED_PTB_NUM		9

struct LED_Data
{
	uint32_t m_led_pin;
	GPIO_Type *m_led_gpio;
};
struct RGB_Data
{
	LED_Data red;
	LED_Data green;
	LED_Data blue;

};

RGB_Data rgbl = { { LED_PTB2_PIN, LED_PTB2_GPIO }, { LED_PTB3_PIN, LED_PTB3_GPIO }, { LED_PTB9_PIN, LED_PTB9_GPIO } };
RGB_Data rgbm = { { LED_PTB10_PIN, LED_PTB10_GPIO }, { LED_PTB11_PIN,LED_PTB11_GPIO }, { LED_PTB18_PIN, LED_PTB18_GPIO } };
RGB_Data rgbr = { { LED_PTB19_PIN, LED_PTB19_GPIO }, { LED_PTB20_PIN,LED_PTB20_GPIO }, { LED_PTB23_PIN, LED_PTB23_GPIO } };

int br, midbr= 50;

LED_Data g_led_pta[ LED_PTA_NUM ] =
{
        { LED_PTA1_PIN, LED_PTA1_GPIO },
        { LED_PTA2_PIN, LED_PTA2_GPIO },
};

LED_Data g_led_ptc[ LED_PTC_NUM ] =
{
        { LED_PTC0_PIN, LED_PTC0_GPIO },
        { LED_PTC1_PIN, LED_PTC1_GPIO },
        { LED_PTC2_PIN, LED_PTC2_GPIO },
        { LED_PTC3_PIN, LED_PTC3_GPIO },
        { LED_PTC4_PIN, LED_PTC4_GPIO },
        { LED_PTC5_PIN, LED_PTC5_GPIO },
        { LED_PTC7_PIN, LED_PTC7_GPIO },
        { LED_PTC8_PIN, LED_PTC8_GPIO },
};


bool clicked = false;
bool clickedOff = false;
bool allOn = false;

void ledsON()
{
		for(int i = 0 ; i < LED_PTC_NUM ; i++)GPIO_PinWrite(g_led_pta[i].m_led_gpio, g_led_pta[i].m_led_pin,1);
}
void ledsoff(){

	for(int i = 0 ; i < LED_PTC_NUM ; i++)GPIO_PinWrite(g_led_pta[i].m_led_gpio, g_led_pta[i].m_led_pin,0);
}

void task_doubleclick(void *t_arg)
{
	int cooldown = 200;
	while(1)
	{
		if(clicked)
		{
			clicked = false;
			vTaskDelay(5);
			for(int i = 0; i < cooldown; i++)
			{
				if(clicked)
				{
					clicked = false;
					ledsON();
					break;
				}
				vTaskDelay(2);
			}
		}
		vTaskDelay(2);
	}
}
void task_doubleclickOff(void *t_arg)
{
	int cooldown = 200;
	while(1)
	{
		if(clickedOff)
		{
			clickedOff = false;
			vTaskDelay(5);
			for(int i = 0; i < cooldown; i++)
			{
				if(clickedOff)
				{
					clickedOff = false;
					ledsoff();
					break;
				}
				vTaskDelay(2);
			}
		}
		vTaskDelay(2);
	}
}

void task_led_pta_blink(void *t_arg)
{
	uint32_t l_inx = 0;
	while(1)
	{
		GPIO_PinWrite(g_led_pta[l_inx].m_led_gpio, g_led_pta[l_inx].m_led_pin,1);
		vTaskDelay(200);
		GPIO_PinWrite(g_led_pta[l_inx].m_led_gpio, g_led_pta[l_inx].m_led_pin,0);
		l_inx++;
		l_inx %= LED_PTA_NUM;
	}
}

void rgb_led_br(void *t_arg)
{
	int cnt = 0;

	while (1)
	{
		if(cnt < br / 5)
		{
			GPIO_PinWrite(rgbl.red.m_led_gpio, rgbl.red.m_led_pin, 1);
			GPIO_PinWrite(rgbl.blue.m_led_gpio, rgbl.blue.m_led_pin, 1);
			GPIO_PinWrite(rgbl.green.m_led_gpio, rgbl.green.m_led_pin, 1);

		}
		else
		{
			GPIO_PinWrite(rgbl.red.m_led_gpio, rgbl.red.m_led_pin, 0);
			GPIO_PinWrite(rgbl.blue.m_led_gpio, rgbl.blue.m_led_pin, 0);
			GPIO_PinWrite(rgbl.green.m_led_gpio, rgbl.green.m_led_pin, 0);
		}

		if(cnt < (100 - midbr) / 5)
		{
			GPIO_PinWrite(rgbr.red.m_led_gpio, rgbr.red.m_led_pin, 1);
			GPIO_PinWrite(rgbr.blue.m_led_gpio, rgbr.blue.m_led_pin, 1);
			GPIO_PinWrite(rgbr.green.m_led_gpio, rgbr.green.m_led_pin, 1);
		}
		else
		{
			GPIO_PinWrite(rgbr.red.m_led_gpio, rgbr.red.m_led_pin, 0);
			GPIO_PinWrite(rgbr.blue.m_led_gpio, rgbr.blue.m_led_pin, 0);
			GPIO_PinWrite(rgbr.green.m_led_gpio, rgbr.green.m_led_pin, 0);
		}

		if (cnt < midbr / 5)
		{
			GPIO_PinWrite(rgbm.red.m_led_gpio, rgbm.red.m_led_pin, 1);
			GPIO_PinWrite(rgbm.blue.m_led_gpio, rgbm.blue.m_led_pin, 1);
			GPIO_PinWrite(rgbm.green.m_led_gpio, rgbm.green.m_led_pin, 1);

		} else {
			GPIO_PinWrite(rgbm.red.m_led_gpio, rgbm.red.m_led_pin, 0);
			GPIO_PinWrite(rgbm.blue.m_led_gpio, rgbm.blue.m_led_pin, 0);
			GPIO_PinWrite(rgbm.green.m_led_gpio, rgbm.green.m_led_pin, 0);
		}
		vTaskDelay(1);
		cnt += 1;
		cnt %= 21;
	}

}

void chgbr(int change)
{
	midbr += change;
	if(midbr < 0) midbr = 0;
	if(midbr > 100) midbr = 100;
}
void chgmidbr(int change)
{
	midbr += change;
	if(midbr < 0) midbr = 0;
	if(midbr > 100) midbr = 100;
}

//void task_snake_left(void *t_arg)
//{
//	while(1)
//	{
//		vTaskSuspend(0);
//
//		for (int i = 0; i < LED_PTC_NUM; i++)
//		{
//			GPIO_PinWrite(g_led_ptc[i].m_led_gpio, g_led_ptc[i].m_led_pin,1);
//			vTaskDelay(200);
//			GPIO_PinWrite(g_led_ptc[i].m_led_gpio, g_led_ptc[i].m_led_pin,0);
//		}
//	}
//}
//
//void task_snake_right(void *t_arg) {
//	while(1)
//	{
//		vTaskSuspend(0);
//
//		for(int i = LED_PTC_NUM - 1; i >= 0; i--)
//		{
//			GPIO_PinWrite(g_led_ptc[i].m_led_gpio, g_led_ptc[i].m_led_pin,1);
//			vTaskDelay(200);
//			GPIO_PinWrite(g_led_ptc[i].m_led_gpio, g_led_ptc[i].m_led_pin,0);
//		}
//	}
//}

bool lastate = false;
bool lastateOff = false;

void task_switches(void *t_arg)
{
	TaskHandle_t l_handle_led_brightness = xTaskGetHandle(TASK_NAME_LED_BRIGHTNESS);
	//TaskHandle_t l_handle_led_snake_l = xTaskGetHandle( TASK_NAME_LED_SNAKE_L);
	TaskHandle_t  l_handle_switch_all = xTaskGetHandle( TASK_NAME_SWITCH_ALL);
	TaskHandle_t  l_handle_switch_all_off = xTaskGetHandle( TASK_NAME_SWITCH_ALL_OFF);
	//TaskHandle_t l_handle_led_snake_r = xTaskGetHandle( TASK_NAME_LED_SNAKE_R);

	while(1)
	{
		if(GPIO_PinRead( SW_PTC9_GPIO, SW_PTC9_PIN) == 0)
		{
			if(l_handle_switch_all)
			{
				vTaskSuspend(l_handle_switch_all);
				chgbr(1);
				if(lastate == false)clicked = true;
				vTaskResume(l_handle_switch_all);
			}
			lastate = true;
		}
		else lastate = false;

		if(GPIO_PinRead( SW_PTC12_GPIO, SW_PTC12_PIN) == 0)
		{
			if(l_handle_switch_all_off)
			{
				vTaskSuspend(l_handle_switch_all_off);
				chgbr(-1);
				if(lastateOff == false) clickedOff = true;
				vTaskResume(l_handle_switch_all_off);
			}
			lastateOff = true;
		}
		else lastateOff = false;

		if(GPIO_PinRead( SW_PTC11_GPIO, SW_PTC11_PIN) == 0)
		{
			if(l_handle_led_brightness)
			{
				vTaskSuspend(l_handle_led_brightness);
				chgmidbr(-1);
				vTaskResume(l_handle_led_brightness);
			}
		}
		if(GPIO_PinRead( SW_PTC10_GPIO, SW_PTC10_PIN) == 0)
		{
			if(l_handle_led_brightness)
			{
				vTaskSuspend(l_handle_led_brightness);
				chgmidbr(1);
				vTaskResume(l_handle_led_brightness);
			}
		}
		vTaskDelay(10);
	}
}

int main(void)
{

	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	BOARD_InitDebugConsole();

	PRINTF("FreeRTOS task demo program.\r\n");
	PRINTF("Switches PTC9 and PTC10 will stop and run PTAx LEDs blinking.\r\n");
	PRINTF("Switches PTC11 and PTC12 will start snake on red LEDS from the left and right side.\r\n");

	if (xTaskCreate(rgb_led_br,TASK_NAME_LED_BRIGHTNESS,configMINIMAL_STACK_SIZE + 100,NULL,NORMAL_TASK_PRIORITY,NULL) != pdPASS) {PRINTF("Unable to create task '%s'!\r\n", TASK_NAME_LED_PTA);}
	//if (xTaskCreate(task_snake_left, TASK_NAME_LED_SNAKE_L,configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY,NULL) != pdPASS) {PRINTF("Unable to create task '%s'!\r\n", TASK_NAME_LED_SNAKE_L);}
	if (xTaskCreate(task_doubleclick, TASK_NAME_SWITCH_ALL,configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY,NULL) != pdPASS) {PRINTF("Unable to create task '%s'!\r\n", TASK_NAME_SWITCH_ALL);}
	if (xTaskCreate(task_doubleclickOff, TASK_NAME_SWITCH_ALL_OFF,configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY,NULL) != pdPASS) {PRINTF("Unable to create task '%s'!\r\n", TASK_NAME_SWITCH_ALL);}
	//if (xTaskCreate(task_snake_right, TASK_NAME_LED_SNAKE_R,configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY,NULL) != pdPASS) {PRINTF("Unable to create task '%s'!\r\n", TASK_NAME_LED_SNAKE_R);}
	if (xTaskCreate(task_switches, TASK_NAME_SWITCHES,configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY,NULL) != pdPASS) {PRINTF("Unable to create task '%s'!\r\n", TASK_NAME_SWITCHES);}
	vTaskStartScheduler();
	while (1);
	return 0;
}