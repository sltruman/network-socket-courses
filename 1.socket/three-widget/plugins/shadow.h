#ifndef SHADOWMAP_H
#define SHADOWMAP_H
#include "three-widget_global.h"

class ThreeWidget;

namespace Shadow {
    struct Plugin;
    TW_LIBRARY Plugin* CreateThreePlugin(ThreeWidget* tw);
    TW_LIBRARY void TW_LIBRARY DestoryThreePlugin(Plugin*);
    TW_LIBRARY void TW_LIBRARY Lighting(Plugin*,bool enable);
}

#endif // SHADOWMAP_H
