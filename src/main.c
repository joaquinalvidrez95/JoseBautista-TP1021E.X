/* 
 * File:   main.c
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on November 16, 2017, 11:19 AM
 */

#include <16F886.h>

#FUSES HS, PUT, NOLVP, PROTECT,NOMCLR, BROWNOUT, NODEBUG     
#use delay(clock=20M)
#use standard_io(a)
#use standard_io(c)
#use standard_io(b)
#use rtos(timer=0)
#byte WPUB = 0x95

#include "josedisplay.h"

// Pinout
#define PIN_BUTTON_MENU     PIN_B0
#define PIN_BUTTON_NEXT     PIN_B1
#define PIN_BUTTON_START    PIN_B4
#define PIN_LED             PIN_B3

// Timeouts
#define TIMEOUT_MENU_BUTTON_MILISECONDS 2000
#define TIMEOUT_RESET_TIMER_MILISECONDS 3000
#define TIMEOUT_HYPHENS_MILISECONDS     2000
#define TIMEOUT_OVERFLOW                3000

#define DELAY_INCREASE_NUMBER_MILISECONDS   200
#define DELAY_AFTER_TIMER_OVERFLOWED        500

// -------------------------FUNCTION PROTOTYPE----------------------------------
void setupHardware(void);
void blinkDisplay(void);
// -------------------------RTOS TASKS------------------------------------------
#task(rate=50ms, max=1ms)
void Task_checkIfStartStopResetButtonIsHeld(void);

#task(rate=50ms, max=1ms)
void Task_checkIfMenuButtonIsHeld(void);

#task(rate=5ms, max=5ms)
void Task_runStateMachine(void);

#task(rate=25ms, max=1ms)
void Task_blinkDisplay(void);
// ----------------------------GLOBAL VARIABLES---------------------------------
BOOLEAN showDisplayCompletely = TRUE;
ButtonState buttonStateStart = BUTTON_STATE_NOT_PUSHED;
ButtonState buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
JoseDisplay myDisplay;

BOOLEAN startButtonState = TRUE;
BOOLEAN menuButtonState = TRUE;
BOOLEAN nextButtonState = TRUE;
DisplayState nextStateAfterWaitingForButtonBeingReleased;
int numberOfMenuButtonHasBeenReleased = 0;

void main(void) {
    setupHardware();
    myDisplay.currentState = STATE_INIT;
    rtos_run();
}

void runStateMachinePart1(void) {
    switch (myDisplay.currentState) {
        case STATE_INIT:
            myDisplay = Display_new();
            Display_showCount(&myDisplay, FALSE);
            break;

        case STATE_IDLE:
            if (input(PIN_BUTTON_START) && (!startButtonState)) {
                Display_resume(&myDisplay);
                Display_saveState(&myDisplay);
                Display_updateRtc(&myDisplay);
                blinkDisplay();
                Display_showCount(&myDisplay, FALSE);
            }
            if (buttonStateStart == BUTTON_STATE_HELD) {
                buttonStateStart = BUTTON_STATE_NOT_PUSHED;
                Display_setState(&myDisplay, STATE_RESETTING);
            }
            break;

        case STATE_COUNTING_DOWN:
            Display_updateTimer(&myDisplay);
            Display_showCount(&myDisplay, TRUE);
            if (buttonStateStart == BUTTON_STATE_HELD) {
                buttonStateStart = BUTTON_STATE_NOT_PUSHED;
                Display_setState(&myDisplay, STATE_RESETTING);
            }
            if (input(PIN_BUTTON_START) && (!startButtonState)) {
                Display_stop(&myDisplay);
                Display_saveRtcCurrentTime(&myDisplay);
                Display_saveState(&myDisplay);
                blinkDisplay();
                Display_showCount(&myDisplay, FALSE);
            }
            if (Display_isTimerDone(&myDisplay)) {
                blinkDisplay();
                blinkDisplay();                
                Display_clearRtc();
                Display_updateTimer(&myDisplay);
                Display_setState(&myDisplay, STATE_READY);
                Display_saveState(&myDisplay);
            }
            break;

        case STATE_RESETTING:
            Display_clearRtc();
            SeventSegmentDisplay_showHyphensTwoDigits();
            delay_ms(TIMEOUT_HYPHENS_MILISECONDS);
            Display_setState(&myDisplay, STATE_WAITING_FOR_BUTTON_BEING_RELEASED);
            nextStateAfterWaitingForButtonBeingReleased = STATE_READY;
            Display_clearRtc();
            Display_updateTimer(&myDisplay);
            Display_showCount(&myDisplay, FALSE);
            break;

        case STATE_WAITING_FOR_BUTTON_BEING_RELEASED:
            if (input(PIN_BUTTON_START) &&
                    input(PIN_BUTTON_MENU) &&
                    input(PIN_BUTTON_NEXT)) {
                Display_setState(&myDisplay, nextStateAfterWaitingForButtonBeingReleased);
                Display_saveState(&myDisplay);
            }
            break;

        case STATE_READY:
            rtos_disable(Task_checkIfStartStopResetButtonIsHeld);
            Display_showCount(&myDisplay, FALSE);
            if (input(PIN_BUTTON_START) && (!startButtonState)) {
                blinkDisplay();
                Display_setState(&myDisplay, STATE_COUNTING_DOWN);
                Display_saveState(&myDisplay);
                Display_clearRtc();
                rtos_enable(Task_checkIfStartStopResetButtonIsHeld);
            }
            if (buttonStateMenu == BUTTON_STATE_HELD) {
                buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
                Display_setState(&myDisplay, STATE_SETTING_ALARM);
            }
            break;
    }
}

