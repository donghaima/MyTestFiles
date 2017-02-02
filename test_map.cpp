#include <iostream>
#include <map>
using namespace std;

class AAA
{
   friend ostream &operator<<(ostream &, const AAA &);

   public:
      int x;
      int y;
      float z;

      AAA();
      AAA(const AAA &);
      ~AAA(){};
      AAA &operator=(const AAA &rhs);
      int operator==(const AAA &rhs) const;
      int operator<(const AAA &rhs) const;
};

AAA::AAA()   // Constructor
{
   x = 0;
   y = 0;
   z = 0;
}

AAA::AAA(const AAA &copyin)   // Copy constructor to handle pass by value.
{                             
   x = copyin.x;
   y = copyin.y;
   z = copyin.z;
}

ostream &operator<<(ostream &output, const AAA &aaa)
{
   output << aaa.x << ' ' << aaa.y << ' ' << aaa.z << endl;
   return output;
}

AAA& AAA::operator=(const AAA &rhs)
{
   this->x = rhs.x;
   this->y = rhs.y;
   this->z = rhs.z;
   return *this;
}

int AAA::operator==(const AAA &rhs) const
{
   if( this->x != rhs.x) return 0;
   if( this->y != rhs.y) return 0;
   if( this->z != rhs.z) return 0;
   return 1;
}

int AAA::operator<(const AAA &rhs) const
{
   if( this->x == rhs.x && this->y == rhs.y && this->z < rhs.z) return 1;
   if( this->x == rhs.x && this->y < rhs.y) return 1;
   if( this->x < rhs.x ) return 1;
   return 0;
}

main()
{
   map<string, AAA> M;
   AAA Ablob ;

   Ablob.x=7;
   Ablob.y=2;
   Ablob.z=4.2355;
   M["A"] = Ablob;

   Ablob.x=5;
   M["B"] = Ablob;

   Ablob.z=3.2355;
   M["C"] = Ablob;

   Ablob.x=3;
   Ablob.y=7;
   Ablob.z=7.2355;
   M["D"] = Ablob;

   for( map<string, AAA>::iterator ii=M.begin(); ii!=M.end(); ++ii)
   {
       cout << (*ii).first << ": " << (*ii).second << endl;
   }

   return 0;
}

