#include <iostream>
#include "signal.h"

using namespace std;

void testFree(int i)
{
    cout << "Free function test : " << i << endl;
}
void testBindFree(float f, int i)
{
    cout << "Free function binding test : " << f << " " << i << endl;
}

int main()
{
    Signal<int> s1;
    Connection<int> c0 = s1.connect(testFree);
    Connection<int> c1 = s1.connect([=](int i){ cout << "Lambda function test : " << i << endl; });

    Connection<int> c2 = s1.connectBind(testBindFree, 3.0f);
    Connection<int> c3 = s1.connectBind([=](float f, int i){ cout << "Lambda binding test : " << f << " " << i << endl; }, 3.0);

    s1.emit(1);
    //s1.emit(2);
    return 0;
}
