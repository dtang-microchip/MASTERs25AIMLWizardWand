/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes


// *****************************************************************************
// *****************************************************************************
// Section: Added includes
// *****************************************************************************
// *****************************************************************************
#include "ws2812b.h"

// *****************************************************************************
// *****************************************************************************
// Section: External prototypes
// *****************************************************************************
// *****************************************************************************
extern DMA_Descriptor_TypeLocal DMA_Descriptor_WB;
extern DMA_Descriptor_TypeLocal DMA_Descriptor;
extern WS2812_TYPE ws2812;
// *****************************************************************************
// *****************************************************************************
// Section: structs
// *****************************************************************************
// *****************************************************************************
//#define MAX_STATES          5          // Number of states in the state machine
#define DEBOUNCE_TIME_MS    50         // Debounce time in milliseconds

struct timermSS {
    uint8_t interval : 1;
    uint32_t count;
    uint32_t timeOut;
} timermS;

typedef enum {
    STATE_IDLE,
    STATE_INIT,
    STATE_RUNNING,
    STATE_ERROR,
    STATE_STANDBY,
    MAX_STATES
          
} State_t;
// *****************************************************************************
// *****************************************************************************
// Section: Internal Prototype
// *****************************************************************************
// *****************************************************************************

void Interval1mS(TC_TIMER_STATUS status, uintptr_t context);
void Clear_LEDS (void);
void execute_state_actions(void);
void transition_to_next_state(void);
void handle_button_press(void);


volatile State_t currentState = STATE_IDLE;
volatile State_t prevState;
volatile uint32_t systemTicks = 0;
volatile bool buttonPressed = false;
volatile uint32_t lastButtonPressTime = 0;
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    /* WS2812 initialization */
    initTimerDMA();
    
    /* Local Timer set-up */
    TC4_TimerStart();
    TC4_TimerCallbackRegister(Interval1mS, (uintptr_t) NULL);
    timermS.timeOut = 100; //100ms timer 
    
    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        
        
        /* for testing purposes this if statement will occur 
         every timermS.timeOut, which in this case is set to 100.*/
        if (timermS.interval == 1) {
        timermS.interval = 0;
        
        /* do stuff here , including changes in LED colors */
        if ( SWITCH_Get() == 0 )
        {
            handle_button_press();
        }
        
        execute_state_actions();
        
        /* ws2812 update is needed to feed the 
         * structure with any changed values to the peripherals */
        ws2812.update = 1;
        ws2812_Update();
        }
        
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/
void Interval1mS(TC_TIMER_STATUS status, uintptr_t context)
{   
    if (++timermS.count >= timermS.timeOut) {
        timermS.interval = 1;
        timermS.count = 0;
    }
    systemTicks++;
}

void Clear_LEDS (void)
{
    for (uint8_t i = 0; i < WS2812_NUM * 3; i++)
    {
        ws2812.color.array[i] = 0;
            
    }
}

// Handle button press with debouncing
void handle_button_press(void) {
    uint32_t currentTime = systemTicks;    
    // Check if debounce time has elapsed
    if (currentTime - lastButtonPressTime >= DEBOUNCE_TIME_MS) {
        // Transition to next state
        transition_to_next_state();   
        lastButtonPressTime = currentTime;
    }
}
// Transition to next state
void transition_to_next_state(void) {
    prevState = currentState;    
    // Increment state
    currentState = (currentState + 1) % MAX_STATES;

}
void execute_state_actions(void){
//    State_t prevState = currentState;
//    if ( currentState != prevState){
     switch(currentState) {
                case STATE_IDLE:
                    // Idle state actions
                    ws2812.color.led[0].red = 0x00;
                    ws2812.color.led[0].green = 0x00;
                    ws2812.color.led[0].blue= 0x00;
                  break;

                case STATE_INIT:
                    // Init state actions
                    ws2812.color.led[0].red = 0xFF;
                    ws2812.color.led[0].green = 0x00;
                    ws2812.color.led[0].blue= 0x00;
                   break;

                case STATE_RUNNING:
                    // Running state actions
                    ws2812.color.led[0].red = 0x00;
                    ws2812.color.led[0].green = 0xFF;
                    ws2812.color.led[0].blue= 0x00;
                    break;

                case STATE_ERROR:
                    // Error state actions
                    ws2812.color.led[0].red = 0xFF;
                    ws2812.color.led[0].green = 0x00;
                    ws2812.color.led[0].blue= 0x00;
                    break;

                    
                case STATE_STANDBY:
                    // Standby state actions
                    ws2812.color.led[0].red = 0x00;
                    ws2812.color.led[1].green = 0xFF;
                    ws2812.color.led[0].blue= 0xFF;
                    break;
                    
                default :
                    break;
            }
    //}
}