#ifndef _GAMETWOPIPE_H__
#define _GAMETWOPIPE_H__

class gameTwoPipe{
public:
  gameTwoPipe(unsigned short _screenHeight, unsigned short _screenWidth, 
    unsigned short _pipeSpeed, unsigned short _holeSize, unsigned short _holePosY);
  void moveStep();
  void backToStart(unsigned short _newholePosY);
  void resetPipe(unsigned short _newHolePosY, unsigned short _newHoleSize, unsigned short _newSpeed);
  unsigned short screenHeight;
  unsigned short screenWidth;
  unsigned short holePosY;
  unsigned short pipeWidth;
  unsigned short holeSize;
  unsigned short minHoleSize;
  unsigned short pipeSpeed;
  unsigned short maxSpeed;
  int x;
  unsigned short previousX;
};


#endif
