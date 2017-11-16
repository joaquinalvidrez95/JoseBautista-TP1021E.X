/* 
 * File:   josedisplay.h
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on November 16, 2017, 11:20 AM
 */

#ifndef JOSEDISPLAY_H
#define	JOSEDISPLAY_H



// Upper bounds
#define ALARM_UPPER_BOUND    99
#define SECOND_NUMBER_UPPER_BOUND   59

#include "timer.h"
#include "buttons.h"
#include "sevensegmentdisplay.h"

typedef enum {
    EEPROM_CURRENT_STATE = 0,
    EEPROM_PREVIOUS_STATE,
    EEPROM_FORMAT,
    EEPROM_ALARM,
    EEPROM_RTC_HOURS,
    EEPROM_RTC_MINUTES,
    EEPROM_RTC_SECONDS
} EEPROM_ADDRESS;

typedef enum {
    STATE_IDLE = 0,
    STATE_READY,
    STATE_COUNTING_DOWN,
    UPPER_BOUND_STATE,
    STATE_INIT,
    STATE_RESETTING,
    STATE_SETTING_ALARM,  
    STATE_SETTING_FORMAT,
    STATE_WAITING_FOR_BUTTON_BEING_RELEASED,
    NUMBER_OF_STATES
} DisplayState;

typedef struct {
    Timer timer;
    TimerFormat format;
    DisplayState currentState;
    DisplayState previousState;
} JoseDisplay;

void Display_updateRtc(JoseDisplay *displayPtr) {
    Time time;
    time = displayPtr->timer.currentTime;
    if (displayPtr->format == FORMAT_MINUTES) {
        time.hour = time.minute / 60;
        time.minute %= 60;
    } else if (displayPtr->format == FORMAT_SECONDS) {
        time.hour = 0;
        time.minute = time.second / 60;
        time.second %= 60;
    }
    Time_setClockTime(&time);
}

void Display_updateTimer(JoseDisplay *displayPtr) {
    displayPtr->timer.currentTime = Time_getCurrentTime();
    if (displayPtr->format == FORMAT_MINUTES) {
        displayPtr->timer.currentTime.minute += (displayPtr->timer.currentTime.hour % 2) * 60;
    } else if (displayPtr->format == FORMAT_SECONDS) {
        displayPtr->timer.currentTime.second += (displayPtr->timer.currentTime.minute % 2) * 60;
    }
    Timer_updateCountdownTime(&displayPtr->timer);
}

void Display_clearRtc(void) {
    Time_clearRtcTime();
    setDate(1, 1, 1, 1);
}

void Display_showCount(JoseDisplay *displayPtr, BOOLEAN withBlink) {
    if (displayPtr->format == FORMAT_MINUTES) {
        Timer_showMinutesOfCountdownTime(&displayPtr->timer, withBlink);
    } else if (displayPtr->format == FORMAT_SECONDS) {
        Timer_showSecondsOfCountdownTime(&displayPtr->timer);
    }
}

void Display_showFormat(JoseDisplay *displayPtr) {
    if (displayPtr->format == FORMAT_MINUTES) {
        Timer_showFormatMinutes();
    } else if (displayPtr->format == FORMAT_SECONDS) {
        Timer_showFormatSeconds();
    }
}

void Display_resume(JoseDisplay *displayPtr) {
    displayPtr->previousState = displayPtr->currentState;
    displayPtr->currentState = STATE_COUNTING_DOWN;
}

void Display_saveState(JoseDisplay *displayPtr) {
    write_eeprom(EEPROM_CURRENT_STATE, displayPtr->currentState);
    write_eeprom(EEPROM_PREVIOUS_STATE, displayPtr->previousState);
}

void Display_setState(JoseDisplay *displayPtr,
        DisplayState displayState) {
    displayPtr->previousState = displayPtr->currentState;
    displayPtr->currentState = displayState;
}

void Display_stop(JoseDisplay *displayPtr) {
    displayPtr->previousState = displayPtr->currentState;
    displayPtr->currentState = STATE_IDLE;
}

void Display_saveRtcCurrentTime(JoseDisplay *displayPtr) {
    write_eeprom(EEPROM_RTC_HOURS, displayPtr->timer.currentTime.hour);
    write_eeprom(EEPROM_RTC_MINUTES, displayPtr->timer.currentTime.minute);
    write_eeprom(EEPROM_RTC_SECONDS, displayPtr->timer.currentTime.second);
}

BOOLEAN Display_isTimerDone(JoseDisplay *displayPtr) {
    return Timer_isTimerFinished(&displayPtr->timer);
}

