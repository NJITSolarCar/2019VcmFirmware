/*
 * indicator.h
 *
 * Controls the color indicator LED. The indicator LED is an RGB LED on the
 * dashboard, designed to display status info to the driver at all times. There
 * are several defined colors and "blink states" that correspond to different
 * statuses, faults, and information. Blink states refer to the frequency of the
 * blinks, in units of 0.1 Hz. While blinking the LED will either be on (using
 * the currently set color) or completely off.
 *
 * This uses the following hardware devices:
 *
 *
 *
 *  Created on: Mar 9, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_INDICATOR_H_
#define SRC_DEVICE_INDICATOR_H_

// Color definitions
#define LED_COLOR_RED           {255, 0, 0}
#define LED_COLOR_GREEN         {0, 255, 0}
#define LED_COLOR_BLUE          {0, 0, 255}
#define LED_COLOR_MAGENTA       {255, 0, 255}
#define LED_COLOR_CYAN          {0, 255, 255}
#define LED_COLOR_ORANGE        {255, 150, 0}
#define LED_COLOR_MAGENTA       {255, 0, 255}
#define LED_COLOR_LIGHTGREEN    {150, 255, 40}

// Status to color / blink mappings
// <<TO BE ADDED LATER>>


#define INDICATOR_PWM_FULL_LOAD		255


/**
 * Represents an RGB color (as PWM duties from 0-255) for the LED. These
 * map directly to PWM duties, where 0 is full off and 255 is full on.
 */
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} tRGBColor;


/**
 * Describes a blink pattern for the LED. A blink pattern is composed of duty
 * and frequency. Duty represents the fraction of the time that the LED is on in
 * the cycle, measured in percent. Freq is the frequency of the blinking, in units
 * of 0.1Hz. A blink frequency of 0 means the light will always stay on
 */
typedef struct {
    uint8_t duty;
    uint8_t freq;
} tBlinkPattern;

/**
 * Convenience structure to hold the complete desciptor of the LED status:
 * blink pattern and color. Useful to make predefined LED stated for different
 * bits of information to display (e.g. a predefined state for BMS faults
 * active as a single constant)
 */
typedef struct {
    tBlinkPattern blinkPattern;
    tRGBColor color;
} tLEDState;


/**
 * Initializes the RGB LED system hardware and timer. Clocks must be
 * enabled before calling this.
 */
void indicator_init();

/**
 * Sets the pattern of the LED. This will immediately restart
 * the blink cycle. If the indicator is not currently running, this
 * call will restart it.
 *
 * @param state the LED state to put the light in
 */
void indicator_setPattern(tLEDState state);


/**
 * Shuts off the indicator LED, and the associated timers. It can be restarted
 * with a call to indicator_setPattern(...)
 */
void indiator_disable();

#endif /* SRC_DEVICE_INDICATOR_H_ */
