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
