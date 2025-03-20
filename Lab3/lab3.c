#include <stdint.h>
#include <stdbool.h>

// Memory-mapped I/O addresses
#define ADC_BASE 0xFF204000        // Base address for ADC
#define GPIO_BASE 0xFF200060       // GPIO port for green LEDs
#define SW_BASE 0xFF200040         // Slide switch base

#define ADC_CHANNEL_0 (volatile uint32_t *)(ADC_BASE + 0x00) // Channel 0
#define ADC_CHANNEL_1 (volatile uint32_t *)(ADC_BASE + 0x04) // Channel 1
#define GPIO_DATA (volatile uint32_t *)(GPIO_BASE)
#define GPIO_DIR (volatile uint32_t *)(GPIO_BASE + 0x04)
#define SW (volatile uint32_t *)(SW_BASE)

void init_gpio() {
    // Set GPIO pins 0-9 as output for LEDs
    *GPIO_DIR = 0x3FF; // 10-bit LED output (bits 0-9)
}

uint16_t read_adc(uint32_t *channel) {
    uint32_t adc_value;

    // Start ADC conversion
    *channel = 1;

    // Wait for conversion to complete (Bit 15 must be 1)
    do {
        adc_value = *channel;
    } while (!(adc_value & 0x8000)); // Bit 15 check

    return (adc_value & 0x0FFF); // Extract lower 12 bits (ADC result)
}

void display_on_leds(uint16_t adc_value) {
    uint16_t led_pattern = 0;

    // Scale the 12-bit ADC value (0-4095) to 10-bit LED range (0-1023)
    uint16_t led_level = adc_value * 10 / 4096; 

    // Turn on LEDs based on the level
    for (uint16_t i = 0; i < led_level; i++) {
        led_pattern |= (1 << i);
    }

    // Output to GPIO LEDs
    *GPIO_DATA = led_pattern;
}

int main() {
    uint16_t adc_value;
    init_gpio();

    while (1) {
        // Read switch position (SW0)
        uint32_t switch_value = *SW & 0x1; // Read bit 0

        if (switch_value == 0) {
            adc_value = read_adc(ADC_CHANNEL_0); // Read from potentiometer 1
        } else {
            adc_value = read_adc(ADC_CHANNEL_1); // Read from potentiometer 2
        }

        // Display the ADC value on the LEDs
        display_on_leds(adc_value);
    }

    return 0;
}
