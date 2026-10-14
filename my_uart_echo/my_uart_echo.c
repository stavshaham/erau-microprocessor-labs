//*****************************************************************************
//
// uart_echo.c - Example for reading data from and writing data to the UART in
//               an interrupt driven fashion.
//
// Copyright (c) 2011-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the DK-TM4C123G Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "grlib/grlib.h"
#include "drivers/cfal96x64x16.h"

#define DELAY_TIME 3000000
#define ON 200000
#define OFF 3800000
#define SIZE 200

int flood = 0;
int led = 0;
int quit = 0;
int lcounter = 0;
int gNumCharRcv = 0;
int counter = 0;

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>UART Echo (uart_echo)</h1>
//!
//! This example application utilizes the UART to echo text.  The first UART
//! (connected to the USB debug virtual serial port on the evaluation board)
//! will be configured in 115,200 baud, 8-n-1 mode.  All characters received on
//! the UART are transmitted back to the UART.
//
//*****************************************************************************

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// Send a string to the UART.
//
//*****************************************************************************
void
UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    //
    // Loop while there are more characters to send.
    //
    while(ui32Count--)
    {
        //
        // Write the next character to the UART.
        //
        UARTCharPut(UART0_BASE, *pui8Buffer++);
    }
}

void process_menu(int32_t c) {
 
   
  // UARTCharPutNonBlocking(UART0_BASE, c);

  if (c == 'f') {
      if (flood) 
        flood = 0;
      else 
        flood = 1;
  }
  
  if (c == 'm') {
    UARTSend((uint8_t *) "\r\n", 2);
    UARTSend((uint8_t *)"Menu Selection:\r\n", 17);
    UARTSend((uint8_t *)"M - Print this menu\r\n", 21);
    UARTSend((uint8_t *)"Q - Quit the program\r\n", 22);
    UARTSend((uint8_t *)"L - Toggle loop count display\r\n", 31);
    UARTSend((uint8_t *)"B - Toggle button display\r\n", 27);
    UARTSend((uint8_t *)"U - Toggle UART RECV/XMIT display\r\n", 35);
    UARTSend((uint8_t *)"F - Toggle flood character enable\r\n", 35);
    UARTSend((uint8_t *)"D - Toggle flashing LED\r\n", 25);
    UARTSend((uint8_t *)"C - Clear terminal window\r\n", 27);
    UARTSend((uint8_t *)"\n", 1);
  }
  
  if (c == 'd') {
    if (led) {
      led = 0;
      UARTSend((uint8_t *) "\r\rLED OFF\r\n", 11);
    } else { 
      led = 1;
      UARTSend((uint8_t *) "\r\rLED ON\r\n", 10);
    }
  }
  
  if (c == 'q') {
    quit = 1;
  }
  
  if (c == 'c') {
    UARTSend((uint8_t *) "\033[2J\033[H", 7);
  }
  
  if (c == 'l') {
    if (lcounter)
      lcounter = 0;
    else
      lcounter = 1;
  }

}

//*****************************************************************************
//
// This example demonstrates how to send a string of data to the UART.
//
//*****************************************************************************
int
main(void)
{
    tRectangle sRect;
    tContext sContext;
    volatile uint32_t ui32Loop;

    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    FPULazyStackingEnable();

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    //
    // Initialize the display driver.
    //
    CFAL96x64x16Init();

    //
    // Initialize the graphics context.
    //
    GrContextInit(&sContext, &g_sCFAL96x64x16);

    //
    // Fill the top part of the screen with blue to create the banner.
    //
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = 9;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    //
    // Change foreground for white text.
    //
    GrContextForegroundSet(&sContext, ClrWhite);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&sContext, g_psFontFixed6x8);
    GrStringDrawCentered(&sContext, "Welcome!", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 4, 0);

    //
    // Initialize the display and write some instructions.
    //
    GrStringDrawCentered(&sContext, "See", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 20, false);
    GrStringDrawCentered(&sContext, "computer", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 30, false);
    GrStringDrawCentered(&sContext, "to continue.", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 40, false);

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
    {
    }

    //
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_2);


    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    //
    // Prompt for text to be entered.
    //
    UARTSend((uint8_t *) "\r\n", 2);
    UARTSend((uint8_t *)"Menu Selection:\r\n", 17);
    UARTSend((uint8_t *)"M - Print this menu\r\n", 21);
    UARTSend((uint8_t *)"Q - Quit the program\r\n", 22);
    UARTSend((uint8_t *)"L - Toggle loop count display\r\n", 31);
    UARTSend((uint8_t *)"B - Toggle button display\r\n", 27);
    UARTSend((uint8_t *)"U - Toggle UART RECV/XMIT display\r\n", 35);
    UARTSend((uint8_t *)"F - Toggle flood character enable\r\n", 35);
    UARTSend((uint8_t *)"D - Toggle flashing LED\r\n", 25);
    UARTSend((uint8_t *)"C - Clear terminal window\r\n", 27);
    UARTSend((uint8_t *)"\n", 1);

    //
    // Loop forever echoing data through the UART.
    //
    
    char str[SIZE];
    
    while(1)
    {
      counter++;
      if (UARTCharsAvail(UART0_BASE)) {
        int32_t local_char;
        local_char = UARTCharGetNonBlocking(UART0_BASE);
        if (local_char != -1) {
          gNumCharRcv++;
          process_menu(local_char);
        }
      }
      
      if (flood) {
        SysCtlDelay(DELAY_TIME);
        UARTSend((uint8_t *) "A", 1);
      }
      
      if (led) {
        //
        // Turn on the LED.
        //
        GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, GPIO_PIN_2);

        //
        // Delay for a bit.
        //
        for(ui32Loop = 0; ui32Loop < ON; ui32Loop++)
        {
        }

        //
        // Turn off the LED.
        //
        GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, 0);

        //
        // Delay for a bit.
        //
        for(ui32Loop = 0; ui32Loop < OFF; ui32Loop++)
        {
        }
      }
      
      if (quit) {
        UARTSend((uint8_t *) "Program Ended!", 14);
        return 0;
      }
      
      if (lcounter) {
            snprintf(str, sizeof(str), "%d", counter);
            GrStringDrawCentered(&sContext, str, -1,
                         GrContextDpyWidthGet(&sContext) / 2, 50, true);
      }
    }
}
