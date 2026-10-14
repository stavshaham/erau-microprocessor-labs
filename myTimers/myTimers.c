//*****************************************************************************
//
// timers.c - Timers example.
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
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "grlib/grlib.h"
#include "drivers/cfal96x64x16.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"


// ---- State Tracker

// --- GLOBAL VARIABLES
int SRV = 0;
int REQ = 0;
uint32_t pui32ADC0Value[1];
tContext g_sContext;


//Bool for LED flash enable
bool isFlashEnabled = false; 

//bool for splash screen
bool isSplashScreen = false;

// print SRV (mode)
bool printSRV = false;

// ---- Function Prototypes

// User defined fucntion for processing menu commands STEP #12
void process_menu(int32_t i32char);

// User defined function for printing menu: 
void print_menu(void); 

// Array counters
char showREQ[10];
char showSRV[10];
char showCount[10];


//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Timer (timers)</h1>
//!
//! This example application demonstrates the use of the timers to generate
//! periodic interrupts.  One timer is set up to interrupt once per second and
//! the other to interrupt twice per second; each interrupt handler will toggle
//! its own indicator on the display.
//
//*****************************************************************************

//*****************************************************************************
//
// Flags that contain the current value of the interrupt indicator as displayed
// on the CSTN display.
//
//*****************************************************************************
uint32_t g_ui32Flags;

//*****************************************************************************
//
// Graphics context used to show text on the CSTN display.
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
// The UART interrupt handler.
//
//*****************************************************************************
void
UARTIntHandler(void)
{
    uint32_t ui32Status;

    //
    // Get the interrrupt status.
    //
    ui32Status = UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    UARTIntClear(UART0_BASE, ui32Status);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while(UARTCharsAvail(UART0_BASE))
    {
        int32_t local_char;
        local_char = UARTCharGetNonBlocking(UART0_BASE);

        if(local_char != -1)
        {
            process_menu(local_char);
        }
    }
}



//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
//
//*****************************************************************************
void
Timer0IntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    
   //
    // Trigger the ADC conversion.
    //
    ADCProcessorTrigger(ADC0_BASE, 0);
   
    //
    // Read ADC Value.
    //
    ADCSequenceDataGet(ADC0_BASE, 0, pui32ADC0Value);


    REQ = pui32ADC0Value[0];
    
    
    if (printSRV){
      REQ /= 10;
    }
    else{
      REQ *= 100;
    }
    ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, ROM_SysCtlClockGet() / REQ);

    ROM_IntMasterDisable();
    
    sprintf(showREQ, "%d ", REQ);
    sprintf(showCount, "%d ", SRV);
    
    GrStringDraw(&g_sContext, showREQ, -1, 58, 36, 1);
    GrStringDraw(&g_sContext, showCount, -1, 58, 26, 1);

    ROM_IntMasterEnable();
    
    SRV = 0;
}

//*****************************************************************************
//
// The interrupt handler for the second timer interrupt.
//
//*****************************************************************************
void
Timer1IntHandler(void)
{
      // Clear the timer interrupt.
    //
    ROM_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    
    // SHOULD ALWAYS INCREMENT ONLY DISPLAY WHEN DIVIDED BY 10
    SRV++;
    
    if (printSRV) {
          sprintf(showSRV, "%d ", SRV);
          ROM_IntMasterDisable();
          GrStringDraw(&g_sContext, showSRV, -1, 58, 46, 1);
          ROM_IntMasterEnable();
    } 

}



//*****************************************************************************
//
// \ a string to the UART.
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


