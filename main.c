#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include "rs232.h"
#include "serial.h"

#include "game.h"


#define bdrate 115200               /* 115200 baud */

void SendCommands (char *buffer );

int main()
{

    char buffer[100];
    // If we cannot open the port then give up immediatly
    if ( CanRS232PortBeOpened() == -1 )
    {
        printf ("\nUnable to open the COM port (specified in serial.h) ");
        exit (0);
    }
    // Time to wake up the robot
    printf ("\nAbout to wake up the robot\n");
    // We do this by sending a new-line
    sprintf (buffer, "\n");
    // printf ("Buffer to send: %s", buffer); // For diagnostic purposes only, normally comment out
    PrintBuffer (&buffer[0]);
    #ifdef MAC
    sleep(0.1);
    #else
    Sleep(100);
    #endif

    FILE *file = fopen("ShapeStrokeData.txt", "r");
    if (file == NULL) {
        printf("Error opening file");
        return 1;
    }
    Shape *shapeList = NULL;
    int shapeListSize = 0;
    shapeListSize = ReadShape(&shapeList, file);
    fclose(file);

    int user1 = 0, user2  = 0;
    shapeSelection(shapeListSize, &user1, &user2);

    float gridSize;
    gridSize = drawTheGrid();
    printf("Grid size: %f\n", gridSize);
    gameLoop(gridSize, shapeList[user1-1],  shapeList[user2-1]);

    // Before we exit the program we need to close the COM port
    CloseRS232Port();
    printf("Com port now closed\n");
    freeShape(shapeList, shapeListSize);
    return (0);
}

// Send the data to the robot - note in 'PC' mode you need to hit space twice
// as the dummy 'WaitForReply' has a getch() within the function.
void SendCommands (char *buffer )
{
    // printf ("Buffer to send: %s", buffer); // For diagnostic purposes only, normally comment out
    PrintBuffer (&buffer[0]);
    WaitForReply();
    Sleep(100); // Can omit this when using the writing robot but has minimal effect
}


//TODO:Build a function to read the data from the text file and store it into correct form.


//TODO:Write a function to allow the user to set the shape and the size of the grid


// int ReadSingleShape(Buffer)
// {

// }