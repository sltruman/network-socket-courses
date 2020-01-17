#ifndef VIEWER_H
#define VIEWER_H
#include "three-widget_global.h"

#include <string>
using std::string;

class ThreeWidget;

namespace Observer {
    class Plugin;
    TW_LIBRARY Plugin* CreateThreePlugin(ThreeWidget*);
    TW_LIBRARY void DestoryThreePlugin(Plugin*);
    TW_LIBRARY void SwitchFirstPerson(Plugin*,bool enable);
    TW_LIBRARY void LookingFor(Plugin*,string name);
    TW_LIBRARY void LookingFor(Plugin*,float xyz[3],float distance);
    TW_LIBRARY void Home(Plugin*);
    TW_LIBRARY void HomePosition(Plugin*,float eye[3],float center[3],float up[3]);
    TW_LIBRARY void SetHomePosition(Plugin*,float eye[3],float center[3],float up[3]);
    TW_LIBRARY void CameraPosition(Plugin*,float eye[3],float center[3],float up[3]);
    TW_LIBRARY void SetCameraPosition(Plugin*,float eye[3],float center[3],float up[3]);
}

#endif // VIEWER_H
