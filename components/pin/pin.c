#include <stdio.h>
#include <stdint.h>
#include "soc/reg_base.h" // DR_REG_GPIO_BASE, DR_REG_IO_MUX_BASE
#include "driver/rtc_io.h" // rtc_gpio_*
#include "pin.h"

// GPIO Matrix Registers 
#define GPIO_OUT_REG          		(DR_REG_GPIO_BASE+0x04)
#define GPIO_OUT_W1TS_REG     		(DR_REG_GPIO_BASE+0x08)
#define GPIO_OUT_W1TC_REG     		(DR_REG_GPIO_BASE+0x0C)
#define GPIO_OUT1_REG         		(DR_REG_GPIO_BASE+0x10)
#define GPIO_OUT1_W1TS_REG   		(DR_REG_GPIO_BASE+0x14)
#define GPIO_OUT1_W1TC_REG    		(DR_REG_GPIO_BASE+0x18)
#define GPIO_ENABLE_REG       		(DR_REG_GPIO_BASE+0x20)
#define GPIO_ENABLE_W1TS_REG  		(DR_REG_GPIO_BASE+0x24)
#define GPIO_ENABLE_W1TC_REG  		(DR_REG_GPIO_BASE+0x28)
#define GPIO_ENABLE1_REG      		(DR_REG_GPIO_BASE+0x2C)
#define GPIO_ENABLE1_W1TS_REG 		(DR_REG_GPIO_BASE+0x30)
#define GPIO_ENABLE1_W1TC_REG 		(DR_REG_GPIO_BASE+0x34)
#define GPIO_STRAP_REG        		(DR_REG_GPIO_BASE+0x38)
#define GPIO_IN_REG           		(DR_REG_GPIO_BASE+0x3C)
#define GPIO_IN1_REG          		(DR_REG_GPIO_BASE+0x40)
#define GPIO_STATUS_REG       		(DR_REG_GPIO_BASE+0x44)
#define GPIO_STATUS_W1TS_REG  		(DR_REG_GPIO_BASE+0x48)
#define GPIO_STATUS_W1TC_REG  		(DR_REG_GPIO_BASE+0x4C)
#define GPIO_STATUS1_REG      		(DR_REG_GPIO_BASE+0x50)
#define GPIO_STATUS1_W1TS_REG 		(DR_REG_GPIO_BASE+0x54)
#define GPIO_STATUS1_W1TC_REG 		(DR_REG_GPIO_BASE+0x58)
#define GPIO_ACPU_INT_REG     		(DR_REG_GPIO_BASE+0x60)
#define GPIO_ACPU_NMI_INT_REG 		(DR_REG_GPIO_BASE+0x64)
#define GPIO_PCPU_INT_REG     		(DR_REG_GPIO_BASE+0x68)
#define GPIO_PCPU_NMI_INT_REG 		(DR_REG_GPIO_BASE+0x6C)
#define GPIO_ACPU_INT1_REG    		(DR_REG_GPIO_BASE+0x74)
#define GPIO_ACPU_NMI_INT1_REG 		(DR_REG_GPIO_BASE+0x78)
#define GPIO_PCPU_INT1_REG    		(DR_REG_GPIO_BASE+0x7C)
#define GPIO_PCPU_NMI_INT1_REG 		(DR_REG_GPIO_BASE+0x80)
#define GPIO_PIN0_REG         		(DR_REG_GPIO_BASE+0x88)
#define GPIO_FUNC0_IN_SEL_CFG_REG  	(DR_REG_GPIO_BASE+0x130)
#define GPIO_FUNC0_OUT_SEL_CFG_REG  (DR_REG_GPIO_BASE+0x530)

// IO MUX Registers
#define IO_MUX_REG(n) (DR_REG_IO_MUX_BASE + (PIN_MUX_REG_OFFSET[n]))

// IO MUX Register Fields
#define FUN_WPD  7
#define FUN_WPU  8
#define FUN_IE   9
#define FUN_DRV_BIT1  10
#define FUN_DRV_BIT2  11
#define MCU_SEL_BIT1  12
#define MCU_SEL_BIT2  13
#define MCU_SEL_BIT3  14

