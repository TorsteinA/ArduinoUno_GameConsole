#include "gameOneStone.h"

gameOneStone::gameOneStone(int _x, int _speed, int _size, int _maxFallLength){
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

void gameOneStone::backToTop(int _newPos){
    y = 10;
    if (++speed >= maxSpeed) speed = maxSpeed;
    if (speed == maxSpeed && ++size >= maxSize) size = maxSize;
    x = _newPos;
}

void gameOneStone::resetStone(int _newPos, int _speed, int _size){
    y = 10;
    speed = _speed;
    size = _size;
    x = _newPos;
}

