#ifndef ANIMATIONS_H
#define ANIMATIONS_H
#include "three-widget_global.h"

#include <string>
using std::string;
#include <set>
using std::set;

namespace ThreeQt {
class ThreeWidget;

struct TW_LIBRARY Animations {
    Animations(ThreeWidget* tw);
    ~Animations();
    void TestAnimations(bool enable);
    void TestAnimation(string name,bool enable);
    void SetModelState(string name,double value);
    set<string> AnimationNames();

    struct Plugin;
    struct AnimationVisitor;
    Plugin* md;
};
}
#endif // ANIMATIONS_H