// Bit manipulation and observation macros
#define REG(r) (*(volatile uint32_t *)(r))
#define REG_BITS 32
#define REG_BYTE 4
#define REG_SET_BIT(r,b) (REG(r) |= (1<<b))
#define REG_CLR_BIT(r,b) (REG(r) &= ~(1<<b))
#define REG_GET_BIT(r,b) ((REG(r) & (1 << b)) >> b)
#define PIN_MAX 39
#define RESET_GPIO_FUNC 0x0100

// Gives byte offset of IO_MUX Configuration Register
// from base address DR_REG_IO_MUX_BASE
static const uint8_t PIN_MUX_REG_OFFSET[] = {
    0x44, 0x88, 0x40, 0x84, 0x48, 0x6c, 0x60, 0x64, // pin  0- 7
    0x68, 0x54, 0x58, 0x5c, 0x34, 0x38, 0x30, 0x3c, // pin  8-15
    0x4c, 0x50, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x8c, // pin 16-23
    0x90, 0x24, 0x28, 0x2c, 0xFF, 0xFF, 0xFF, 0xFF, // pin 24-31
    0x1c, 0x20, 0x14, 0x18, 0x04, 0x08, 0x0c, 0x10, // pin 32-39
};

// Reset the configuration of a pin to not be an input or an output.
// Pull-up is enabled so the pin does not float.
// Return zero if successful, or non-zero otherwise.
int32_t pin_reset(pin_num_t pin)
{
	if (rtc_gpio_is_valid_gpio(pin)) 
	{ // hand-off work to RTC subsystem
		rtc_gpio_deinit(pin);
		rtc_gpio_pullup_en(pin);
		rtc_gpio_pulldown_dis(pin);
	}
	// Reset GPIO_PINn_REG: All fields zero
    if (pin >= 0 && pin <= (PIN_MAX-2)) 
	{ // Assuming valid pin range based on TRM
        REG(GPIO_PIN0_REG + (pin * REG_BYTE)) = 0; // Offset increases by 4 bytes per pin
    }
	// Reset GPIO_FUNCn_OUT_SEL_CFG_REG: GPIO_FUNCn_OUT_SEL=0x100
	if (pin >= 0)
	{
		REG(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin*REG_BYTE)) = RESET_GPIO_FUNC;
	}
	// Reset IO_MUX_x_REG: 
	REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD); // FUN_WPD = 0
	REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU); // FUN_WPU = 1
	REG_CLR_BIT(IO_MUX_REG(pin), FUN_DRV_BIT1); // FUN_DRV = 2 (set the first bit to 0)
	REG_SET_BIT(IO_MUX_REG(pin), FUN_DRV_BIT2); // FUN_DRV = 2 (set the second bit to 1)
	REG_CLR_BIT(IO_MUX_REG(pin), MCU_SEL_BIT1); // MCU_SEL = 2 (set the first bit to 0)
	REG_SET_BIT(IO_MUX_REG(pin), MCU_SEL_BIT2); // MCU_SEL = 2 (set the second bit to 1)
	// Now that the pin is reset, set the output level to zero
	return pin_set_level(pin, 0);
}

// Enable or disable a pull-up on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pullup(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pullup_en(pin);
		else return rtc_gpio_pullup_dis(pin);
	}
	// Set the FUN_WPU bit in an IO_MUX register
	if (enable)
	{
		REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU);
	}
	// Clear the FUN_WPU bit in an IO_MUX register
	else
	{
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPU);
	}
	return 0;
}

// Enable or disable a pull-down on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pulldown(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pulldown_en(pin);
		else return rtc_gpio_pulldown_dis(pin);
	}
	// Set the FUN_WPD bit in an IO_MUX register
	if (enable)
	{
		REG_SET_BIT(IO_MUX_REG(pin), FUN_WPD);
	}
	// Clear the FUN_WPD bit in an IO_MUX register
	else
	{
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD);
	}
	return 0;
}

// Enable or disable the pin as an input signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_input(pin_num_t pin, bool enable)
{
	// Set the FUN_IE bit in an IO_MUX register
	if (enable)
	{
		REG_SET_BIT(IO_MUX_REG(pin), FUN_IE);
	}
	// Clear the FUN_IE bit in an IO_MUX register
	else
	{
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_IE);
	}
	return 0;
}

