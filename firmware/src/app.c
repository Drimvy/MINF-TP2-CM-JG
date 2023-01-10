/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "GestPWM.h"
#include "Mc32DriverLcd.h"
#include "Mc32Delays.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;
S_pwmSettings PwmData;

uint8_t LedOffFlag = 1;
uint8_t chenillard = 0b00000001;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    APP_UpdateState(APP_STATE_INIT);

    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            /* Initialisation Displaying */
            lcd_init(); 
            printf_lcd("TP1 PWM 2022-2023");
            lcd_gotoxy(1,2);
            printf_lcd("Ali Zoubir"); 
            lcd_gotoxy(1,3);
            printf_lcd("Caroline Miéville"); 
            lcd_bl_on();
            
            /* Peripherals initalisations */
            /*initalisation des timers*/
            // DRV_TMR0_Start();  
            DRV_TMR0_Start();
            // DRV_TMR1_Start();  
            DRV_TMR1_Start();           
            // DRV_TMR2_Start();  
            DRV_TMR2_Start();            
            // DRV_TMR3_Start();  
            DRV_TMR3_Start();
            
            /*Initialisation des OC*/
            //init OC0 
            DRV_OC0_Start();
            //init OC1 
            DRV_OC1_Start(); 
            
            //Initialisation l'ADc
            BSP_InitADC10();
            
            //initialiser le Hbrige
            BSP_EnableHbrige();
            
            /* Initialize GPWM */
            GPWM_Initialize(&PwmData);
            
            APP_UpdateState(APP_STATE_WAIT);
            
            /* All LEDS ON */
            APP_LedMask(0x00);
            
            break;
        }
        case APP_STATE_WAIT:
        {
            
            break;
        }
        case APP_STATE_SERVICE_TASKS:
        {
            
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}



void APP_UpdateState(APP_STATES newState)
{
    appData.state = newState;
}

void APP_LedMask(uint8_t LedVal)
{
    if((LedVal&0b00000001) != 0)
        BSP_LEDOn(BSP_LED_0);
    else
       BSP_LEDOff(BSP_LED_0); 
    if((LedVal&0b00000010)>>1 != 0)
        BSP_LEDOn(BSP_LED_1);
    else
       BSP_LEDOff(BSP_LED_1); 
    if((LedVal&0b00000100)>>2 != 0)
        BSP_LEDOn(BSP_LED_2);
    else
       BSP_LEDOff(BSP_LED_2); 
    if((LedVal&0b00001000)>>3 != 0)
        BSP_LEDOn(BSP_LED_3);
    else
       BSP_LEDOff(BSP_LED_3); 
    if((LedVal&0b00010000)>>4 != 0)
        BSP_LEDOn(BSP_LED_4);
    else
       BSP_LEDOff(BSP_LED_4);    
    if((LedVal&0b00100000)>>5 != 0)
        BSP_LEDOn(BSP_LED_5);
    else
       BSP_LEDOff(BSP_LED_5);  
    if((LedVal&0b01000000)>>6 != 0)
        BSP_LEDOn(BSP_LED_6);
    else
       BSP_LEDOff(BSP_LED_6);   
    if((LedVal&0b10000000)>>7 != 0)
        BSP_LEDOn(BSP_LED_7);
    else
       BSP_LEDOff(BSP_LED_7); 
}

/*******************************************************************************
 End of File
 */