//*****************************************************************************
//
// This example application demonstrates the use of the timers to generate
// periodic interrupts.
//
//*****************************************************************************
int
main(void)
{
    volatile uint32_t ui32Loop;
    tRectangle sRect;

    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    FPULazyStackingEnable();

    //
    // Set the clocking to run directly from the crystal.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);
    
    

    //
    // Initialize the display driver.
    //
    CFAL96x64x16Init();

    //     ------------------- Sad configuration for ADC: -----------------------
    // The ADC0 peripheral must be enabled for use.
    //

    
    
    //
    // Initialize the graphics context and find the middle X coordinate.
    //
    GrContextInit(&g_sContext, &g_sCFAL96x64x16);

    //
    // Fill the top part of the screen with blue to create the banner.
    //
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&g_sContext) - 1;
    sRect.i16YMax = 9;
    GrContextForegroundSet(&g_sContext, ClrDarkBlue);
    GrRectFill(&g_sContext, &sRect);

    //
    // Change foreground for white text.
    //
    GrContextForegroundSet(&g_sContext, ClrWhite);

    
    
    //UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    
    //check if pheripheral access is enabled 
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    
    //check if pheripheral access is enabled 
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
    {
    }
    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    
    //
    // Set GPIO A0 and A1 as UART pins.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    // Enable the UART interrupt.
    //
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    
    //led
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    //check if pheripheral access is enabled 
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
    {
    }
    //Enabling pin 2 (PG2) and setting direction as output
    GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_2);
    
    // ADC
    // Enable ADC0 for analog inputs Step 7
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    //check if peripheral access is enabled ADC
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
    {
    }

    // Enable Port K
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);

    // Check if peripheral access is enabled GPIO
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK))
    {
    }

    // PK0 (AIN16), PK1 (AIN17), PK2(AIN18)
    GPIOPinTypeADC(GPIO_PORTK_BASE, GPIO_PIN_1);

    //
    // Enable the first sample sequencer
    //
    ADCSequenceDisable(ADC0_BASE, 0);
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_END | ADC_CTL_CH17);
    ADCSequenceEnable(ADC0_BASE, 0);
    
    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&g_sContext, g_psFontFixed6x8);
    GrStringDrawCentered(&g_sContext, "timers", -1,
                         GrContextDpyWidthGet(&g_sContext) / 2, 4, 0);

    //
    // Initialize timer status display.
    //
    GrContextFontSet(&g_sContext, g_psFontFixed6x8);
    GrStringDraw(&g_sContext, "SRV:", -1, 16, 26, 0);
    GrStringDraw(&g_sContext, "REQ:", -1, 16, 36, 0);
    GrStringDraw(&g_sContext, "COUNT:", -1, 16, 46, 0);

    //
    // Enable the peripherals used by this example.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    //
    // Enable processor interrupts.
    //
    ROM_IntMasterEnable();

    //
    // Configure the two 32-bit periodic timers.
    //
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet());
    ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, ROM_SysCtlClockGet() / 2);

    //
    // Setup the interrupts for the timer timeouts.
    //
    ROM_IntEnable(INT_TIMER0A);
    ROM_IntEnable(INT_TIMER1A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the timers.
    //
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
    ROM_TimerEnable(TIMER1_BASE, TIMER_A);

    //
    // Loop forever while the timers run.
    //
    
    print_menu();
    ROM_IntMasterEnable();
    while(1)
    {
      // -----------------------------------------------------------------------------
     
      
      //
      // Display the AIN0 (PE3) digital value on the console.
      //
      // UARTprintf("AIN0 = %4d\r", pui32ADC0Value[0]);
  
      // enabling flash LED
      if(isFlashEnabled)
      {
        // Turn on the LED.
        GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, GPIO_PIN_2);

        // Delay for a bit.
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++)
        {
        }

        // Turn off the LED.
        GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, 0);

        // Delay for a bit.
        for(ui32Loop = 0; ui32Loop < 200000; ui32Loop++)
        {
        }
      }
      
        //enabling splash screen
      if(isSplashScreen)
      {

        // Clears screen
        sRect.i16XMin = 0;
        sRect.i16YMin = 0;
        sRect.i16XMax = 100;
        sRect.i16YMax = 100;
        GrContextForegroundSet(&g_sContext, ClrBlack);
        GrRectFill(&g_sContext, &sRect);
        
        // Splash Screen
        sRect.i16XMin = 0;
        sRect.i16YMin = 0;
        sRect.i16XMax = GrContextDpyWidthGet(&g_sContext) - 1;
        sRect.i16YMax = 9;
        GrContextForegroundSet(&g_sContext, ClrDarkBlue);
        GrRectFill(&g_sContext, &sRect);
        GrContextForegroundSet(&g_sContext, ClrWhite);
        GrContextFontSet(&g_sContext, g_psFontFixed6x8);
        GrStringDrawCentered(&g_sContext, "Preston Stav", -1,
                             GrContextDpyWidthGet(&g_sContext) / 2, 4, 0);
       
        SysCtlDelay(5000000);
        
        // Clears screen
        sRect.i16XMin = 0;
        sRect.i16YMin = 0;
        sRect.i16XMax = 100;
        sRect.i16YMax = 100; 
        GrContextForegroundSet(&g_sContext, ClrBlack);
        GrRectFill(&g_sContext, &sRect);
        
        
        // For Timer
         sRect.i16XMin = 0;
         sRect.i16YMin = 0;
         sRect.i16XMax = GrContextDpyWidthGet(&g_sContext) - 1;
         sRect.i16YMax = 9;
         GrContextForegroundSet(&g_sContext, ClrDarkBlue);
         GrRectFill(&g_sContext, &sRect);

         //
         // Change foreground for white text.
         //
         GrContextForegroundSet(&g_sContext, ClrWhite);

         //
         // Put the application name in the middle of the banner.
         //
         GrContextFontSet(&g_sContext, g_psFontFixed6x8);
         GrStringDrawCentered(&g_sContext, "timers", -1,
                               GrContextDpyWidthGet(&g_sContext) / 2, 4, 0);

         //
         // Initialize timer status display.
         //
         GrContextFontSet(&g_sContext, g_psFontFixed6x8);
         GrStringDraw(&g_sContext, "SRV:", -1, 16, 26, 0);
         GrStringDraw(&g_sContext, "REQ:", -1, 16, 36, 0);
         GrStringDraw(&g_sContext, "COUNT:", -1, 16, 46, 0);
        
        
        isSplashScreen = !isSplashScreen;

      }

    }
}


