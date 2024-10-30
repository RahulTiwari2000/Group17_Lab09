#include <stdint.h>
#include <math.h>
#include "tm4c123gh6pm.h"

#define STCTRL *((volatile uint32_t *) 0xE000E010)        // STATUS AND CONTROL REGISTER
#define STRELOAD *((volatile uint32_t *) 0xE000E014)      // RELOAD VALUE REGISTER
#define STCURRENT *((volatile uint32_t *) 0xE000E018)     // CURRENT VALUE REGISTER

#define ENABLE (1 << 0)                                   // ENABLE TIMER BY SETTING BIT 0 OF CSR
#define CLKINT (1 << 2)                                   // SPECIFY CPU CLOCK
#define CLOCK_HZ 16000000                                 // CLOCK FREQUENCY
#define SYSTICK_RELOAD_VALUE(us) ((CLOCK_HZ / 1000000) * (us) - 1)   // SYSTICK RELOAD VALUE CONVERSION IN MICROSECONDS

#define MCP4725_ADDRESS 0x60                              // I2C Address for MCP4725 DAC

void I2C0_init(void) {
    SYSCTL_RCGCI2C_R |= 0x01;                             // Enable I2C0 clock
    SYSCTL_RCGCGPIO_R |= 0x01;                            // Enable GPIO Port A clock

    while ((SYSCTL_PRGPIO_R & 0x01) == 0);                // Wait for Port A clock to stabilize

    GPIO_PORTA_AFSEL_R |= 0xC0;                           // Enable alternate function on PA6 (SCL) and PA7 (SDA)
    GPIO_PORTA_ODR_R |= 0x80;                             // Enable open-drain on PA7 (SDA)
    GPIO_PORTA_DEN_R |= 0xC0;                             // Enable digital function on PA6 and PA7
    GPIO_PORTA_PCTL_R &= ~0xFF000000;                     // Clear PCTL for PA6 and PA7
    GPIO_PORTA_PCTL_R |= 0x33000000;                      // Set I2C function for PA6 and PA7

    I2C0_MCR_R = 0x10;                                    // Initialize I2C0 master function
    I2C0_MTPR_R = 0x07;                                   // Set SCL clock frequency (based on 100kHz standard mode)
}
void I2C0_SEND(uint8_t PERIPHERAL_ADDRESS, uint8_t ANALOG_SAMPLE_MSB, uint8_t ANALOG_SAMPLE_LSB) {
    I2C0_MSA_R = (PERIPHERAL_ADDRESS << 1);               // Set peripheral address, write mode
    I2C0_MDR_R = ANALOG_SAMPLE_MSB;                       // Load MSB
    I2C0_MCS_R = 0x03;                                    // Send Start + Run
    while (I2C0_MCS_R & 0x01);                            // Wait for transmission to finish
    if (I2C0_MCS_R & 0x02) return;                        // Return if error occurs

    I2C0_MDR_R = ANALOG_SAMPLE_LSB;                       // Load LSB
    I2C0_MCS_R = 0x05;                                    // Send Run + Stop
    while (I2C0_MCS_R & 0x01);                            // Wait for transmission to finish
    if (I2C0_MCS_R & 0x02) return;                        // Return if error occurs
}
void systick_setting(void) {
    STRELOAD = SYSTICK_RELOAD_VALUE(1000);                // Set reload value for 1ms delay
    STCTRL |= ENABLE | CLKINT;                            // Enable SysTick with system clock
    STCURRENT = 0;                                        // Clear current value register
}
void delay(int us) {
    STRELOAD = SYSTICK_RELOAD_VALUE(us);                  // Set reload value for required delay
    STCURRENT = 0;                                        // Clear current value register
    STCTRL |= ENABLE | CLKINT;                            // Enable SysTick with system clock
    while ((STCTRL & (1 << 16)) == 0);                    // Wait for flag to set
    STCTRL &= ~ENABLE;                                    // Stop timer
}
void AOUT(uint16_t ANALOG_SAMPLE) {
    uint8_t ANALOG_SAMPLE_MSB = (ANALOG_SAMPLE >> 8) & 0x0F;               // Take upper 4 bits of 12-bit data
    uint8_t ANALOG_SAMPLE_LSB = ANALOG_SAMPLE & 0xFF;                      // Take lower 8 bits of 12-bit data

    I2C0_SEND(MCP4725_ADDRESS, ANALOG_SAMPLE_MSB, ANALOG_SAMPLE_LSB);      // Send 12-bit data to DAC
}

int samples[100] = {                                                       // Define sine waveform samples
    2252, 2436, 2594, 2722, 2819, 2886, 2925, 2942, 2942, 2933,
    2922, 2916, 2921, 2940, 2977, 3031, 3101, 3185, 3276, 3371,
    3463, 3547, 3619, 3675, 3714, 3735, 3739, 3729, 3709, 3682,
    3652, 3622, 3594, 3570, 3547, 3525, 3499, 3466, 3420, 3356,
    3271, 3161, 3025, 2863, 2678, 2473, 2255, 2031, 1808, 1596,
    1401, 1229, 1085,  973,  891,  839,  812,  804,  809,  820,
     829,  831,  819,  791,  746,  683,  606,  518,  424,  330,
     241,  162,   98,   51,   21,    9,   12,   27,   51,   80,
     111,  140,  166,  189,  211,  235,  264,  303,  357,  431,
     528,  651,  800,  974, 1170, 1382, 1604, 1829, 2047, 2252
};
