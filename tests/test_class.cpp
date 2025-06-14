#include "signal.h"
#include <cassert>

class Foo
{
public:
    bool wasCalled = false;

    void member()
    {
        this->wasCalled = true;
    }

};

int main()
{
    Signal<> s;
    Foo f;

    Connection c = s.connect(&f, &Foo::member);

    s.emit();

    assert(f.wasCalled);
    return EXIT_SUCCESS;
}
