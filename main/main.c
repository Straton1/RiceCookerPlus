#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "hw.h"
#include "pin.h"
#include "in.h"
#include "out.h"
#include "config.h"

static const char *TAG = "DEBUG";

// The update period as an integer in ms
#define PER_MS ((uint32_t)(CONFIG_GAME_TIMER_PERIOD*1000))
#define TIME_OUT 500 // ms

#define CHK_RET(x) ({                                           \
        int32_t ret_val = (x);                                  \
        if (ret_val != 0) {                                     \
            ESP_LOGE(TAG, "FAIL: return %ld, %s", ret_val, #x); \
        }                                                       \
        ret_val;                                                \
    })

TimerHandle_t update_timer; // Declare timer handle for update callback

volatile bool interrupt_flag;

uint32_t isr_triggered_count;
uint32_t isr_handled_count;

// Interrupt handler - use flag method
void update() {
	interrupt_flag = true;
	isr_triggered_count++;
}

// Main application
void app_main(void)
{
	// ISR flag and counts
	interrupt_flag = false;
	isr_triggered_count = 0;
	isr_handled_count = 0;

	// Initialization
	in_init();
    out_init();

	// Configure I/O pins for buttons
	pin_reset(HW_START_DEMO);
	pin_input(HW_START_DEMO, true);
	pin_reset(HW_DISPENSE_SW_A);
	pin_input(HW_DISPENSE_SW_A, true);
	pin_reset(HW_DISPENSE_SW_B);
	pin_input(HW_DISPENSE_SW_B, true);
    pin_reset(HW_QUIT);
	pin_input(HW_QUIT, true);

	pin_reset(HW_DISPENSE_M_A);
	pin_output(HW_DISPENSE_M_A, true);
	pin_reset(HW_DISPENSE_M_B);
	pin_output(HW_DISPENSE_M_B, true);
	pin_reset(HW_WATER_PUMP);
	pin_output(HW_WATER_PUMP, true);
    pin_reset(HW_ON_OFF);
	pin_output(HW_ON_OFF, true);
	pin_reset(HW_PWM_SRVO);
	pin_output(HW_PWM_SRVO, true);

	// Initialize update timer
	update_timer = xTimerCreate(
		"update_timer",        // Text name for the timer.
		pdMS_TO_TICKS(PER_MS), // The timer period in ticks.
		pdTRUE,                // Auto-reload the timer when it expires.
		NULL,                  // No need for a timer ID.
		update                 // Function called when timer expires.
	);
	if (update_timer == NULL) {
		ESP_LOGE(TAG, "Error creating update timer");
		return;
	}
	if (xTimerStart(update_timer, pdMS_TO_TICKS(TIME_OUT)) != pdPASS) {
		ESP_LOGE(TAG, "Error starting update timer");
		return;
	}

	// Main game loop
	uint64_t t1, t2, tmax = 0; // For hardware timer values
    uint8_t START_FINISH = true;
	while (pin_get_level(HW_QUIT)) // while MENU button not pressed
	{
		while (!interrupt_flag) ;
		t1 = esp_timer_get_time();
		interrupt_flag = false;
		isr_handled_count++;

		in_tick(START_FINISH);
		out_tick(START_FINISH);
        
		t2 = esp_timer_get_time() - t1;
		if (t2 > tmax) tmax = t2;
	}

    // Quit handling
	printf("Handled %lu of %lu interrupts\n", isr_handled_count, isr_triggered_count);
	printf("WCET us:%llu\n", tmax);
	in_deinit();
    out_deinit();
}