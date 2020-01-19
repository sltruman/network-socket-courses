#ifndef EDITOR_H
#define EDITOR_H
#include "three-widget_global.h"

#include <set>
#include <string>
#include <vector>
#include <utility>
using std::set;
using std::string;
using std::vector;
using std::pair;

namespace ThreeQt {
typedef vector<double> vec2;
typedef vector<double> vec3;

class ThreeWidget;

struct TW_LIBRARY Editor {
    Editor(ThreeWidget*);
    ~Editor();
    string AddModel(string modelPath,string name = "");
    void RemoveModel(string name);
    void RotateModel(string name,vec3 xyz,double degrees);
    void ModelPos(string name,vec3 xyz);
    vec3 ModelPos(string name);

    void ModelTrans(string name,vec3 xyz);
    vec3 ModelTrans(string name);
    void ModelRotate(string name,vec3 axis,double degrees);
    vec3 ModelRotate(string name);
    void ModelScale(string name,vec3 xyz);
    vec3 ModelScale(string name);

    void MaskModel(string name,unsigned int mask);
    string CopyModel(string name);
    set<string> PlaneIntersect(vec2 lt,vec2 rd,unsigned int mask = 0x2);
    pair<vec3,float> BoundBox(string name);
    string MakeTranslateAxisDragger(vec3 xyz,float radius,unsigned int mask = 0x1);
    void MakePlane(string name,vec2 lt,vec2 rd);
    static bool MakeCube(string path,float xyz[3],float radius);
    static bool MakeSphere(string path,float xyz[3],float radius);
    static bool MakeBase(string path,float xyz[3],int length,int interval=1);

    struct Plugin;
    struct LookingForVisitor;
    Plugin* md;
};
}
#endif // EDITOR_H