// Enable or disable the pin as an output signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_output(pin_num_t pin, bool enable)
{
	// invalid input, return non-zero
	if (pin < 0 || pin > PIN_MAX)
	{
		return 1; 
	}
	// Set or clear the I/O pin bit in the ENABLE or ENABLE1 register
	// use Enable reg for pins 0-31
	if (pin < REG_BITS) 
	{
		// Enable output
		if (enable) 
		{
			REG_SET_BIT(GPIO_ENABLE_REG, pin); 
		}
		// Disable output
		else
		{
			REG_CLR_BIT(GPIO_ENABLE_REG, pin); 
		}
	}
	// use Enable1 reg for pins 32-39
	// NOTE: subtract 32 from the pin number to find the right bit position
	else
	{
		// Enable output
		if (enable)
		{
			REG_SET_BIT(GPIO_ENABLE1_REG, (pin-REG_BITS));
		}
		// Disable output
		else
		{
			REG_CLR_BIT(GPIO_ENABLE1_REG, (pin-REG_BITS));
		}
	}
	return 0;
}

// Enable or disable the pin as an open-drain signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_odrain(pin_num_t pin, bool enable)
{
	// Return non-zero for invalid input if pin is out of range
	if (pin < 0 || pin > (PIN_MAX-2))
	{
		return 1;
	}
	// Set the PAD_DRIVER bit in a PIN register
	if (enable)
	{
		REG_SET_BIT(GPIO_PIN0_REG + (pin*REG_BYTE), 2); // the second bit is the PAD_DRIIVER bit
		return 0;
	}
	// Clear the PAD_DRIVER bit in a PIN register
	else
	{
		REG_CLR_BIT(GPIO_PIN0_REG + (pin*REG_BYTE), 2); // the second bit is the PAD_DRIIVER bit
		return 0;
	}
	return 1; // unexpected error
}

// Sets the output signal level if the pin is configured as an output.
// Return zero if successful, or non-zero otherwise.
int32_t pin_set_level(pin_num_t pin, int32_t level)
{
	// invalid input, return non-zero
	if (pin < 0 || pin > PIN_MAX)
	{
		return 1; 
	}
	// Set or clear the I/O pin bit in the OUT or OUT1 register
	// use out reg for pins 0-31
	if (pin < REG_BITS) 
	{
		// Set bit to 1 for high level
		if (level != 0) 
		{
			REG_SET_BIT(GPIO_OUT_REG, pin); 
		}
		// Set bit to 0 for low level
		else
		{
			REG_CLR_BIT(GPIO_OUT_REG, pin); 
		}
	}
	// use out1 reg for pins 32-39
	// NOTE: subtract 32 from the pin number to find the right bit position
	else
	{
		// Set bit to 1 for high level
		if (level != 0)
		{
			REG_SET_BIT(GPIO_OUT1_REG, (pin-REG_BITS));
		}
		// Set bit to 0 for low level
		else
		{
			REG_CLR_BIT(GPIO_OUT1_REG, (pin-REG_BITS));
		}
	}
	return 0;
}

// Gets the input signal level if the pin is configured as an input.
// Return zero or one if successful, or negative otherwise.
int32_t pin_get_level(pin_num_t pin)
{
	// check validity of pin number
	if (pin < 0 || pin > PIN_MAX)
	{
		return -1; // return negative for lack of success
	}
	// Get the I/O pin bit from the IN register
	if (pin < REG_BITS)
	{
		return REG_GET_BIT(GPIO_IN_REG, pin);
	}
	// Get the I/O pin bit from the IN1 register
	else
	{
		return REG_GET_BIT(GPIO_IN1_REG, (pin-REG_BITS));
	}
}

// Get the value of the input registers, one pin per bit.
// The two 32-bit input registers are concatenated into a uint64_t.
uint64_t pin_get_in_reg(void)
{
	// Read the IN and IN1 registers, return the concatenated values
	uint32_t in_reg = REG(GPIO_IN_REG);
	uint32_t in1_reg = REG(GPIO_IN1_REG);
	uint64_t result = ((uint64_t)in1_reg << REG_BITS) | in_reg;
	return result;
}

// Get the value of the output registers, one pin per bit.
// The two 32-bit output registers are concatenated into a uint64_t.
uint64_t pin_get_out_reg(void)
{
	// Read the OUT and OUT1 registers, return the concatenated values
	uint32_t out_reg = REG(GPIO_OUT_REG);
	uint32_t out1_reg = REG(GPIO_OUT1_REG);
	uint64_t result = ((uint64_t)out1_reg << REG_BITS) | out_reg;
	return result;
}
