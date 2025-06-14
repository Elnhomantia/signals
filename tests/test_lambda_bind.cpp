#include "signal.h"
#include <cassert>

int main()
{
    int bind = 0;
    bool wasCalled = false;
    Signal<bool&> s;

    Connection c = s.connect([](int& bind, bool& wasCalled){
        wasCalled = true;
        bind = 3;
    }, std::ref(bind));

    s.emit(wasCalled);

    assert(wasCalled);
    assert(bind == 3);
    return EXIT_SUCCESS;
}
