#include "signal.h"
#include <cassert>

class Foo
{
public:
    bool wasCalled = false;
    int bind = 0;

    void member(int bind)
    {
        this->wasCalled = true;
        this->bind = bind;
    }

};

int main()
{
    Signal<> s;
    std::shared_ptr<Foo> sf = std::make_shared<Foo>();

    Connection c = s.connect(sf, &Foo::member, 3);

    s.emit();

    assert(sf->wasCalled);
    assert(sf->bind == 3);
    return EXIT_SUCCESS;
}
