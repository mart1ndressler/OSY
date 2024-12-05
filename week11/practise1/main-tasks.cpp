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
#include "semphr.h"
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
#define TASK_NAME_LED_SNAKE_L	"led_snake_l"
#define TASK_NAME_LED_SNAKE_R	"led_snake_r"

#define LED_PTA_NUM 	2
#define LED_PTC_NUM		8
#define LED_PTB_NUM		9

// pair of GPIO port and LED pin.
struct LED_Data
{
	uint32_t m_led_pin;
	GPIO_Type *m_led_gpio;
};

struct RGBLED_Data {
	uint32_t m_led_pin;
	GPIO_Type *m_led_gpio;
	int color_value = 0;
};

// PTB2		LED_PTB2_PIN   LED_PTB2_GPIO
// PTB3		LED_PTB3_PIN   LED_PTB3_GPIO
// PTB9		LED_PTB9_PIN   LED_PTB9_GPIO
// PTB10	LED_PTB10_PIN  LED_PTB10_GPIO
// PTB11	LED_PTB11_PIN  LED_PTB11_GPIO
// PTB18	LED_PTB18_PIN  LED_PTB18_GPIO
// PTB19	LED_PTB19_PIN  LED_PTB19_GPIO
// PTB20	LED_PTB20_PIN  LED_PTB20_GPIO
// PTB23	LED_PTB23_PIN  LED_PTB23_GPIO


RGBLED_Data rgb_ptb[LED_PTB_NUM] = {
		{ LED_PTB2_PIN, LED_PTB2_GPIO },
		{ LED_PTB3_PIN, LED_PTB3_GPIO },
		{ LED_PTB9_PIN, LED_PTB9_GPIO },

		{ LED_PTB10_PIN, LED_PTB10_GPIO },
		{ LED_PTB11_PIN, LED_PTB11_GPIO },
		{ LED_PTB18_PIN, LED_PTB18_GPIO },

		{ LED_PTB19_PIN, LED_PTB19_GPIO },
		{ LED_PTB20_PIN, LED_PTB20_GPIO },
		{ LED_PTB23_PIN, LED_PTB23_GPIO },
};


// all PTAx LEDs in array
LED_Data g_led_pta[ LED_PTA_NUM ] =
{
		{ LED_PTA1_PIN, LED_PTA1_GPIO },
		{ LED_PTA2_PIN, LED_PTA2_GPIO },
};

// all PTCx LEDs in array
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
double brightness_level = 0;

void change_led_brightness(void *t_arg){

	const int period_ms = 10;


	while(1) {
		if(brightness_level >= 90){
			brightness_level = 90;
		}
		if(brightness_level <= 0){
			brightness_level = 0;
		}
		int on_time = (brightness_level * period_ms) / 100;
		int off_time = period_ms - on_time;
		if(brightness_level != 0){
			GPIO_PinWrite( g_led_pta[ 0 ].m_led_gpio, g_led_pta[ 0 ].m_led_pin, 1 );
		}
		vTaskDelay(on_time);

		GPIO_PinWrite( g_led_pta[ 0 ].m_led_gpio, g_led_pta[ 0 ].m_led_pin, 0 );
		vTaskDelay(off_time);
	}
}
double brt2 = 90;
void change_led_brt2(void *t_arg){

	const int period_ms = 10;

		while(1) {
			if(brt2 >= 90){
				brt2 = 90;
			}
			if(brt2 <= 0){
				brt2 = 0;
			}
			int on_time = (brt2 * period_ms) / 100;
			int off_time = period_ms - on_time;
			if(brt2 != 0){
				GPIO_PinWrite( g_led_pta[ 1 ].m_led_gpio, g_led_pta[ 1 ].m_led_pin, 1 );
			}

			vTaskDelay(on_time);

			GPIO_PinWrite( g_led_pta[ 1 ].m_led_gpio, g_led_pta[ 1 ].m_led_pin, 0 );
			vTaskDelay(off_time);
		}
}

