#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

using boost::shared_ptr;
struct B;
struct A{
    ~A() { std::cout << "~A" << std::endl; }
    shared_ptr<B> b;    
};
struct B {
    ~B() { std::cout << "~B" << std::endl; }
  //shared_ptr<A> a;   // looping: need to change to use weak_ptr; otherwise pointers won't be cleaned up
  weak_ptr<A> a;
};

int main() {
    shared_ptr<A> a (new A);
    shared_ptr<B> b (new B);
    a->b = b;
    b->a = a;

    return 0;
}
