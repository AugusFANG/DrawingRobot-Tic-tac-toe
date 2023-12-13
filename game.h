#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<stdbool.h>

typedef struct PenStroke{
    int x;
    int y;
    int penStatus;
}PenStroke;

typedef struct Shape{
    char name[50];
    int numStrokes;
    struct PenStroke *strokes;
}Shape;

void safeFlush(FILE *fp);
int ReadShape (Shape **shapeList, FILE *file);
void freeShape(Shape *shapeList, int size);
void shapeSelection(int size, int *user1, int *user2);
void SendCommands (char *buffer);
float drawTheGrid();
void moveSelection(int* x, int *y, int userID);
void drawWinner(float x1, float y1, float x2, float y2, float gridSize);
int checkWinner(float gridSize);
bool gameOver();
void gameLoop(float gridSize, Shape shape1, Shape shape2);
void drawShape(Shape shape, int x, int y, float scale, float gridSize);
void runSingleStep(float x, float y, int penStatus, int lastStatus);
void resetRobot();
float reScale(float gridSize);

#endif