void add_brightness(void *t_arg){
	double brt = 0.0;
	int per = 10;
	while(1) {
			if(brt >= 90){
				brt = 90;
			}
			if(brt <= 0){
				brt = 0;
			}
			int on_time = (brt * per) / 100;
			int off_time = per - on_time;
			GPIO_PinWrite( g_led_pta[ 1 ].m_led_gpio, g_led_pta[ 1 ].m_led_pin, 1 );
			vTaskDelay(on_time);

			GPIO_PinWrite( g_led_pta[ 1 ].m_led_gpio, g_led_pta[ 1 ].m_led_pin, 0 );
			vTaskDelay(off_time);
			brt += 0.05;
		}
}


void task_rgb_on(void *t_arg) {
	const int brightness_levels[] = {10, 30, 50, 70, 90};
	    const int period_ms = 10;
	    int current_level = 0;

	    TickType_t last_wake_time = xTaskGetTickCount();

	    while (1) {
	        for (int rgb_inx = 0; rgb_inx < LED_PTB_NUM; rgb_inx++) {
	            int on_time = (brightness_levels[current_level] * period_ms) / 100;
	            int off_time = period_ms - on_time;

	            GPIO_PinWrite(rgb_ptb[rgb_inx].m_led_gpio, rgb_ptb[rgb_inx].m_led_pin, 1);
	            vTaskDelay(on_time / portTICK_PERIOD_MS);


	            GPIO_PinWrite(rgb_ptb[rgb_inx].m_led_gpio, rgb_ptb[rgb_inx].m_led_pin, 0);
	            vTaskDelay(off_time / portTICK_PERIOD_MS);
	        }

	        current_level = (current_level + 1) % (sizeof(brightness_levels) / sizeof(brightness_levels[0]));

	        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(50));
	    }
}

double rgb1_brt = 90.0;
void rbg1_brt(void *t_arg){

	int per = 10;
	while (1) {
				int on_time = (rgb1_brt * per) / 100;
				int off_time = per - on_time;

				if(rgb1_brt >=90){
					rgb1_brt = 90;
				}
				if(rgb1_brt <= 0){
					rgb1_brt = 0;
				}
		        for (int rgb_inx = 0; rgb_inx < 3; rgb_inx++) {

		        	if(rgb1_brt != 0){
		        		GPIO_PinWrite(rgb_ptb[rgb_inx].m_led_gpio, rgb_ptb[rgb_inx].m_led_pin, 1);
		        	}
		            vTaskDelay(on_time);


		            GPIO_PinWrite(rgb_ptb[rgb_inx].m_led_gpio, rgb_ptb[rgb_inx].m_led_pin, 0);
		            vTaskDelay(off_time);
		        }
	}
}

double rgb2_brt = 0.0;
void rbg2_brt(void *t_arg){

	int per = 10;
		while (1) {
					int on_time = (rgb2_brt * per) / 100;
					int off_time = per - on_time;

					if(rgb2_brt >=90){
						rgb2_brt = 90;
					}
					if(rgb2_brt <= 0){
						rgb2_brt = 0;
					}
			        for (int rgb_inx = 3; rgb_inx < 6; rgb_inx++) {

			        	if(rgb2_brt != 0){
			        		GPIO_PinWrite(rgb_ptb[rgb_inx].m_led_gpio, rgb_ptb[rgb_inx].m_led_pin, 1);
			        	}
			            vTaskDelay(on_time);


			            GPIO_PinWrite(rgb_ptb[rgb_inx].m_led_gpio, rgb_ptb[rgb_inx].m_led_pin, 0);
			            vTaskDelay(off_time);
			        }
		}

}

double rgb3_brt = 0.0;
void rbg3_brt(void *t_arg){


	int per = 10;
			while (1) {
						int on_time = (rgb3_brt * per) / 100;
						int off_time = per - on_time;

						if(rgb3_brt >=90){
							rgb3_brt = 90;
						}
						if(rgb3_brt <= 0){
							rgb3_brt = 0;
						}
				        for (int rgb_inx = 6; rgb_inx < 9; rgb_inx++) {

				        	if(rgb3_brt != 0){
				        		GPIO_PinWrite(rgb_ptb[rgb_inx].m_led_gpio, rgb_ptb[rgb_inx].m_led_pin, 1);
				        	}
				            vTaskDelay(on_time);


				            GPIO_PinWrite(rgb_ptb[rgb_inx].m_led_gpio, rgb_ptb[rgb_inx].m_led_pin, 0);
				            vTaskDelay(off_time);
				        }
			}
}




