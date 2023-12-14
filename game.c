#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<stdbool.h>
#include "game.h"

// The array that holds the chess board, it is a 3x3 array of integers
int chessBoard[3][3]={{0,0,0},{0,0,0},{0,0,0}};

// Define the gap between the shapes and the grid
#define GAP 2

//  Define the original unit size of the shapes
#define SHAPE_UNITS 16


/**
 * @brief Clears the scanf fields
 * @param fp: pointer of the file to be cleared
*/
void safeFlush()
{
    int ch;
    while((ch = getchar()) != '\n' && ch != EOF);
}

/**
 * @brief Read a shape from a file
 * @param file The file to read
 * @param shapes the pointer to the array of shapes
 * @return the number of shapes
*/
int readShape (Shape **shapes, FILE *file)
{
    int listSize;
    fscanf(file, "NumShapes %d\n", &listSize);

    Shape *shapeList = malloc((size_t)listSize * sizeof(Shape));
    char dataToBeRead[50];

    int  i = -1, j = 0;
    // Read the shape data into the shape list line by line
    while (fgets(dataToBeRead, 50, file) != NULL) {
        if (dataToBeRead[0] >= 'A' && dataToBeRead[0] <= 'Z') {
            // If the first character is an uppercase letter, define a new shape
            i = i + 1;
            j = 0;
            sscanf(dataToBeRead, "%s %d", shapeList[i].name, &shapeList[i].numStrokes);
            shapeList[i].strokes = malloc((size_t)shapeList[i].numStrokes * sizeof(PenStroke));

            // printf("New shape added: %s %d\n", shapeList[i].name, shapeList[i].numStrokes);

        } else if(dataToBeRead[0] >= '0' && dataToBeRead[0] <= '9') {
            // If the first character is a number, add a new stroke to the current shape
            sscanf(dataToBeRead, "%d %d %d", &shapeList[i].strokes[j].x, &shapeList[i].strokes[j].y, &shapeList[i].strokes[j].penStatus);
            // printf("-- new stroke added: %d %d %d\n", shapeList[i].strokes[j].x, shapeList[i].strokes[j].y, shapeList[i].strokes[j].penStatus);
            j += 1;
        }else{
            break;
        }
    }

    *shapes = shapeList;
    return listSize;
}

/**
 * @brief Free the memory allocated for the shapes
 * @param shapeList list of shapes
 * @param listSize number of shapes
*/
void freeShape(Shape *shapeList, int size){
    for(int i = 0; i < size; ++i)
    {
        free(shapeList[i].strokes);
    }
    free(shapeList);
}

/**
 * @brief Let the players select the shape they want to play
 * @param size the number of shapes in the list
 * @param user1 the shape ID selected by the first player
 * @param user2 the shape ID selected by the second player
*/
void shapeSelection(int size, int *user1, int *user2){
    int scanfResult;
    // allow user 1 to select a shape.
    do{
        printf("[msg] Please select a shape ID for USER1 from 1 to %d: ", size);
        scanfResult = scanf("%d", user1);
        safeFlush();

        if (scanfResult == 0) {
            printf("[Alert] Invalid shape ID.\n");
        }else if (*user1 < 1 || *user1 > size){
            printf("[Alert] Invalid shape ID.\n");
        }else break;
    }while(1);
    do{
        printf("[msg] Please select a shape ID for USER2 from 1 to %d: ", size);
        scanfResult = scanf("%d", user2);
        safeFlush();

        if (scanfResult == 0) {
            printf("[Alert] Invalid shape ID.\n");
        }else if (*user2 < 1 || *user2 > size){
            printf("[Alert] Invalid shape ID.\n");
        }else if (*user1 == *user2) {
            printf("[Alert] You cannot select the same shape.\n");
        }
        else break;
    }while(1);

    printf("\n\n[Note] You selected shape %d for USER1 and shape %d for USER2 \n\n", *user1, *user2);
}

