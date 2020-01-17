#ifndef BASIC_H
#define BASIC_H
#include "three-widget_global.h"

#include <set>
#include <string>
#include <vector>
using std::set;
using std::string;
using std::vector;
using std::pair;

namespace ThreeQt {
typedef vector<double> vec2;
typedef vector<double> vec3;

class ThreeWidget;
class ThreeEarthWidget;

struct TW_LIBRARY Basic {
    Basic(ThreeWidget* tw);
    ~Basic();
    string OpenScene(string modelPath);
    void AddScene(string modelPath);
    void CloseScene();
    void AxisRotate(float x,float y,float z,double degress);
    set<string> Models();
    void HeightLight(string name,float rgba[4]);
    void HeightLight(string name,bool enable);
    void SetBackground(string imagePath);
    void SetBackground(float r,float g,float b,float a);
    void SetHandleFrame(bool enable);
    void RunFrame();
    void BoundBox(float center[3],float& radius);
    void Bubbles(bool enable);
    string Intersect(vec2 xy, unsigned int mask);
    pair<vec3,string> Intersect(float x,float y,unsigned int mask = 0x2);

    struct Plugin;
    struct NameVisitor;
    struct LookingForVisitor;
    struct HeightLightCallback;
    Plugin* md;
};
}

#endif // BASIC_H
