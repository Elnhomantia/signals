#include <iostream>
#include "signal.h"

using namespace std;

void testFree(int i)
{
    cout << "Free function test : " << i << endl;
}
void testBindFree(string f, int i)
{
    cout << "Free function binding test : " << f << " " << i << endl;
}

class Foo
{
public:
    void testClass(int i)
    {
        cout << "Class function test : " << i << endl;
    }
    void testBindClass(string f, int i)
    {
        cout << "Class function binding test : " << f << " " << i << endl;
    }
};

int main()
{
    Signal<int> s1;

    Connection<int> c0 = s1.connect(&testFree);
    Connection<int> c1 = s1.connect([=](int i){ cout << "Lambda function test : " << i << endl; });

    Connection<int> c2 = s1.connect(&testBindFree, "3.2f");
    Connection<int> c3 = s1.connect([=](string f, int i){ cout << "Lambda binding test : " << f << " " << i << endl; }, "3.2f");

    Foo f;
    Connection<int> c4 = s1.connect(&f, &Foo::testClass);
    Connection<int> c5 = s1.connect(&f, &Foo::testBindClass, "3.2f");

    shared_ptr<Foo> sf = make_shared<Foo>();
    Connection<int> c6 = s1.connect(sf, &Foo::testClass);
    Connection<int> c7 = s1.connect(sf, &Foo::testBindClass, "3.2f");

    s1.emit(1);
    //s1.emit(2);
    return 0;
}
