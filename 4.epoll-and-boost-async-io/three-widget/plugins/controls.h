#ifndef CONTROLS_H
#define CONTROLS_H
#include "three-widget_global.h"

#include <string>
using std::string;

#include <functional>
using std::function;

#include <vector>
using std::vector;

namespace ThreeQt {
typedef vector<double> vec2;
typedef vector<double> vec3;

class ThreeWidget;
class ThreeEarthWidget;

struct TW_LIBRARY Controls {
    enum MouseButton {
        None = -1,
        Left    = 1<<0,
        Middle  = 1<<1,
        Right   = 1<<2
    };

    enum MouseEventType {
        Push                = 1<<0,
        Release             = 1<<1,
        DoubleClick         = 1<<2,
        Drag                = 1<<3
    };

    enum ScrollingMotion
    {
        SCROLL_NONE,
        SCROLL_LEFT,
        SCROLL_RIGHT,
        SCROLL_UP,
        SCROLL_DOWN,
        SCROLL_2D
    };

    Controls(ThreeWidget* tw);
    ~Controls();
    void SetMouseButton(MouseButton mouseCenter, MouseButton mousePan,MouseButton mouseRotate);
    void SetMouseWheel(ScrollingMotion mouseWheelUp, ScrollingMotion mouseWheelDown);
    void SetMouseOverEventHandler(function<bool(vec2 xy)> f);
    void SetMouseClickEventHandler(function<bool(MouseButton,MouseEventType,vec2 xy,vec2 xyNormalized)> f);
    void SetMouseWheelEventHandler(function<bool(ScrollingMotion,double distance)> f);
    void SetFrameEventhandler(function<void()> f);

    struct Plugin;
    struct EventHandler;
    Plugin* md;
};
}

#endif // CONTROLS_H
