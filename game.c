#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<stdbool.h>
#include "game.h"

int chessBoard[3][3]={{0,0,0},{0,0,0},{0,0,0}};
#define GAP 2
#define SHAPE_UNITS 16



void safeFlush(FILE *fp)
{
    int ch;
    while( (ch = fgetc(fp)) != EOF && ch != '\n' );
}

/**
 * @brief Read a shape from a file
 * @param file The file to read
 * @param shapes the pointer to the array of shapes
 * @return the number of shapes
*/
int ReadShape (Shape **shapes, FILE *file)
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

    do{
        printf("[msg] Please select a shape ID for USER1 from 1 to %d: ", size);
        scanfResult = scanf("%d", user1);
        safeFlush(stdin);

        if (scanfResult == 0) {
            while ((getchar()) != '\n');
            printf("[Alert] Invalid shape ID.\n");
        }else if (*user1 < 1 || *user1 > size){
            printf("[Alert] Invalid shape ID.\n");
        }else break;
    }while(1);
    do{
        printf("[msg] Please select a shape ID for USER2 from 1 to %d: ", size);
        scanfResult = scanf("%d", user2);
        safeFlush(stdin);

        if (scanfResult == 0) {
            while ((getchar()) != '\n');
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

float drawTheGrid()
{
    char buffer[100];
    float gridSize;
    int scanfResult = 0;
    printf("[msg] Please Select a Grid Size (30-100)(mm):");

    do{
        scanfResult = scanf("%f",&gridSize);
        safeFlush(stdin);

        if (scanfResult == 0) {
            while ((getchar()) != '\n');
        }else if (gridSize < 30 || gridSize > 100){
            //do nothing
        }else break;
        printf("[Alert] Invalid Grid Size. \n");
        printf("[msg] Please Reselect the Grid Size(30-100)(mm):");
    }while(1);


    // Initialise the Robot
    sprintf (buffer, "G1 X0 Y0 F1000\n");
    SendCommands(buffer);
    sprintf (buffer, "M3\n");
    SendCommands(buffer);
    sprintf (buffer, "S0\n");
    SendCommands(buffer);

    double grid[8][2] = {{0, -1/3.0}, {1, -1/3.0},{0, -2/3.0}, {1, -2/3.0}, {1/3.0, 0}, {1/3.0, -1}, {2/3.0, 0}, {2/3.0, -1}};
    int gridPensStatus[8] = {0, 1, 0, 1, 0, 1, 0, 1};

    // Draw the Grid
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

void resetRobot(){
    char buffer[100];
    sprintf (buffer, "S0\n");
    SendCommands(buffer);
    sprintf (buffer, "G0 X0 Y0\n");
    SendCommands(buffer);
}

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

float reScale(float gridSize){
    float scale;
    scale=((gridSize/3)-2*GAP)/SHAPE_UNITS;
    return scale;
}

void getStartPosition(int x, int y, float gridSize, float *startX, float *startY){
    *startX = 2.0f + (float)(x-1) * gridSize/3.0f;
    *startY = 2.0f - (float)(4-y) * gridSize/3.0f;
}

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

void moveSelection(int* x, int *y, int userID){
    int scanfResult = -1;
    printf("[msg] Player %d, please choose your next move: ", userID);
    do{
        scanfResult = scanf("%d %d", x, y);
        safeFlush(stdin);

        if (scanfResult == 0) {
            while ((getchar()) != '\n');
            printf("[Alert] Invalid move.\n");
            printf("[msg] Player %d, please try again: ", userID);
        }else if (*x < 1 || *x > 3 || *y < 1 || *y > 3 || chessBoard[*x - 1 ][*y - 1 ] != 0){
            printf("[Alert] This postion is either occupied or not valid.\n");
            printf("[msg] Player %d, please try again: ", userID);
        }else break;
    }while(1);

    chessBoard[*x - 1 ][*y - 1 ] = userID;
}

void drawWinner(float x1, float y1, float x2, float y2, float gridSize){
    runSingleStep(x1 * gridSize, y1 * gridSize, 0, 0);
    runSingleStep(x2 * gridSize, y2 * gridSize, 1, 0);
    resetRobot();
}

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

void gameLoop(float gridSize, Shape shape1, Shape shape2){
    int roundNumber = 1;
    int x1, y1, x2, y2;
    float scale = reScale(gridSize);

    int winner = 0;
    while(1){
        printf("[Info] Round Number: %d\n", roundNumber);
        moveSelection(&x1, &y1, 1);
        drawShape(shape1, x1, y1, scale, gridSize);

        winner = checkWinner(gridSize);
        if(winner != 0){
            printf("[Congratulations] Player %d wins!\n", winner);
            break;
        }

        if(gameOver()){
            printf("[Note] It's a draw, the game is over.\n");
            break;
        }

        moveSelection(&x2, &y2, 2);
        drawShape(shape2, x2, y2, scale, gridSize);

        winner = checkWinner(gridSize);
        if(winner != 0){
            printf("[Congratulations] Player %d wins!\n", winner);
            break;
        }

        if(gameOver()){
            printf("[Note] It's a draw, the game is over.\n");
            break;
        }

        roundNumber ++;
    }
}

