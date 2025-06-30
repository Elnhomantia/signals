#include <event_signal.h>
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
    Foo f;

    Connection c = s.connect(&f, &Foo::member, 3);

    s.emit();

    assert(f.wasCalled);
    assert(f.bind == 3);
    return EXIT_SUCCESS;
}
