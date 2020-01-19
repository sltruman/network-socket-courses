#ifndef VIEWER_H
#define VIEWER_H
#include "three-widget_global.h"

#include <string>
#include <vector>
using std::vector;
using std::string;

namespace ThreeQt {
typedef vector<double> vec2;
typedef vector<double> vec3;

class ThreeWidget;

struct TW_LIBRARY Observer {
    Observer(ThreeWidget*);
    ~Observer();
    void SwitchFirstPerson(bool enable);
    void LookingFor(string name);
    void LookingFor(vec3 xyz,double distance = 0.0);
    void Home();
    void HomePosition(float eye[3],float center[3],float up[3]);
    void SetHomePosition(float eye[3],float center[3],float up[3]);
    void CameraPosition(float eye[3],float center[3],float up[3]);
    void SetCameraPosition(float eye[3],float center[3],float up[3]);

    struct Plugin;
    struct LookingForVisitor;
    Plugin* md;
};
}
#endif // VIEWER_H