void Task_runStateMachine(void) {
    runStateMachinePart1();
    switch (myDisplay.currentState) {
        case STATE_SETTING_ALARM:
            if (showDisplayCompletely) {
                Display_showLimitTime(&myDisplay);
            } else {
                SevenSegmentDisplay_clearDisplayTwoLines();
            }
            if (!input(PIN_BUTTON_NEXT)) {
                while (!input(PIN_BUTTON_NEXT)) {
                    Display_increaseAlarm(&myDisplay);
                    Display_showLimitTime(&myDisplay);
                    delay_ms(DELAY_INCREASE_NUMBER_MILISECONDS);
                }
            }
            if (numberOfMenuButtonHasBeenReleased >= 2) {
                numberOfMenuButtonHasBeenReleased = 0;
                Display_setState(&myDisplay, STATE_SETTING_FORMAT);
            }
            if (!menuButtonState && input(PIN_BUTTON_MENU) &&
                    Display_isAlarmOkay(&myDisplay)) {
                numberOfMenuButtonHasBeenReleased++;

            }

            break;

        case STATE_SETTING_FORMAT:
            if (showDisplayCompletely) {
                Display_showFormat(&myDisplay);
            } else {
                SevenSegmentDisplay_clearDisplayTwoLines();
            }

            if (input(PIN_BUTTON_NEXT) && (!nextButtonState)) {
                Display_swapFormat(&myDisplay);
            }
            if (!menuButtonState && input(PIN_BUTTON_MENU)) {
                Display_saveAlarm(&myDisplay);
                Display_saveFormat(&myDisplay);
                Display_clearRtc();
                Display_updateTimer(&myDisplay);
                Display_setState(&myDisplay, STATE_READY);
                Display_saveState(&myDisplay);
            }
            break;
    }

    startButtonState = input(PIN_BUTTON_START);
    menuButtonState = input(PIN_BUTTON_MENU);
    nextButtonState = input(PIN_BUTTON_NEXT);
    rtos_yield();
}

void Task_checkIfStartStopResetButtonIsHeld(void) {
    static int nextUpButtonCounter = 0;
    if (!input(PIN_BUTTON_START)) {
        nextUpButtonCounter++;
    } else {
        nextUpButtonCounter = 0;
        buttonStateStart = BUTTON_STATE_NOT_PUSHED;
    }
    if (nextUpButtonCounter >= (TIMEOUT_RESET_TIMER_MILISECONDS / 50 / 3)) {
        buttonStateStart = BUTTON_STATE_HELD;
        nextUpButtonCounter = 0;
    }
    rtos_yield();
}

void Task_checkIfMenuButtonIsHeld(void) {
    static int menuButtonCounter = 0;

    if (!input(PIN_BUTTON_MENU)) {
        menuButtonCounter++;
    } else {
        menuButtonCounter = 0;
        buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
    }
    if (menuButtonCounter >= (TIMEOUT_MENU_BUTTON_MILISECONDS / 50 / 3)) {
        buttonStateMenu = BUTTON_STATE_HELD;
        menuButtonCounter = 0;
    }
    rtos_yield();
}

void Task_blinkDisplay(void) {
    showDisplayCompletely = ~showDisplayCompletely;
    rtos_yield();
}

void setupHardware(void) {
    delay_ms(200);
    port_b_pullups(0xFF);
    WPUB = 0xFF;
}

void blinkDisplay(void) {
    Display_showCount(&myDisplay, FALSE);
    delay_ms(200);
    SevenSegmentDisplay_clearDisplay();
    delay_ms(200);
    Display_showCount(&myDisplay, FALSE);
    //    delay_ms(200);
}

