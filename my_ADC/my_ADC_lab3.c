//*****************************************************************************
// CEC 322 Laboratory Exercise #3
//
// Introduction to Analog Signal Acquisition & Display
//
// A polled. menu driven program that reads 3 analog inputs
// and displays each numerically, in a bar graph, or not at all.
//
// John Markham &&  Stav Shaham
// 22 September 2026
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
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
#include "driverlib/adc.h"      // ADC Library

//****************************************************************************
// Variable && Constants Defined
// ***************************************************************************

#define  BAUD_RATE 115200
#define HEARTBEAT_LOOPS_COUNT 100

// Channel Display Modes
#define CHANNEL_DISPLAY_OFF 0
#define NUMERIC_MODE 1
#define BAR_MODE 2
tContext sContext;

// User Defined variable for counting number of characters received
uint32_t gNumCharRecv = 0;
//Bool for LED flash enable
bool isFlashEnabled = true;
//user defined variable for counter
uint32_t counter = 0; 
//User defined variable for the quit functionality on the menu
uint32_t quit_variable = 1;

// Initialize and define channels to start in numeric mode
uint32_t channel_1_mode = NUMERIC_MODE;
uint32_t channel_2_mode = NUMERIC_MODE;
uint32_t channel_3_mode = NUMERIC_MODE;


//*****************************************************************************
// FUNCTION PROTOTYPING
//*****************************************************************************
void process_menu(int32_t i32char);
void print_menu(void); 
void heartbeat(void);
void splash_screen(void);

//*****************************************************************************
// The error routine that is called if the driver library encounters an error.
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
// The UART interrupt handler. No longer used for this lab.
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
        //
        // Read the next character from the UART and write it back to the UART.
        //
        UARTCharPutNonBlocking(UART0_BASE,
                                   UARTCharGetNonBlocking(UART0_BASE));
    }
}

//*****************************************************************************
// Send a string to the UART.
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


