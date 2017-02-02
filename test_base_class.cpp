#include <iostream>

class Base {
public:
  Base(int i): a(i) {}
  virtual ~Base() {}
  int get_a() {return a;}
  
protected:
  int a;
};

class Child: public Base {

public:
  Child(int i): Base(i*2), b(i) {}
  void foo();

  int get_b()
  {
    return b;
  }
private:
  int b;
};

void Child::foo() {
  b = Base::a; // Access variable 'a' from parent
  std::cout << "Child::foo(): a=" << Base::a << ", b=" << b << std::endl;
}




int main()
{
  Child* Cld = new Child(10);
  Base* Bse = new Base(1);
  
  std::cout << "Child->b=" << Cld->get_b() << ", Base->a=" << Bse->get_a() << ", Chd->a="
            << Cld->get_a() << std::endl;

  Cld->foo();
  
  return 0;
}
