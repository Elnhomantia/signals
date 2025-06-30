#include <event_signal.h>
#include <cassert>

int main()
{
    bool wasCalled = false;
    Signal<bool&> s;

    Connection c = s.connect([](bool& wasCalled){ wasCalled = true; });

    s.emit(wasCalled);

    assert(wasCalled);
    return EXIT_SUCCESS;
}