void switch_four_leds_on(void *t_arg){

	int cnt = 0;

	while(1){
		cnt++;
		if(cnt > 1000) {
			GPIO_PinWrite( g_led_ptc[ 2 ].m_led_gpio, g_led_ptc[ 2 ].m_led_pin, 1 );
			GPIO_PinWrite( g_led_ptc[ 3 ].m_led_gpio, g_led_ptc[ 3 ].m_led_pin, 1 );
			GPIO_PinWrite( g_led_ptc[ 4 ].m_led_gpio, g_led_ptc[ 4 ].m_led_pin, 1 );
			GPIO_PinWrite( g_led_ptc[ 5 ].m_led_gpio, g_led_ptc[ 5 ].m_led_pin, 1 );
			vTaskSuspend(0);
			cnt=0;
		}
	}

}

void switch_four_leds_off(void *t_arg){

	int cnt = 0;

	while(1){
		cnt++;
		if(cnt > 1000) {
			GPIO_PinWrite( g_led_ptc[ 2 ].m_led_gpio, g_led_ptc[ 2 ].m_led_pin, 0 );
			GPIO_PinWrite( g_led_ptc[ 3 ].m_led_gpio, g_led_ptc[ 3 ].m_led_pin, 0 );
			GPIO_PinWrite( g_led_ptc[ 4 ].m_led_gpio, g_led_ptc[ 4 ].m_led_pin, 0 );
			GPIO_PinWrite( g_led_ptc[ 5 ].m_led_gpio, g_led_ptc[ 5 ].m_led_pin, 0 );
			vTaskSuspend(0);
			cnt=0;
		}
	}

}

SemaphoreHandle_t semaforON;
SemaphoreHandle_t semaforOFF;

void switch_all_leds_on(void *t_arg){

	while(1){
		vTaskSuspend(0);
		for(int i =0;i<8;i++){
			GPIO_PinWrite( g_led_ptc[ i ].m_led_gpio, g_led_ptc[ i ].m_led_pin, 1 );
		}

	}

}

void switch_all_leds_off(void *t_arg){

	while(1){
		vTaskSuspend(0);
			for(int i =0;i<8;i++){
				GPIO_PinWrite( g_led_ptc[ i ].m_led_gpio, g_led_ptc[ i ].m_led_pin, 0);
			}

	}

}

// This task blink alternatively both PTAx LEDs
void task_led_pta_blink( void *t_arg )
{
	uint32_t l_inx = 0;



    while ( 1 )
    {
    	// switch LED on
        GPIO_PinWrite( g_led_pta[ l_inx ].m_led_gpio, g_led_pta[ l_inx ].m_led_pin, 1 );
        vTaskDelay( 500 );

        // switch LED off

        GPIO_PinWrite( g_led_pta[ l_inx ].m_led_gpio, g_led_pta[ l_inx ].m_led_pin, 0 );
        // next LED
        l_inx++;
        l_inx %= LED_PTA_NUM;
    }
}

SemaphoreHandle_t semafor;
// This task is snake animation from left side on red LEDs
void task_snake_left( void *t_arg )
{
	for(int i=0;i<8;i++){
		GPIO_PinWrite( g_led_ptc[ i ].m_led_gpio, g_led_ptc[ i ].m_led_pin, 1 );
	}
	while ( 1 )
	{
		vTaskSuspend( 0 );
		xSemaphoreTake(semafor, portMAX_DELAY);

		for ( int inx = LED_PTC_NUM - 1; inx >= 0; inx-- )
		{

	        	GPIO_PinWrite( g_led_ptc[ inx ].m_led_gpio, g_led_ptc[ inx ].m_led_pin, 0 );
	        	vTaskDelay( 200 );
		}

		for ( int inx = 0; inx < LED_PTC_NUM; inx++ )
		{

			       	GPIO_PinWrite( g_led_ptc[ inx ].m_led_gpio, g_led_ptc[ inx ].m_led_pin, 1);
			        vTaskDelay( 200 );
		}
		xSemaphoreGive(semafor);
	}
}

