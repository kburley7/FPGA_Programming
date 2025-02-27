#define SW_BASE 0xFF200040
#define HEX3_HEX0_BASE 0xFF200020
#define LED_BASE	   0xFF200000

volatile int *switch_ptr = (int *)SW_BASE;
volatile char *hex0_ptr = (char *)HEX3_HEX0_BASE;

// Lookup table for 7-segment display encoding
const char HEX_CODES[16] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

// Function to read the 4-bit value from slide switches
int read_switches() {
    return (*switch_ptr) & 0xF;  // Mask to get only the lower 4 bits
}

// Function to display a digit on HEX0
void display_digit(int digit) {
    *hex0_ptr = HEX_CODES[digit];  // Update HEX0 with the corresponding 7-segment pattern
}

// Delay function
void delay() {
    volatile int i;
    for (i = 0; i < 700000; i++);  // Adjust this value if needed
}

int main() {
    while (1) {
        int value = read_switches();  // Read the switch values
        display_digit(value);         // Display the corresponding hex digit
        delay();
        *hex0_ptr = 0x00;             // Turn off HEX0
        delay();
    }
    return 0;
}
