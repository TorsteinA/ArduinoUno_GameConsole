#include "gameOneStone.h"

gameOneStone::gameOneStone(unsigned short _x, unsigned short _speed, unsigned short _size, unsigned short _maxFallLength){
    x = _x;
    y = 10;
    speed = _speed;
    size = _size;
    previousY = y;
    maxFallLength = _maxFallLength;
    maxSpeed = 15;
    maxSize = 20;
}

void gameOneStone::moveStep(){
    previousY = y;
    y += speed;
}

void gameOneStone::backToTop(unsigned short _newPos){
    y = 10;
    if (++speed >= maxSpeed) speed = maxSpeed;
    if (speed == maxSpeed && ++size >= maxSize) size = maxSize;
    x = _newPos;
}

void gameOneStone::resetStone(unsigned short _newPos, unsigned short _speed, unsigned short _size){
    y = 10;
    speed = _speed;
    size = _size;
    x = _newPos;
}

