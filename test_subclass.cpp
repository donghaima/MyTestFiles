#include <string>
#include <iostream>

using namespace std;

class base
{
  int m_ts;

public:
  void setTS(int ts) { m_ts = ts; }
  int getTS(void) { return m_ts; }
};


class subc : public base {
  int myTS;

public:  
  subc(int ts)
  {
    myTS = ts;
    setTS(ts * 2);
  }
};


class pub {

public:
  void myPrint(base& B)
  {
    cout << "ts = " << B.getTS() << endl;
  }


};

int main(void)
{
  base B;
  subc *C = new subc(-100);

  B.setTS(1200);

  pub P;
  P.myPrint(B);
  P.myPrint(*C);

  return 0;
}