// This task is snake animation from right side on red LEDs
void task_snake_right( void *t_arg )
{
	while ( 1 )
	{
		vTaskSuspend( 0 );
		xSemaphoreTake(semafor, portMAX_DELAY);

		for ( int inx = 0; inx < LED_PTC_NUM; inx++ )
				{


			        	GPIO_PinWrite( g_led_ptc[ inx ].m_led_gpio, g_led_ptc[ inx ].m_led_pin, 0 );
			        	vTaskDelay( 200 );

				}

				for ( int inx = LED_PTC_NUM - 1; inx >= 0; inx--)
				{
					       	GPIO_PinWrite( g_led_ptc[ inx ].m_led_gpio, g_led_ptc[ inx ].m_led_pin, 1);
					        vTaskDelay( 200 );
				}
		xSemaphoreGive(semafor);
	}
}
TickType_t last_click_ptc9 = 0, last_click_ptc10 = 0;
int count9=0, count10 = 0;
const TickType_t double_click_window = pdMS_TO_TICKS(250);
// This task monitors switches and control others led_* tasks
void task_switches( void *t_arg )
{
	// Get task handles for task names
	TaskHandle_t l_handle_led_ptb = xTaskGetHandle("task-rgb");
	TaskHandle_t l_handle_brightness= xTaskGetHandle("task-brt");
	TaskHandle_t handle_brt2 = xTaskGetHandle("task-brt2");

	TaskHandle_t l_handle_led_pta = xTaskGetHandle( TASK_NAME_LED_PTA );
	TaskHandle_t l_handle_led_snake_l = xTaskGetHandle( TASK_NAME_LED_SNAKE_L );
	TaskHandle_t l_handle_led_snake_r = xTaskGetHandle( TASK_NAME_LED_SNAKE_R );

	TaskHandle_t handle_on = xTaskGetHandle("task-leds-on");
	TaskHandle_t handle_off = xTaskGetHandle("task-leds-off");

	TaskHandle_t handle_all_on = xTaskGetHandle("all-leds-on");
	TaskHandle_t handle_all_off = xTaskGetHandle("all-leds-off");

	TaskHandle_t handle_brt = xTaskGetHandle("brt-on");

	TaskHandle_t handle_rgb1 = xTaskGetHandle("rgb1");
	TaskHandle_t handle_rgb2 = xTaskGetHandle("rgb2");
	TaskHandle_t handle_rgb3 = xTaskGetHandle("rgb3");

	int lastStage9 =1;
	int presentStage9 = 1;

	int lastStage10 =1;
	int presentStage10 = 1;

	int presentStage11 = 1;
	int lastStage11 = 1;

	int lastStage12 = 1;
	int presentStage12 = 1;
	bool lighted = true;

	while ( 1 )
	{
		TickType_t now = xTaskGetTickCount();
		// test PTC9 switch

		presentStage9 = GPIO_PinRead( SW_PTC9_GPIO, SW_PTC9_PIN);

		if(presentStage9 == 0 && lastStage9 == 0){
			brightness_level -= 0.05;
			brt2 += 0.05;
			rgb1_brt += 0.05;
			rgb3_brt -= 0.05;
		}
		if (presentStage9 == 1 && lastStage9 == 0)
		{
			/*if ( l_handle_led_pta )
				vTaskSuspend( l_handle_led_pta );*/
			if(l_handle_led_ptb){
				vTaskSuspend(l_handle_led_ptb);
			}

			////4 ledky
			if(handle_on){
				vTaskResume(handle_on);
			}
			///8 ledky 2klik
			if (now - last_click_ptc9 < double_click_window)
			{
				count9++;
			}
			else
			{
				count9 = 1;
			}
			last_click_ptc9 = now;

			if (count9 == 2)
			{

				if( lighted == false){
					if (handle_all_on)
					{
						vTaskResume(handle_all_on);
					}
					lighted = true;
				}
				else {
					if(handle_all_off){
						vTaskResume(handle_all_off);
					}
					lighted = false;
				}

			    count9= 0;
			}
		}
		lastStage9 = presentStage9;

		// test PTC10 switch

		presentStage10 = GPIO_PinRead( SW_PTC10_GPIO, SW_PTC10_PIN );
		if(presentStage10 == 0 && lastStage10 == 0){
			brightness_level += 0.05;
			brt2 -= 0.05;
			rgb2_brt -= 0.05;
		}
		if ( presentStage10 == 1 && lastStage10 == 0 )
		{
			/*if ( l_handle_led_pta )
				vTaskResume( l_handle_led_pta );*/
			if(l_handle_led_ptb){
				vTaskResume(l_handle_led_ptb);
			}

			///4 ledky
			if(handle_off){
				vTaskResume(handle_off);
			}
			///8 ledky 2klik
			if (now - last_click_ptc10 < double_click_window)
			{
				count10++;
			}
			else{
				count10 = 1;
			}
			last_click_ptc10 = now;

			if(count10 == 2){
				if(handle_all_off){
					vTaskResume(handle_all_off);
				}
				count10 = 0;
			}
		}
		lastStage10 = presentStage10;






		// test PTC11 switch
		presentStage11 = GPIO_PinRead( SW_PTC11_GPIO, SW_PTC11_PIN );
		if ( presentStage11 == 0 && lastStage11 == 0)
		{
			rgb2_brt += 0.05;

		}
		if(presentStage11 == 1 && lastStage11 == 0){
			if ( l_handle_led_snake_l )
				vTaskResume( l_handle_led_snake_l );
		}
		lastStage11 = presentStage11;



		// test PTC12 switch
		presentStage12 = GPIO_PinRead( SW_PTC12_GPIO, SW_PTC12_PIN );

		if(presentStage12 == 0 && lastStage12 == 0){
			rgb1_brt -= 0.05;
						rgb3_brt += 0.05;
		}

		if ( presentStage12 == 0 && lastStage12 == 1 )
		{

			if ( handle_brt )
				vTaskResume( handle_brt );
		}
		if( presentStage12 == 1 && lastStage12 == 0){
			if ( handle_brt )
				vTaskSuspend(handle_brt);

			if ( l_handle_led_snake_l )
				vTaskResume( l_handle_led_snake_r );
		}
		lastStage12 = presentStage12;

		vTaskDelay( 1 );
	}
}

