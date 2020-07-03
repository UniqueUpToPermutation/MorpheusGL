#include <iostream>


class A { };
class B { };
class C { public: int a; };

template <>
struct PARENT_TYPE_<A> {
	typedef B RESULT;
};
template <> 
struct PARENT_TYPE_<B> {
	typedef C RESULT;
};

int main() {

}