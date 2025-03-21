#include <stdint.h>   // Includes standard integer types (uint32_t, uint16_t)
#include <stdbool.h>  // Includes standard boolean definitions

// Define memory-mapped I/O base addresses for peripherals
#define ADC_BASE 0xFF204000        // Base memory address for Analog-to-Digital Converter (ADC)
#define GPIO_BASE 0xFF200060       // Base memory address for General-Purpose Input/Output (GPIO), specifically LEDs
#define SW_BASE 0xFF200040         // Base memory address for slide switches

// Define pointers to specific ADC channel registers (for reading potentiometer values)
#define ADC_CHANNEL_0 (volatile uint32_t *)(ADC_BASE + 0x00) // Address pointer for ADC Channel 0
#define ADC_CHANNEL_1 (volatile uint32_t *)(ADC_BASE + 0x04) // Address pointer for ADC Channel 1

// Define pointers to GPIO data and direction registers (for controlling LEDs)
#define GPIO_DATA (volatile uint32_t *)(GPIO_BASE)           // Data register controls LED state (on/off)
#define GPIO_DIR (volatile uint32_t *)(GPIO_BASE + 0x04)     // Direction register sets GPIO pin direction (input/output)

// Define pointer to slide switch input register
#define SW (volatile uint32_t *)(SW_BASE)                    // Input register reads slide switches position

// Initializes GPIO pins for controlling LEDs
void init_gpio() {
    // Set GPIO pins 0-9 as outputs (each bit represents an LED)
    *GPIO_DIR = 0x3FF; // 0x3FF = binary 1111111111, making the first 10 pins outputs
}

// Reads and returns the ADC value from the specified ADC channel
// FIXED: Parameter type changed to include 'volatile' to resolve warning
uint16_t read_adc(volatile uint32_t *channel) {
    uint32_t adc_value;

    // Initiates an ADC conversion by writing '1' to the ADC channel's register
    *channel = 1;

    // Wait loop: repeatedly read ADC channel register until conversion is complete
    // Conversion complete indicated by bit 15 (most significant bit) being set to 1
    do {
        adc_value = *channel; // Continuously read the ADC channel register
    } while (!(adc_value & 0x8000)); // Check if bit 15 is set (conversion done)

    // Once conversion is complete, extract the actual ADC value (lower 12 bits)
    return (adc_value & 0x0FFF); // Mask upper bits, keeping lower 12 bits only (ADC result ranges from 0 to 4095)
}

// Displays the ADC reading visually using LEDs
void display_on_leds(uint16_t adc_value) {
    uint16_t led_pattern = 0; // Stores which LEDs to turn on (initially all off)

    // Converts 12-bit ADC value (range 0-4095) to a 10-level LED scale (0-10 LEDs)
    uint16_t led_level = (adc_value * 10+2047) / 4096; // Scaling calculation

    // FIXED: Ensure for-loop compatibility with C89 by declaring loop variable outside
    uint16_t i;
    for (i = 0; i < led_level; i++) {
        led_pattern |= (1 << i); // Set corresponding bit in led_pattern (turns on LED 'i')
    }

    // Output the pattern to the GPIO LEDs to visually display ADC level
    *GPIO_DATA = led_pattern;
}

// Main function (entry point of program execution)
int main() {
    uint16_t adc_value;  // Variable stores the ADC reading

    init_gpio(); // Initializes GPIO pins at the program start

    // Main program loop (runs indefinitely)
    while (1) {
        // Reads the current state of slide switch 0 (SW0)
        uint32_t switch_value = *SW & 0x1; // Extracts only the least significant bit (switch 0 position)

        if (switch_value == 0) { // If switch 0 is OFF
            adc_value = read_adc(ADC_CHANNEL_0); // Reads from potentiometer 1 via ADC channel 0
        } else { // If switch 0 is ON
            adc_value = read_adc(ADC_CHANNEL_1); // Reads from potentiometer 2 via ADC channel 1
        }

        // Shows the ADC reading on LEDs for visual feedback
        display_on_leds(adc_value);
    }

    return 0; // Return statement (program should never reach here because of infinite loop)
}
