#ifndef CONTROLS_H
#define CONTROLS_H

#include <QtCore/qglobal.h>

#if defined(TW_LIBRARY)
#  define TW_LIBRARY Q_DECL_EXPORT
#else
#  define TW_LIBRARY Q_DECL_IMPORT
#endif


#include <string>
using std::string;

#if defined (QT5)
#include <functional>
using std::function;
#endif

class ThreeWidget;
class ThreeEarthWidget;

namespace Controls {
    class Plugin;
    class EarthPlugin;

    enum MouseButtonMask {
        NONE = -1,
        LEFT_MOUSE_BUTTON    = 1<<0,
        MIDDLE_MOUSE_BUTTON  = 1<<1,
        RIGHT_MOUSE_BUTTON   = 1<<2
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

    TW_LIBRARY Plugin* CreateThreePlugin(ThreeWidget* tw);
    TW_LIBRARY void DestoryThreePlugin(Plugin* p);
    TW_LIBRARY void SwitchTeleportMode(Plugin* p,bool enable);
    TW_LIBRARY void SetMouseButton(Plugin* plugin,MouseButtonMask mouseCenter, MouseButtonMask mousePan,MouseButtonMask mouseRotate);
    TW_LIBRARY void SetMouseWheel(Plugin* p,ScrollingMotion mouseWheelUp, ScrollingMotion mouseWheelDown);
#if defined (QT5)
    TW_LIBRARY void SetMouseOverEventHandler(Plugin* p,function<bool(string name,float xyz[3])> f);
    TW_LIBRARY void SetMouseClickEventHandler(Plugin* p,function<bool(MouseButtonMask,string name,float xyz[3])> f);
    TW_LIBRARY void SetMouseWheelEventHandler(Plugin* p,function<bool(ScrollingMotion,double distance)> f);
#else
    struct MouseOverEventHandler {
        virtual bool operator()(string name,float xyz[3])=0;
    };

    TW_LIBRARY void SetMouseOverEventHandler(Plugin* plugin,MouseOverEventHandler* f);

    struct MouseClickEventHandler {
        virtual bool operator()(MouseButtonMask,string name,float xyz[3])=0;
    };

    TW_LIBRARY void SetMouseClickEventHandler(Plugin* p,MouseClickEventHandler* f);
#endif
#if defined (OSGEARTH)
    TW_LIBRARY void DestoryThreePlugin(EarthPlugin* p);
    TW_LIBRARY EarthPlugin* CreateThreePlugin(ThreeEarthWidget* tw);
    TW_LIBRARY void SetMouseWheelEventHandler(EarthPlugin* p,function<void(ScrollingMotion,double range)> f);
#endif

}


#endif // CONTROLS_H