/**
 * @brief Draw the grid on the paper before the game starts
*/
float drawTheGrid()
{
    char buffer[100];
    float gridSize;

    // Get the size of the grid from user input
    printf("[msg] Please select a grid size (30-100)(mm):");

    do{
        scanf("%f",&gridSize);
        safeFlush();

        if (gridSize >= 30 && gridSize <= 100){
            break;
        }
        printf("[Alert] Invalid Grid Size. \n");
        printf("[msg] Please reselect the grid size(30-100)(mm):");
    }while(1);


    // Initialise the robot before everything else
    sprintf (buffer, "G1 X0 Y0 F1000\n");
    SendCommands(buffer);
    sprintf (buffer, "M3\n");
    SendCommands(buffer);
    sprintf (buffer, "S0\n");
    SendCommands(buffer);

    double grid[8][2] = {{0, -1/3.0}, {1, -1/3.0},{0, -2/3.0}, {1, -2/3.0}, {1/3.0, 0}, {1/3.0, -1}, {2/3.0, 0}, {2/3.0, -1}};
    int gridPensStatus[8] = {0, 1, 0, 1, 0, 1, 0, 1};

    // Draw the chess board grid
    for (int i = 0; i < 8; i++){
        sprintf (buffer, "G%d X%.2f Y%.2f\n", gridPensStatus[i], gridSize * grid[i][0], gridSize * grid[i][1]);
        SendCommands(buffer);
        if(i % 2 == 0){
            sprintf (buffer, "S1000\n");
            SendCommands(buffer);
        }else{
            sprintf (buffer, "S0\n");
            SendCommands(buffer);
        }
    }

    sprintf (buffer, "G0 X0 Y0\n");
    SendCommands(buffer);
    return gridSize;
}

/**
 * @brief Reset the robot position to the initial position
*/
void resetRobot(){
    char buffer[100];
    sprintf (buffer, "S0\n");
    SendCommands(buffer);
    sprintf (buffer, "G0 X0 Y0\n");
    SendCommands(buffer);
}

/**
 * @brief Run each single movement of the robot.
 * @param x the horizontal position of current movement
 * @param y the vertical position of current movement
 * @param penStatus the current pen status of the robot
 * @param lastPenStatus the last pen status of the robot
*/
void runSingleStep(float x, float y, int penStatus, int lastStatus){
    char buffer[100];
    if(penStatus != lastStatus){
        if(penStatus == 0){
            sprintf (buffer, "S0\n");
            SendCommands(buffer);
        }else{
            sprintf (buffer, "S1000\n");
            SendCommands(buffer);
        }
    }
    sprintf (buffer, "G%d X%.2f Y%.2f\n", penStatus, x, y);
    SendCommands(buffer);
}

/**
 * @brief Rescale the shapes according to the grid size
 * @param gridSize the size of the grid
 * @return the new scale of the shapes
*/
float reScale(float gridSize){
    float scale;
    scale=((gridSize/3)-2*GAP)/SHAPE_UNITS;
    return scale;
}

/**
 * @brief Get the starting position of each shape due to the grid size
 * @param x the position of the current move in the x-direction
 * @param y the position of the current move in the y-direction
 * @param startX the starting position of the current shape in the x-direction
 * @param startY the starting position of the current shape in the y-direction
*/
void getStartPosition(int x, int y, float gridSize, float *startX, float *startY){
    *startX = 2.0f + (float)(x-1) * gridSize/3.0f;
    *startY = 2.0f - (float)(4-y) * gridSize/3.0f;
}

/**
 * @brief Draw a shape on the paper
 * @param shape the shape to be drawn
 * @param x the position of the current move in the x-direction
 * @param y the position of the current move in the y-direction
 * @param scale the scale of the shape
 * @param gridSize the size of the grid
*/
void drawShape(Shape shape, int x, int y, float scale, float gridSize){
    int lastStatues = 0;
    float startX = 0.0f, startY = 0.0f;
    getStartPosition(x, y, gridSize, &startX, &startY);
    runSingleStep(startX, startY, 0, 0);

    for(int i = 0; i < shape.numStrokes; i++){
        runSingleStep((float)shape.strokes[i].x * scale + startX, (float)shape.strokes[i].y * scale + startY, shape.strokes[i].penStatus, lastStatues);
        lastStatues = shape.strokes[i].penStatus;
    }
    resetRobot();
    printf("[info] Current step has been completed\n");
}

