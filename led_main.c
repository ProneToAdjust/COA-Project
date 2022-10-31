/* gcc test.c -l pigpio -l pthread
   sudo ./a.out */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <pigpio.h>

pthread_t blink_both, red_eight, green_reduce, end_prog;
int arrToggleKillThread[] = {1, 1, 1}; // used for signalling to threads to end
int currentCommand = -1;
int previousCommand = -1;

int GREEN_LED = 18; // pin number that LED is connected to
int RED_LED = 19;   // pin number that LED is connected to

int getUserCommand();
void recordPreviousCommand(int command);
void killThread(int command);
void *endProgram(void *arg);

void initialize();
void *on_leds();
void *off_leds();
void *blink_leds_2Hz();
void *blink_red_8Hz();
void *blink_green_2Hz_reduced_brightness();

int main()
{
    initialize(); // initialize LEDs
    do
    {
        recordPreviousCommand(currentCommand); // keeps track of previous command
        currentCommand = getUserCommand();     // set user's input into variable "currentCommand"

        if (previousCommand == currentCommand) // check if previous command = current command. If same, loop repeats to ask for user input again
        {
            printf("\nAlready executing this command, please pick another command >:) \n");
        }
        else
        {
            if (previousCommand == 2)
            {
                killThread(previousCommand);
            }
            if (previousCommand == 3)
            {
                killThread(previousCommand);
            }
            switch (currentCommand) // creates a thread for the LED function. At the same time, still allows user's input for next instruction
            {
            case 0:
                off_leds();
                break;
            case 1:
                on_leds();
                break;
            case 2:
                pthread_create(&blink_both, NULL, &blink_leds_2Hz, NULL);
                break;
            case 3:
                pthread_create(&red_eight, NULL, &blink_red_8Hz, NULL);
                pthread_create(&green_reduce, NULL, &blink_green_2Hz_reduced_brightness, NULL);
                break;
            case 1234:
                pthread_create(&end_prog, NULL, &endProgram, NULL);
                pthread_join(end_prog, NULL); // waits for the endProgram thread to finish before exiting the program
                printf("\n Exiting program. . . . \n");
                break;
            default: // if user's input is not one of the listed commands
                printf("\n Please try again and input 0,1,2 or 3.\n");
                break;
            }
        }

    } while (currentCommand != 1234); // if user inputs 1234, program stops asking for user's input and exits program

    return 0;
}

void recordPreviousCommand(int command) // records the previous command for thread killing purposes and optimization
{
    if (command == 0 || command == 1 || command == 2 || command == 3) // only records command if current user input is valid
    {
        previousCommand = command;
    }
}

void killThread(int command) // terminates the specified command
{
    if (command == 2)
    {
        arrToggleKillThread[0] = 0;     // sets the value in array of int to 0, acts as a toggle/signal for the LED functions to terminate it's while loop.
        pthread_join(blink_both, NULL); // waits for thread to end before continuing
    }
    else if (command == 3)
    {
        arrToggleKillThread[1] = 0;       // same as above, acts as toggle
        arrToggleKillThread[2] = 0;       // same as above, acts as toggle
        pthread_join(red_eight, NULL);    // waits for thread to end before continuing
        pthread_join(green_reduce, NULL); // waits for thread to end before continuing
    }
}

void *endProgram(void *arg) // function ensures all threads and leds are properly terminated/shut off properly
{
    printf("\n Terminating processes, please wait for the program to end :) \n");
    if (previousCommand == 2 || previousCommand == 3) // if last command is 2 or 3, ensure thread(s) are properly terminated
    {
        killThread(previousCommand);
    }
    else if (previousCommand == 1) // if leds are on, turn it off
    {
        off_leds();
    }
}

int getUserCommand() // gets user input from console
{
    int user_input;
    printf("\nSoftware commands:\n 0 to turn off both red and green LED lights \n 1 to turn on red and green LED lights \n 2 to keep blinking with the pattern of 2 times per second, for both red and green LED lights \n 3 to change the pattern of the red LED light and green LED light in different ways \n 1234 to exit the program");
    printf("\n \nPlease input software command:");
    scanf("%d", &user_input);

    return user_input;
}

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
    printf("LEDS ON");
    gpioWrite(GREEN_LED, 1);
    gpioWrite(RED_LED, 1);
}

void *off_leds()
{
    printf("LEDS OFF");
    gpioWrite(GREEN_LED, 0);
    gpioWrite(RED_LED, 0);
}

void *blink_leds_2Hz()
{
    while (arrToggleKillThread[0])
    {
        printf("BOTH LED BLINKING AT SAME RATE \n");
        on_leds();
        usleep(250000);
        off_leds();
        usleep(250000);
        if (arrToggleKillThread[0] == 0) // checks for toggle continiously in while loop, once killThread() is called, exit loop.
                                         // Ensures thread properly terminates itself with no memory leaks
        {
            arrToggleKillThread[0] = 1; // resets toggle
            break;
        }
    }
}

void *blink_red_8Hz()
{
    while (arrToggleKillThread[1])
    {
        printf("RED BLINKING 8HZ \n");
        gpioWrite(RED_LED, 1);
        usleep(62500);
        gpioWrite(RED_LED, 0);
        usleep(62500);
        if (arrToggleKillThread[1] == 0) // checks for toggle continiously in while loop, once killThread() is called, exit loop.
                                         // Ensures thread properly terminates itself with no memory leaks
        {
            arrToggleKillThread[1] = 1; // resets toggle
            break;
        }
    }
}

void *blink_green_2Hz_reduced_brightness()
{
    while (arrToggleKillThread[2])
    {
        printf(". . . . . GREEN BLINKING 2HZ \n");
        gpioPWM(GREEN_LED, 63);
        usleep(250000);
        gpioWrite(GREEN_LED, 0);
        usleep(250000);
        if (arrToggleKillThread[2] == 0) // checks for toggle continiously in while loop, once killThread() is called, exit loop.
                                         // Ensures thread properly terminates itself with no memory leaks
        {
            arrToggleKillThread[2] = 1; // resets toggle
            break;
        }
    }
}