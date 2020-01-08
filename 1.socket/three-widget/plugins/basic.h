#ifndef BASIC_H
#define BASIC_H
#include "three-widget_global.h"

#include <set>
#include <string>
using std::set;
using std::string;

class ThreeWidget;
class ThreeEarthWidget;

namespace Basic {
    class Plugin;
    class EarthPlugin;

    TW_LIBRARY Plugin* CreateThreePlugin(ThreeWidget*);
    TW_LIBRARY void DestoryThreePlugin(Plugin*);
    TW_LIBRARY void OpenScene(Plugin*,string modelPath);
    TW_LIBRARY void CloseScene(Plugin*);
    TW_LIBRARY void AxisRotate(Plugin*,float x,float y,float z,double degress);
    TW_LIBRARY set<string> Models(Plugin*);
    TW_LIBRARY void HeightLight(Plugin*,string name,float rgba[4]);
    TW_LIBRARY void HeightLight(Plugin*,string name,bool enable);
    TW_LIBRARY void SetBackground(Plugin*,string imagePath);
    TW_LIBRARY void SetBackground(Plugin*,float r,float g,float b,float a);
    TW_LIBRARY void SetHandleFrame(Plugin*,bool enable);
    TW_LIBRARY void RunFrame(Plugin*);
    TW_LIBRARY void BoundBox(Plugin*,float center[3],float& radius);
    TW_LIBRARY void Bubbles(Plugin*,bool enable,float timeOfRadius = 10.0f);

#if defined (OSGEARTH)
    TW_LIBRARY EarthPlugin* CreateThreePlugin(ThreeEarthWidget* tew);
    TW_LIBRARY void DestoryThreePlugin(EarthPlugin*);
    TW_LIBRARY void CameraPosition(EarthPlugin* p,double &longitude,double &latitude,double &pitch,double &range);
    TW_LIBRARY void SetHomePosition(EarthPlugin* p,double longitude,double latitude,double pitch,double range);
    TW_LIBRARY void Home(EarthPlugin* p);
    TW_LIBRARY void RunFrame(EarthPlugin* p);
    TW_LIBRARY void SetHandleFrame(EarthPlugin* p,bool enable);
    TW_LIBRARY void LookingFor(EarthPlugin* p,double longitude,double latitude,double pitch,double range,double duration);
#endif
}

#endif // BASIC_H
