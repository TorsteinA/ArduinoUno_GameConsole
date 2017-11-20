#ifndef _GAMEONESTONE_H__
#define _GAMEONESTONE_H__

class gameOneStone{
public:
    gameOneStone(int _x, int _speed, int _size, int _max);
    ~gameOneStone();
    void moveStep();
    void backToTop(int _newPos);
    void resetStone(int _newPos, int _speed);
    int x;
    int y;
    int previousY;
    int speed;
    int size;
    int maxFallLength;
    int maxSpeed;
};


#endif