/**
 * @brief Let the players to select the move they want to make
 * @param x the position of the selected move in the x-direction
 * @param y the position of the selected move in the y-direction
 * @param userID represents the ID of the current player
*/
void moveSelection(int* x, int *y, int userID){
    int scanfResult;
    printf("[msg] Player %d, please choose your next move: ", userID);
    do{
        scanfResult = scanf("%d %d", x, y);
        safeFlush();

        if (scanfResult == 0) {
            printf("[Alert] Invalid move.\n");
            printf("[msg] Player %d, please try again: ", userID);
        }else if (*x < 1 || *x > 3 || *y < 1 || *y > 3 || chessBoard[*x - 1 ][*y - 1 ] != 0){
            printf("[Alert] This postion is either occupied or not valid.\n");
            printf("[msg] Player %d, please try again: ", userID);
        }else break;
    }while(1);

    chessBoard[*x - 1 ][*y - 1 ] = userID;
}

/**
 * @brief draw the line for winner
 * @param x1 starting x of the line
 * @param y1 starting y of the line
 * @param x2 ending x of the line
 * @param y2 ending y of the line
 * @param gridSize the size of the grid
*/
void drawWinner(float x1, float y1, float x2, float y2, float gridSize){
    runSingleStep(x1 * gridSize, y1 * gridSize, 0, 0);
    runSingleStep(x2 * gridSize, y2 * gridSize, 1, 0);
    resetRobot();
}

/**
 * @brief Check if any player has won the game
 * @param gridSize the size of the grid
 * @return 1 if player 1 has won, 2 if player 2 has won, 0 if neither player
*/
int checkWinner(float gridSize){
    float lineParams[3] = {1.0f/6.0f, 0.5f, 5.0f/6.0f};
    //check if there is a row or a column having the same shapes.
    for(int i = 0; i < 3; ++i){
        if(chessBoard[i][0] == chessBoard[i][1] && chessBoard[i][1] == chessBoard[i][2] && chessBoard[i][0]!= 0){
            drawWinner(lineParams[i], 0.0, lineParams[i], -1.0, gridSize);
            return chessBoard[i][0];
        }
        if(chessBoard[0][i] == chessBoard[1][i] && chessBoard[1][i] == chessBoard[2][i] && chessBoard[0][i]!= 0){
            drawWinner(0.0, -lineParams[2-i], 1.0, -lineParams[2-i], gridSize);
            return chessBoard[0][i];
        }
    }

    //check if there is a diagonal having the same shapes.
    if(chessBoard[0][0] == chessBoard[1][1] && chessBoard[1][1] == chessBoard[2][2] && chessBoard[0][0]!= 0){
        drawWinner(0.0, -1.0, 1.0, 0.0, gridSize);
        return chessBoard[0][0];
    }else if(chessBoard[0][2] == chessBoard[1][1] && chessBoard[1][1] == chessBoard[2][0] && chessBoard[0][2]!= 0){
        drawWinner(0.0, 0.0, 1.0, -1.0, gridSize);
        return chessBoard[0][2];
    }else return 0;
}

/**
 * @brief Check if the game is over
 * @return true if the game is over, false otherwise
*/
bool gameOver(){
    for(int i = 0; i < 3; ++i){
        for(int j = 0; j < 3; ++j){
            if(chessBoard[i][j] == 0){
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief The main function of the game
 * @param gridSize the size of the grid
 * @param shape1 shape selected by the player 1
 * @param shape2 shape selected by the player 2
*/
void gameLoop(float gridSize, Shape shape1, Shape shape2){
    int roundNumber = 1;
    int x1, y1, x2, y2;
    float scale = reScale(gridSize);

    int winner = 0;
    while(1){
        // Inform the round information
        printf("[Info] Round Number: %d\n", roundNumber);

        // Player 1's turn
        moveSelection(&x1, &y1, 1);
        drawShape(shape1, x1, y1, scale, gridSize);

        // Check for winner
        winner = checkWinner(gridSize);
        if(winner != 0){
            printf("[Congratulations] Player %d wins!\n", winner);
            break;
        }
        // Check if the board is full, if so, the game is over.
        if(gameOver()){
            printf("[Note] It's a draw, the game is over.\n");
            break;
        }

        // Player 2's turn
        moveSelection(&x2, &y2, 2);
        drawShape(shape2, x2, y2, scale, gridSize);

        // Check for winner
        winner = checkWinner(gridSize);
        if(winner != 0){
            printf("[Congratulations] Player %d wins!\n", winner);
            break;
        }
        // Check if the board is full, if so, the game is over.
        if(gameOver()){
            printf("[Note] It's a draw, the game is over.\n");
            break;
        }

        roundNumber ++;
    }
}