int main(void)
{
    tRectangle sRect;
    int32_t bar;
    volatile uint32_t ui32Loop;

    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    FPULazyStackingEnable();

    // Set the clocking to run directly from the crystal.
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    // Initialize the display driver.
    CFAL96x64x16Init();

    // Initialize the graphics context.
    GrContextInit(&sContext, &g_sCFAL96x64x16);

    // Set font
    GrContextFontSet(&sContext, g_psFontFixed6x8);

    // Enable the peripherals used by this example.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Set GPIO A0 and A1 as UART pins.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Configure the UART for 115,200, 8-N-1 operation.
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), BAUD_RATE,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    //Enable the user LED to use in function (portG)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

    //check if peripheral access is enabled
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
    {
    }

    //Enabling pin 2 (PG2) and setting direction as output
    GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_2); 

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
    GPIOPinTypeADC(GPIO_PORTK_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);

    //
    // Enable the first sample sequencer
    //
    ADCSequenceDisable(ADC0_BASE, 0);
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH16);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH17);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_END | ADC_CTL_CH18);
    ADCSequenceEnable(ADC0_BASE, 0);

    // Print the menu
    print_menu();

    // Startup splash screen from STEP 6
    splash_screen();

    // Prompt for text to be entered.
    UARTSend((uint8_t *)"Enter choice: ", 14);

    SysCtlDelay(6000000 * 4);
    
    while(1)
    {
      uint32_t nums[8];
      char str[100];
      //
      // Trigger the sample sequence.
      //
      ADCProcessorTrigger(ADC0_BASE, 0);
      ADCSequenceDataGet(ADC0_BASE, 0, nums);

        // Channel 1, 2, 3 display modes
        // CH 1
        // Print Number
        if(channel_1_mode == NUMERIC_MODE)
        {
            sprintf(str, "   %d   ", nums[0]);
            GrContextForegroundSet(&sContext, ClrWhite);
            GrStringDrawCentered(&sContext, str, -1,
                       GrContextDpyWidthGet(&sContext) / 2, 10, true);
        }
        // Print Bar
        if(channel_1_mode == BAR_MODE)
        {
            // Convert 0-4095 number into pixel widths 0-95
            bar = (nums[0] * GrContextDpyWidthGet(&sContext)) / 4096;

            // Draw red bar from left of screen
            sRect.i16XMin = 0;
            sRect.i16XMax = bar;
            sRect.i16YMin = 4;
            sRect.i16YMax = 16;
            GrContextForegroundSet(&sContext, ClrRed);
            GrRectFill(&sContext, &sRect);

            // Draw black bar to fill remaining space
            sRect.i16XMin = bar + 1;
            sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
            GrContextForegroundSet(&sContext, ClrBlack);
            GrRectFill(&sContext, &sRect);
        }
        // Display off
        if(channel_1_mode == CHANNEL_DISPLAY_OFF)
        {
            // Blank out channel 1
            sRect.i16XMin = 0;
            sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
            sRect.i16YMin = 0;
            sRect.i16YMax = 19;
            GrContextForegroundSet(&sContext, ClrBlack);
            GrRectFill(&sContext, &sRect);

        }

        // CH 2
        if(channel_2_mode == NUMERIC_MODE)
        {
            sprintf(str, "   %d   ", nums[1]);
            GrContextForegroundSet(&sContext, ClrWhite);
            GrStringDrawCentered(&sContext, str, -1,
                       GrContextDpyWidthGet(&sContext) / 2, 30, true);
        }

        if(channel_2_mode == BAR_MODE)
        {
            bar = (nums[1] * GrContextDpyWidthGet(&sContext)) / 4096;

            sRect.i16XMin = 0;
            sRect.i16XMax = bar;
            sRect.i16YMin = 24;
            sRect.i16YMax = 36;
            GrContextForegroundSet(&sContext, ClrBlue);
            GrRectFill(&sContext, &sRect);

            sRect.i16XMin = bar + 1;
            sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
            GrContextForegroundSet(&sContext, ClrBlack);
            GrRectFill(&sContext, &sRect);
        }

        if(channel_2_mode == CHANNEL_DISPLAY_OFF)
        {
            sRect.i16XMin = 0;
            sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
            sRect.i16YMin = 20;
            sRect.i16YMax = 39;
            GrContextForegroundSet(&sContext, ClrBlack);
            GrRectFill(&sContext, &sRect);
        }
        // CH 3
        if(channel_3_mode == NUMERIC_MODE)
        {
            sprintf(str, "   %d   ", nums[2]);
            GrContextForegroundSet(&sContext, ClrWhite);
            GrStringDrawCentered(&sContext, str, -1,
                       GrContextDpyWidthGet(&sContext) / 2, 50, true);
        }

        if(channel_3_mode == BAR_MODE)
        {
            bar = (nums[2] * GrContextDpyWidthGet(&sContext)) / 4096;

            sRect.i16XMin = 0;
            sRect.i16XMax = bar;
            sRect.i16YMin = 44;
            sRect.i16YMax = 56;
            GrContextForegroundSet(&sContext, ClrYellow);
            GrRectFill(&sContext, &sRect);

            sRect.i16XMin = bar + 1;
            sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
            GrContextForegroundSet(&sContext, ClrBlack);
            GrRectFill(&sContext, &sRect);
        }

        if(channel_3_mode == CHANNEL_DISPLAY_OFF)
        {
            sRect.i16XMin = 0;
            sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
            sRect.i16YMin = 40;
            sRect.i16YMax = 59;
            GrContextForegroundSet(&sContext, ClrBlack);
            GrRectFill(&sContext, &sRect);
        }


      counter++; 
      while(UARTCharsAvail(UART0_BASE))
        {
            int32_t local_char;
            local_char = UARTCharGetNonBlocking(UART0_BASE);

            if(local_char != -1)
            {
                gNumCharRecv++;
                process_menu(local_char);
            }
        }

        // Enabling flash LED Heartbeat
        heartbeat();
        // Enabling quit program
        if(quit_variable == 0)
        {
          return(0); 
        }
    }
    
}

