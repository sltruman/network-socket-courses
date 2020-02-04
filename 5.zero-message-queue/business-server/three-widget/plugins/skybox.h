#ifndef SKYBOX_H
#define SKYBOX_H

#include "three-widget_global.h"
#include <string>
using std::string;

namespace ThreeQt {

class ThreeWidget;

struct TW_LIBRARY SkyBox {
     SkyBox(ThreeWidget* tw);
     ~SkyBox();

     void CreateSkyBox(string dirPath);

     struct Plugin;
     struct TexMatCallback;
     class MoveEarthySkyWithEyePointTransform;

     Plugin* md;
};

}

#endif
