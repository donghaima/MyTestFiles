#include <iostream>
#include <tr1/functional>

int myCallbackCalc(void *arg)
{
  return 20;
}

class GameCharacter
{
public:
  typedef std::tr1::function<int (void *)> CallbackFunc;

  GameCharacter(CallbackFunc cbf, void *arg) :
    m_cbFunc(cbf),
    m_argPtr(arg)
  { }

  int healthValue() const { return m_cbFunc(m_argPtr); }

private:
  CallbackFunc m_cbFunc;
  void *m_argPtr;
};


int main(void)
{
  int i=10;
  void *p = &i;
  GameCharacter *myGC = new GameCharacter(myCallbackCalc, p);
  std::cout << "myGC healthValue= " << myGC->healthValue() << std::endl;

  return 0;
}
