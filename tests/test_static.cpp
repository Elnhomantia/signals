#include <event_signal.h>
#include <cassert>

void staticF(bool& wasCalled)
{
    wasCalled = true;
}

int main()
{
    bool wasCalled = false;
    Signal<bool&> staticTest;

    Connection staticC = staticTest.connect(&staticF);

    staticTest.emit(wasCalled);

    assert(wasCalled);
    return EXIT_SUCCESS;
}
