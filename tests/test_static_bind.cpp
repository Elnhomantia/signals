#include <signals.h>
#include <cassert>

void staticFB(int& bind, bool& wasCalled)
{
    bind = 3;
    wasCalled = true;
}

int main()
{
    int bind = 0;
    bool wasCalled = false;
    Signal<bool&> s;

    Connection c = s.connect(&staticFB, std::ref(bind));

    s.emit(wasCalled);

    assert(wasCalled);
    assert(bind == 3);
    return EXIT_SUCCESS;
}

