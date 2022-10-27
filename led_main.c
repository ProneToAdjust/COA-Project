#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <pigpio.h>

pthread_t threads[4];
int toggleKillCommand[] = {1, 1, 1, 1}; // on/off toggle to send a signal for function to end
int currentCommand = -1;
int previousCommand = -1;

int GREEN_LED = 18; // pin number that LED is connected to
int RED_LED = 19;   // pin number that LED is connected to

int getUserCommand();
void recordPreviousCommand(int command);
void killCommand(int command);

void *offLedCaller(void *command);
void *onLedCaller(void *command);
void *blinkBothLedCaller(void *command);
void *reduceBrightnessLedCaller(void *command);

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
        recordPreviousCommand(currentCommand);                                                        // keeps track of previous command
        currentCommand = getUserCommand();                                                            // set user's input into variable "currentCommand"
        if (currentCommand == 0 || currentCommand == 1 || currentCommand == 2 || currentCommand == 3) // if current input is a valid command, kill previous command
        {
            killCommand(previousCommand);
            pthread_join(threads[previousCommand], NULL); // waits for previous command's thread to terminate before executing new command
        }

        switch (currentCommand) // creates a thread for the LED function. At the same time, still allows user's input for next instruction
        {
        case 0:
            pthread_create(&threads[currentCommand], NULL, &offLedCaller, &currentCommand);
            break;
        case 1:
            pthread_create(&threads[currentCommand], NULL, &onLedCaller, &currentCommand);
            break;
        case 2:
            pthread_create(&threads[currentCommand], NULL, &blinkBothLedCaller, &currentCommand);
            break;
        case 3:
            pthread_create(&threads[currentCommand], NULL, &reduceBrightnessLedCaller, &currentCommand);
            break;
        default: // if user's input is not in the listed commands
            printf("\n Please try again and input 0,1,2 or 3.\n");
            break;
        }

    } while (currentCommand != 1234); // if user inputs 1234, program stops asking for user's input and starts to end program

    printf("\n Exiting program. . . . \n");
    killCommand(previousCommand); // stops the last called command before ending program
    return 0;
}

void recordPreviousCommand(int command) // records the current command into previous command before taking in new user input
{
    if (command == 0 || command == 1 || command == 2 || command == 3) // only records command if current input is valid
    {
        previousCommand = command;
    }
}

void killCommand(int command) // terminates the specified command
{
    toggleKillCommand[command] = 0; // sets the value in array of int to 0, acts as a toggle/signal for the LED functions to terminate it's while loop.
                                    // function continulously checks for toggle on every loop, once toggle is 0, function stops looping
}

int getUserCommand() // gets user input from console
{
    int user_input;
    printf("\nSoftware commands:\n 0 to turn off both red and green LED lights \n 1 to turn on red and green LED lights \n 2 to keep blinking with the pattern of 2 times per second, for both red and green LED lights \n 3 to change the pattern of the red LED light and green LED light in different ways");
    printf("\n \nPlease input software command:");
    scanf("%d", &user_input);

    return user_input;
}

void *offLedCaller(void *command)
{
    int index = *(int *)command; // cast command to int pointer and dereference
    while (toggleKillCommand[index])
    {
        off_leds();
        if (toggleKillCommand[index] == 0) // checks for toggle continiously in while loop, once killCommand() is called, exit loop
        {
            toggleKillCommand[index] = 1; // resets toggle
            break;
        }
    }
}

void *onLedCaller(void *command)
{
    int index = *(int *)command; // cast command to int pointer and dereference
    while (toggleKillCommand[index])
    {
        on_leds();
        if (toggleKillCommand[index] == 0) // checks for toggle continiously in while loop, once killCommand() is called, exit loop
        {
            toggleKillCommand[index] = 1; // resets toggle
            break;
        }
    }
}

void *blinkBothLedCaller(void *command)
{
    int index = *(int *)command; // cast command to int pointer and dereference
    while (toggleKillCommand[index])
    {
        blink_leds_2Hz();
        if (toggleKillCommand[index] == 0) // checks for toggle continiously in while loop, once killCommand() is called, exit loop
        {
            toggleKillCommand[index] = 1; // resets toggle
            break;
        }
    }
}

void *reduceBrightnessLedCaller(void *command)
{
    int index = *(int *)command; // cast command to int pointer and dereference
    while (toggleKillCommand[index])
    {
        blink_red_8Hz();
        blink_green_2Hz_reduced_brightness();
        if (toggleKillCommand[index] == 0) // checks for toggle continiously in while loop, once killCommand() is called, exit loop
        {
            toggleKillCommand[index] = 1; // resets toggle
            break;
        }
    }
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
    on_leds();
    usleep(250000);
    off_leds();
    usleep(250000);
}

void *blink_red_8Hz()
{
    gpioWrite(RED_LED, 1);
    usleep(62500);
    gpioWrite(RED_LED, 0);
    usleep(62500);
}

void *blink_green_2Hz_reduced_brightness()
{
    gpioPWM(GREEN_LED, 63);
    usleep(250000);
    gpioWrite(GREEN_LED, 0);
    usleep(250000);
}