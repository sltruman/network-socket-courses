#ifndef ANIMATIONS_H
#define ANIMATIONS_H
#include "three-widget_global.h"

#include <string>
using std::string;
#include <set>
using std::set;

class ThreeWidget;

namespace Animations {
    struct Plugin;
    TW_LIBRARY Plugin* CreateThreePlugin(ThreeWidget* tw);
    TW_LIBRARY void DestoryThreePlugin(Plugin*);
    TW_LIBRARY void TestAnimations(Plugin* plugin,bool enable);
    TW_LIBRARY void SetModelState(Plugin* p,string name,double value);
    TW_LIBRARY set<string> Animations(Plugin* plugin);
}

#endif // ANIMATIONS_H
