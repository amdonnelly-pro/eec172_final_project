//*****************************************************************************
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"
#include "uart_if.h"
#include "i2c_if.h"
#include "i2s.h"

// IMPORTED FROM LAB 2
#include "hw_apps_rcm.h"
#include "gpio.h"
#include "oled_test.h"
#include "Adafruit_SSD1351.h"
#include "Adafruit_GFX.h"

#include "spi.h"
#include "driverlib/spi.h"

// Common interface includes
#include "gpio_if.h"
#include "pinmux.h"

#include "speaker.h"

#define APPLICATION_VERSION     "1.4.0"
#define APP_NAME                "Final Project"
#define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}
#define MASTER_MODE  0


//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
static void
BoardInit(void)
{
#ifndef USE_TIRTOS
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);
    PRCMCC3200MCUInit();
}


//  function delays 3*ulCount cycles
static void delay(unsigned long ulCount){
    int i;

  do{
    ulCount--;
        for (i=0; i< 65535; i++) ;
    }while(ulCount);
}


void Init_GPS_UART(void)
{
    MAP_UARTConfigSetExpClk(UARTA1_BASE, MAP_PRCMPeripheralClockGet(PRCM_UARTA1),
                            9600, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_PAR_NONE));

    MAP_UARTEnable(UARTA1_BASE);
}

//*****************************************************************************
void main()
{
    // Initial board configurations
    BoardInit();
    PinMuxConfig();

    InitTerm();
    ClearTerm();
    Message("Debugging via print statements !\n\r");

    // Establish UART
    Init_GPS_UART();

    // Establish I2S
    Init_DAC_I2S();

    while (1) {
        if (MASTER_MODE) {
            unsigned long sw3 = GPIOPinRead(GPIOA1_BASE, GPIO_PIN_5);
            unsigned long sw2 = GPIOPinRead(GPIOA2_BASE, GPIO_PIN_6);
            if (sw2 != 0) {
                Message("\nSwitch 2 pressed\n\r");
                // Update server (POST request)
            }
            if (sw3 != 0) {
                Message("\nSwitch 3 pressed\n\r");
                // Update server (POST request)
            }

            // Fetch lat,lng coords from server (GET request)
            // Redraw marker relative to center of Kemper hallway
        } else {
            // Fetch lat,lng coords from GPS module (UART communication)
            char buffer[128];
            int idx = 0;
            while (idx < 128) {
                char c = UARTCharGet(UARTA1_BASE);  // sent as ASCII chars across UART
                if (c == '\n') {
                    buffer[idx] = '\0';
                    break;
                }
                buffer[idx] = c;
                idx++;
            }
            Message(buffer);

            // Update server (POST request)
            // Fetch directions from server (GET request)
            // Play audio (I2S communication)
            PlaySound();

            // Sleep
            delay(50);
        }
    }
}
