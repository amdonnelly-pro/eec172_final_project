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

char* parseSegment(char* start_pntr, char* buffer) {
    char* end_pntr = strstr(start_pntr, ",");
    int length = (int)(end_pntr - start_pntr);
    strncpy(buffer, start_pntr, length);
    buffer[length] = '\0';

    return end_pntr;
}

//*****************************************************************************
void main()
{
    BoardInit();
    PinMuxConfig();

    InitTerm();
    ClearTerm();

    Init_GPS_UART();

    Report("Debugging via print statements !\n\r");

    while (1)
    {
        MAP_UtilsDelay(1000000);

        // Fetch lat,lng coords from GPS module (UART communication)
        char buffer[128];
        int idx = 0;
        while (idx < 128) {
            if (MAP_UARTCharsAvail(UARTA1_BASE))
            {
                char c = MAP_UARTCharGet(UARTA1_BASE);
                if (c == '\n') {
                    buffer[idx] = '\0';
                    break;
                }
                buffer[idx] = c;
                idx++;
            }
            else
            {
                // Report("No data\r\n");
            }
        }
        Message("\nFull Message: \r\n");
        Message(buffer);

        char x[100];
        char dir1[100];
        char y[100];
        char dir2[100];


        // Find the first occurrence of sub in str
        char* result = strstr(buffer, "$GPRMC");

        if (result != NULL) {
            result = strstr(result, ",");
            int i;
            for (i = 0; i<2; i++) {
                result = strstr(result + 1, ",");
            }
            char* start_pntr = result + 1;
            char* end_pntr = parseSegment(start_pntr, x);
            end_pntr = parseSegment(end_pntr + 1, dir1);
            end_pntr = parseSegment(end_pntr + 1, y);
            end_pntr = parseSegment(end_pntr + 1, dir2);

            printf("Substring found!\n");
            printf("Full text from match: %s %s %s %s\n", x, dir1, y, dir2);
        } else {
            printf("Substring not found.\n");
        }
    }
}
