#include <event_signal.h>
#include <cassert>
#include <memory>

class Foo
{
    public_signal(sPub, int)

public:
    bool wasCalled = false;

    void emitSig()
    {
        sPub.emit(0);
    }

    void member(int i)
    {
        this->wasCalled = true;
    }

};

int main()
{
    Foo f;
    std::shared_ptr<Foo> sf = std::make_shared<Foo>();

    Connection c1 = f.connect_sPub(&f, &Foo::member);
    Connection c2 = sf->connect_sPub(sf, &Foo::member);

    f.emitSig();
    sf->emitSig();

    assert(f.wasCalled);
    assert(sf->wasCalled);
    return EXIT_SUCCESS;
}