void Display_swapFormat(JoseDisplay *displayPtr) {
    if (displayPtr->format == FORMAT_MINUTES) {
        displayPtr->format = FORMAT_SECONDS;
        displayPtr->timer.alarmTime.second = displayPtr->timer.alarmTime.minute;
        displayPtr->timer.alarmTime.minute = 0;
        displayPtr->timer.alarmTime.hour = 0;
        displayPtr->timer.hoursUpperBound = 0;
        displayPtr->timer.minutesUpperBound = 0;
        displayPtr->timer.secondsUpperBound = ALARM_UPPER_BOUND;

    } else if (displayPtr->format == FORMAT_SECONDS) {
        displayPtr->format = FORMAT_MINUTES;
        displayPtr->timer.alarmTime.hour = 0;
        displayPtr->timer.alarmTime.minute = displayPtr->timer.alarmTime.second;
        displayPtr->timer.alarmTime.second = 0;
        displayPtr->timer.hoursUpperBound = 0;
        displayPtr->timer.minutesUpperBound = ALARM_UPPER_BOUND;
        displayPtr->timer.secondsUpperBound = SECOND_NUMBER_UPPER_BOUND;
    }
}

void Display_saveFormat(JoseDisplay *displayPtr) {
    write_eeprom(EEPROM_FORMAT, displayPtr->format);
}

void Display_saveAlarm(JoseDisplay *displayPtr) {
    if (displayPtr->format == FORMAT_MINUTES) {
        write_eeprom(EEPROM_ALARM, displayPtr->timer.alarmTime.minute);
    } else if (displayPtr->format == FORMAT_SECONDS) {
        write_eeprom(EEPROM_ALARM, displayPtr->timer.alarmTime.second);
    }
}

void Display_showLimitTime(JoseDisplay *displayPtr) {
    if (displayPtr->format == FORMAT_MINUTES) {
        Timer_showMinutesOfLimitTime(&displayPtr->timer);
    } else if (displayPtr->format == FORMAT_SECONDS) {
        Timer_showSecondsOfLimitTime(&displayPtr->timer);
    }
}

void Display_increaseAlarm(JoseDisplay *displayPtr) {
    if (displayPtr->format == FORMAT_MINUTES) {
        Timer_increaseTimerMinutes(&displayPtr->timer);
    } else if (displayPtr->format == FORMAT_SECONDS) {
        Timer_increaseTimerSeconds(&displayPtr->timer);
    }
}

BOOLEAN Display_isAlarmOkay(JoseDisplay *displayPtr) {
    if (displayPtr->format == FORMAT_MINUTES) {
        return (displayPtr->timer.alarmTime.minute != 0);
    } else if (displayPtr->format == FORMAT_SECONDS) {
        return (displayPtr->timer.alarmTime.second != 0);
    }
}

JoseDisplay Display_new(void) {
    JoseDisplay erickaDisplay;

    erickaDisplay.format = read_eeprom(EEPROM_FORMAT) % 2;
    erickaDisplay.currentState = read_eeprom(EEPROM_CURRENT_STATE) % UPPER_BOUND_STATE;
    erickaDisplay.previousState = read_eeprom(EEPROM_PREVIOUS_STATE) % UPPER_BOUND_STATE;

    if ((erickaDisplay.previousState == STATE_IDLE)
            && (erickaDisplay.currentState == STATE_IDLE)) {
        erickaDisplay.previousState = STATE_COUNTING_DOWN;
    }

    switch (erickaDisplay.format) {
        case FORMAT_MINUTES:
            erickaDisplay.timer = Timer_newMinutes(ALARM_UPPER_BOUND);

            erickaDisplay.timer.alarmTime.hour = 0;
            erickaDisplay.timer.alarmTime.minute = read_eeprom(
                    EEPROM_ALARM) % (ALARM_UPPER_BOUND + 1);
            erickaDisplay.timer.alarmTime.second = 0;
            break;
        case FORMAT_SECONDS:
            erickaDisplay.timer = Timer_newSeconds(ALARM_UPPER_BOUND);
            erickaDisplay.timer.alarmTime.hour = 0;
            erickaDisplay.timer.alarmTime.minute = 0;
            erickaDisplay.timer.alarmTime.second = read_eeprom(
                    EEPROM_ALARM) % (ALARM_UPPER_BOUND + 1);

            break;
    }

    switch (erickaDisplay.currentState) {
        case STATE_IDLE:
            Timer_updateTimerFromEeprom(
                    &erickaDisplay.timer,
                    EEPROM_RTC_HOURS,
                    EEPROM_RTC_MINUTES,
                    EEPROM_RTC_SECONDS
                    );
            Display_updateRtc(&erickaDisplay);
            Timer_updateCountdownTime(&erickaDisplay);
            break;
        case STATE_READY:
            Display_clearRtc();
            Display_updateTimer(&erickaDisplay);
            Display_showCount(&erickaDisplay, FALSE);
            break;

        case STATE_COUNTING_DOWN:
            Display_updateTimer(&erickaDisplay);
            if (Display_isTimerDone(&erickaDisplay)) {
                Display_clearRtc();
                Display_updateTimer(&erickaDisplay);
                Display_setState(&erickaDisplay, STATE_READY);
                Display_saveState(&erickaDisplay);
            }
            break;
    }

    return erickaDisplay;
}

#endif	/* JOSEDISPLAY_H */

