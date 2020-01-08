#ifndef SHADOWMAP_H
#define SHADOWMAP_H
#include "three-widget_global.h"

namespace ThreeQt {
class ThreeWidget;

struct TW_LIBRARY Shadow {
    Shadow(ThreeWidget* tw);
    ~Shadow();
    void Lighting(bool enable);

    struct Plugin;
    Plugin* md;
};
}
#endif // SHADOWMAP_H
