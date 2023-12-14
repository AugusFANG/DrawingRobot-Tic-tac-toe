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

    FILE *file = fopen("ShapeStrokeData.txt", "r");
    if (file == NULL) {
        printf("Error opening file");
        return 1;
    }
    Shape *shapeList = NULL;
    int shapeListSize = 0;
    shapeListSize = readShape(&shapeList, file);
    fclose(file);

    int user1 = 0, user2  = 0;
    shapeSelection(shapeListSize, &user1, &user2);



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
    Sleep(100);

    // This is a special case - we wait  until we see a dollar ($)
    WaitForDollar();

    printf ("\nThe robot is now ready to draw\n");



    // Draw the grid and start the game.
    float gridSize;
    gridSize = drawTheGrid();
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
