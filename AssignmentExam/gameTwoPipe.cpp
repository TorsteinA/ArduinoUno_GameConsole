#include "gameTwoPipe.h"

gameTwoPipe::gameTwoPipe(unsigned short _screenHeight, unsigned short _screenWidth, 
    unsigned short _pipeSpeed, unsigned short _holeSize, unsigned short _holePosY){
    screenHeight = _screenHeight;
    screenWidth = _screenWidth;
    pipeSpeed = _pipeSpeed;
    holeSize = _holeSize;
    holePosY = _holePosY;
    minHoleSize = 20;
    maxSpeed = 8;
    x = screenWidth;
    previousX = x;
    pipeWidth = 10;
}

void gameTwoPipe::moveStep(){
    previousX = x;
    x -= pipeSpeed;
}

void gameTwoPipe::backToStart(unsigned short _newHolePosY){
    x = screenWidth;
    previousX = x;
    if (++pipeSpeed >= maxSpeed) pipeSpeed = maxSpeed;
    if (pipeSpeed == maxSpeed) {
        if (--holeSize < minHoleSize) holeSize = minHoleSize;
    }
    holePosY = _newHolePosY;
}

void gameTwoPipe::resetPipe(unsigned short _newHolePosY, unsigned short _newHoleSize, unsigned short _newPipeSpeed){
    holePosY = _newHolePosY;
    holeSize = _newHoleSize;
    pipeSpeed = _newPipeSpeed;
    x = screenWidth;
}

