#ifndef EDITOR_H
#define EDITOR_H
#include "three-widget_global.h"

#include <set>
#include <string>

using std::set;
using std::string;

class ThreeWidget;

namespace Editor {
    class Plugin;
    TW_LIBRARY Plugin* CreateThreePlugin(ThreeWidget*);
    TW_LIBRARY void DestoryThreePlugin(Plugin*);
    TW_LIBRARY void AddModel(Plugin*,string modelPath,string name);
    TW_LIBRARY void RemoveModel(Plugin*,string name);
    TW_LIBRARY void MoveModel(Plugin*,string name,float xyz[3]);
    TW_LIBRARY void SetModelPos(Plugin*,string name,float xyz[3]);
    TW_LIBRARY bool MakeCube(string path,float xyz[3],float radius);
    TW_LIBRARY bool MakeSphere(string path,float xyz[3],float radius);
    TW_LIBRARY bool MakeBase(string path,float xyz[3],float length);
}

#endif // EDITOR_H