// Start application
int main(void) {
	semafor = xSemaphoreCreateMutex();
	semaforON = xSemaphoreCreateMutex();
	semaforOFF = xSemaphoreCreateMutex();
    // Init board hardware.
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();

    PRINTF( "FreeRTOS task demo program.\r\n" );
    PRINTF( "Switches PTC9 and PTC10 will stop and run PTAx LEDs blinking.\r\n" );
    PRINTF( "Switches PTC11 and PTC12 will staxSemaphoreGive(semafor);rt snake on red LEDS from the left and right side.\r\n");

    // Create tasks
 /*   if ( xTaskCreate(
    		task_led_pta_blink,
    		TASK_NAME_LED_PTA,
			configMINIMAL_STACK_SIZE + 100,
			NULL,
			NORMAL_TASK_PRIORITY,
			NULL ) != pdPASS )
    {
        PRINTF( "Unable to create task '%s'!\r\n", TASK_NAME_LED_PTA );
    }
*/
    //if ( xTaskCreate( task_rgb_on, "task-rgb", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL ) != pdPASS )
     //   {
     //       PRINTF( "Unable to create task-rgb \r\n");
     //  }

    //if ( xTaskCreate( task_snake_left, TASK_NAME_LED_SNAKE_L, configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL ) != pdPASS );


    //if ( xTaskCreate( task_snake_right, TASK_NAME_LED_SNAKE_R, configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL ) != pdPASS);


    if ( xTaskCreate( task_switches, TASK_NAME_SWITCHES, configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );

    //if( xTaskCreate( change_led_brightness, "task-brt", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );
    //if( xTaskCreate (change_led_brt2, "task-brt2", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );

    if( xTaskCreate( switch_four_leds_on, "task-leds-on", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );

    if( xTaskCreate( switch_four_leds_off, "task-leds-off", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );

    //if( xTaskCreate( switch_all_leds_on, "all-leds-on", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );

    //if( xTaskCreate( switch_all_leds_off, "all-leds-off", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );

    //if( xTaskCreate( rbg1_brt, "rgb1", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );
    //if( xTaskCreate( rbg2_brt, "rgb2", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );
    //if( xTaskCreate( rbg3_brt, "rgb3", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );

    //if( xTaskCreate( add_brightness, "brt-on", configMINIMAL_STACK_SIZE + 100, NULL, NORMAL_TASK_PRIORITY, NULL) != pdPASS );




    vTaskStartScheduler();

    while ( 1 );

    return 0 ;
}