// Function to process menu command
void process_menu(int32_t i32char)
{
    switch(i32char)
    {
    case 'm': //print menu 
            print_menu(); 
            break; 
    case 'q':  //quit 
            UARTSend("\r\nProgram Ended: \r\n", 19);
            quit_variable = 0; 
            break;
    case 'd': //toggle flashing lED
            isFlashEnabled = !isFlashEnabled; 
            break;
    case 'c': //clear display
            UARTSend("\f", 1);
            break; 
    case 's': //show splash screen STEP 6
        splash_screen();
        break;
    case '1': //cycle channel 1 display mode STEP 5
        if(channel_1_mode == NUMERIC_MODE)
        {
            channel_1_mode = BAR_MODE;
        }
        else if(channel_1_mode == BAR_MODE)
        {
            channel_1_mode = CHANNEL_DISPLAY_OFF;
        }
        else
        {
            channel_1_mode = NUMERIC_MODE;
        }
        break;
    case '2': //cycle channel 2 display mode
        if(channel_2_mode == NUMERIC_MODE)
        {
            channel_2_mode = BAR_MODE;
        }
        else if(channel_2_mode == BAR_MODE)
        {
            channel_2_mode = CHANNEL_DISPLAY_OFF;
        }
        else
        {
            channel_2_mode = NUMERIC_MODE;
        }
        break;
    case '3': //cycle channel 3 display mode
        if(channel_3_mode == NUMERIC_MODE)
        {
            channel_3_mode = BAR_MODE;
        }
        else if(channel_3_mode == BAR_MODE)
        {
            channel_3_mode = CHANNEL_DISPLAY_OFF;
        }
        else
        {
            channel_3_mode = NUMERIC_MODE;
        }
        break;
    default:
        break;
    }

}

// Function to print menu step #14
void print_menu(void)
{
    UARTSend("\r\nMenu: \r\n", 10);
    UARTSend("m: Print the menu\r\n", 19);
    UARTSend("q: Quit the Program\r\n", 21);
    UARTSend("d: Toggle flashing LED\r\n", 24);
    UARTSend("s: Show splash screen\r\n", 23);
    UARTSend("1: Channel 1 mode\r\n", 19);
    UARTSend("2: Channel 2 mode\r\n", 19);
    UARTSend("3: Channel 3 mode\r\n", 19);
    UARTSend("c: Clear terminal display\r\n", 27);
}

// LED Flash Heartbeat no blocking
void heartbeat(void)
{
    if(isFlashEnabled)
    {
        if((counter % HEARTBEAT_LOOPS_COUNT) == 0)
        {
            if(GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_2))
            {
                GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, 0);
            }
            else
            {
                GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, GPIO_PIN_2);
            }
        }
    }
}

// Show Flash Screen STEP 6
void splash_screen(void)
{
    tRectangle sRect;
    // Clear the whole screen first
    sRect.i16XMin = 0;
    sRect.i16YMin = 0;
    sRect.i16XMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.i16YMax = GrContextDpyHeightGet(&sContext) - 1;
    GrContextForegroundSet(&sContext, ClrBlack);
    GrRectFill(&sContext, &sRect);

    // Blue banner across the top, same as lab 2
    sRect.i16YMax = 9;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    // Custom team Splash
    GrContextForegroundSet(&sContext, ClrWhite);
    GrStringDrawCentered(&sContext, "CEC322 LAB 3", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 4, false);
    GrStringDrawCentered(&sContext, "John Markham", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 24, false);
    GrStringDrawCentered(&sContext, "Stav Shaham", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 36, false);

    // Hold it 2-4 seconds. SysCtlDelay runs a 3 cycle loop per count,
    // so SysCtlClockGet() counts is about 3 seconds at 16 MHz.
    SysCtlDelay(SysCtlClockGet());

    // Clear again so the channel display starts on a blank screen
    sRect.i16YMax = GrContextDpyHeightGet(&sContext) - 1;
    GrContextForegroundSet(&sContext, ClrBlack);
    GrRectFill(&sContext, &sRect);
    GrContextForegroundSet(&sContext, ClrWhite);
}