#include <stdint.h>
#include <stdbool.h>

// Memory Mapped I/O addresses
#define TIMER_BASE 0xFFFEC600
#define TIMER_LOAD (volatile uint32_t *)(TIMER_BASE)
#define TIMER_COUNT (volatile uint32_t *)(TIMER_BASE + 0x04)
#define TIMER_CTRL (volatile uint32_t *)(TIMER_BASE + 0x08)
#define TIMER_STATUS (volatile uint32_t *)(TIMER_BASE + 0x0C)

#define KEY_BASE 0xFF200050
#define SW_BASE 0xFF200040
#define HEX3_HEX0_BASE 0xFF200020
#define HEX5_HEX4_BASE 0xFF200030

// Seven-segment display encoding for digits 0-9
uint8_t seven_seg_digits[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

// Stopwatch state variables
uint32_t minutes = 0, seconds = 0, hundredths = 0;
uint32_t lap_minutes = 0, lap_seconds = 0, lap_hundredths = 0;
bool is_running = false;
bool lap_mode = false;
uint32_t previous_keys = 0xF; // All keys unpressed (active low)

// Timer Initialization
void init_timer() {
    *TIMER_LOAD = 2000000; // 0.01s at 200 MHz
    *TIMER_CTRL = 0x3;     // Enable Timer, Auto-reload, Start Timer
}

void reset_timer() {
    *TIMER_CTRL = 0x0;
    *TIMER_STATUS = 0x1;
}

void start_timer() {
    *TIMER_STATUS = 0x1; // Clear timeout flag
    *TIMER_CTRL = 0x3;   // Enable Timer, Auto-reload, Start Timer
    is_running = true;
}

void stop_timer() {
    *TIMER_CTRL = 0x0;
    is_running = false;
}

void clear_stopwatch() {
    minutes = 0;
    seconds = 0;
    hundredths = 0;
    lap_minutes = 0;
    lap_seconds = 0;
    lap_hundredths = 0;
}

void update_time() {
    hundredths++;
    if (hundredths >= 100) {
        hundredths = 0;
        seconds++;
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
            if (minutes >= 60) {
                minutes = 0;
            }
        }
    }
}

void store_lap_time() {
    lap_minutes = minutes;
    lap_seconds = seconds;
    lap_hundredths = hundredths;
}

void display_digit(uint32_t digit, uint32_t position) {
    volatile uint32_t *hex;
    if (position < 4) {
        hex = (volatile uint32_t *)HEX3_HEX0_BASE;
        *hex &= ~(0xFF << (position * 8));
        *hex |= (seven_seg_digits[digit] << (position * 8));
    } else {
        hex = (volatile uint32_t *)HEX5_HEX4_BASE;
        *hex &= ~(0xFF << ((position - 4) * 8));
        *hex |= (seven_seg_digits[digit] << ((position - 4) * 8));
    }
}

void display_time(uint32_t min, uint32_t sec, uint32_t hnd) {
    display_digit(hnd % 10, 0);       // Hundredths - Units
    display_digit(hnd / 10, 1);       // Hundredths - Tens
    display_digit(sec % 10, 2);       // Seconds - Units
    display_digit(sec / 10, 3);       // Seconds - Tens
    display_digit(min % 10, 4);       // Minutes - Units
    display_digit(min / 10, 5);       // Minutes - Tens
}

void update_display() {
    if (lap_mode) {
        display_time(lap_minutes, lap_seconds, lap_hundredths);
    } else {
        display_time(minutes, seconds, hundredths);
    }
}

// Improved Button Handling
void check_buttons() {
    uint32_t keys = *(volatile uint32_t *)KEY_BASE;
    uint32_t sw = *(volatile uint32_t *)SW_BASE;

    // Detect button state change (active low)
    if ((previous_keys & 0x1) && !(keys & 0x1)) { // KEY0 - Start
        start_timer();
    }
    if ((previous_keys & 0x2) && !(keys & 0x2)) { // KEY1 - Stop
        stop_timer();
    }
    if ((previous_keys & 0x4) && !(keys & 0x4)) { // KEY2 - Lap
        store_lap_time();
    }
    if ((previous_keys & 0x8) && !(keys & 0x8)) { // KEY3 - Clear
        clear_stopwatch();
        update_display();  // Force update display on Clear
    }
    // SW0 - Toggle Display
    lap_mode = (sw & 0x1) ? true : false;

    previous_keys = keys;
}

int main() {
    init_timer();
    clear_stopwatch();

    while (1) {
        // Check if timer interrupt occurred
        if (*TIMER_STATUS & 0x1) {
            *TIMER_STATUS = 0x1; // Clear Timeout
            if (is_running) {
                update_time();
            }
            update_display();
        }
        check_buttons();
    }
}