void process_menu(int32_t i32char)
{
    tRectangle sRect;
    switch(i32char)
    {
      case 'm': //print menu 
              print_menu(); 
              break; 
      case 'd': //toggle flashing lED
              isFlashEnabled = !isFlashEnabled; 
              break; 
      case 's': 
              printSRV = !printSRV;
              SRV = 0;
              REQ = 0;
              
              ROM_IntMasterDisable();
              
              // Clears screen
              sRect.i16XMin = 80;
              sRect.i16YMin = 20;
              sRect.i16XMax = 100;
              sRect.i16YMax = 100; 
              GrContextForegroundSet(&g_sContext, ClrBlack);
              GrRectFill(&g_sContext, &sRect);
                            
              if (!printSRV) {
                  sRect.i16XMin = 0;
                  sRect.i16YMin = 46;
                  sRect.i16XMax = 100;
                  sRect.i16YMax = 100; 
                  GrContextForegroundSet(&g_sContext, ClrBlack);
                  GrRectFill(&g_sContext, &sRect);
              }

              GrContextForegroundSet(&g_sContext, ClrWhite);
                
              ROM_IntMasterEnable();
              
              break;
      case 'c': //clear display
              UARTSend("\f", 1);
              break; 
      case 'p':
            isSplashScreen = !isSplashScreen;
            break;
      
      default:
          break;
    }

}

// function to print menu step #14
void print_menu(void)
{
  UARTSend("\r\nMenu: \r\n", 10); 
  UARTSend("m: Print the menu\r\nq: Quit the Program\r\nd: Toggle flashing LED\r\nf: Toggle flood character enable\r\n", 98);
  UARTSend("p: Toggle splash screen\r\n", 26); 
  UARTSend("s: Toggle SRV\r\n", 26); 
  UARTSend("c: Clear terminal display\r\n", 28);
}

