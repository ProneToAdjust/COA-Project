/* gcc test.c -l pigpio -l pthread
   sudo ./a.out */

#include <pigpio.h>
#include <unistd.h>
#include <pthread.h>

void initialize();
void *on_leds();
void *off_leds();
void *blink_leds_2Hz();
void *blink_red_8Hz();
void *blink_green_2Hz_reduced_brightness();

int GREEN_LED = 18;
int RED_LED = 19;

void initialize()
{
    if (gpioInitialise() < 0)
    {
        // pigpio initialisation failed.
    }
    else
    {
        // pigpio initialised okay.
        gpioSetMode(GREEN_LED, PI_OUTPUT);
        gpioSetMode(RED_LED, PI_OUTPUT);
    }
}

void *on_leds()
{
    gpioWrite(GREEN_LED, 1);
    gpioWrite(RED_LED, 1);
}

void *off_leds()
{
    gpioWrite(GREEN_LED, 0);
    gpioWrite(RED_LED, 0);
}

void *blink_leds_2Hz()
{
    while (1)
    {
        on_leds();
        usleep(250000);
        off_leds();
        usleep(250000);
    }
}

void *blink_red_8Hz()
{
    while (1)
    {
        gpioWrite(RED_LED, 1);
        usleep(62500);
        gpioWrite(RED_LED, 0);
        usleep(62500);
    }
}

void *blink_green_2Hz_reduced_brightness()
{
    while (1)
    {
        gpioPWM(GREEN_LED, 63);
        usleep(250000);
        gpioWrite(GREEN_LED, 0);
        usleep(250000);
    }
}