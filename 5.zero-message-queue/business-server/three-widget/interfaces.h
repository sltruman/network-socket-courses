#ifndef INTERFACES_H
#define INTERFACES_H

#include "three-widget_global.h"

namespace ThreeQt {

class ThreeWidget;
class ThreeEarthWidget;

TW_LIBRARY ThreeWidget* CreateThreeWidget();
TW_LIBRARY void DestoryThreeWidget(ThreeWidget* tw);

#if defined (OSGEARTH)
TW_LIBRARY ThreeEarthWidget* CreateThreeEarthWidget();
TW_LIBRARY void DestoryThreeWidget(ThreeEarthWidget* tw);
#endif

}
#endif // INTERFACES_H


