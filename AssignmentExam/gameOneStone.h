#ifndef _GAMEONESTONE_H__
#define _GAMEONESTONE_H__

class gameOneStone{
public:
  gameOneStone(unsigned short _x, unsigned short _speed, unsigned short _size, unsigned short _maxFallLength);
  ~gameOneStone();
  void moveStep();
  void backToTop(unsigned short _newPos);
  void resetStone(unsigned short _newPos, unsigned short _speed, unsigned short _size);
  unsigned short x;
  unsigned short y;
  unsigned short previousY;
  unsigned short speed;
  unsigned short size;
  unsigned short maxFallLength;
  unsigned short maxSpeed;
  unsigned short maxSize;
};


#endif
