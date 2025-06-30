#include <event_signal.h>
#include <cassert>
#include <memory>

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
    std::shared_ptr<Foo> sf = std::make_shared<Foo>();

    Connection c = s.connect(sf, &Foo::member);

    s.emit();

    assert(sf->wasCalled);
    return EXIT_SUCCESS;
}
