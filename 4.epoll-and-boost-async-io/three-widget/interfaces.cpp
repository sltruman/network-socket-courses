#include "interfaces.h"
#include "threewidget.h"
#if defined (OSGEARTH)
#include "threeearthwidget.h"
#endif
#include "plugins/authorization.h"

#include <iostream>
using namespace std;

namespace ThreeQt {

ThreeWidget* CreateThreeWidget() {
    if(!Authorization::verity()) {
        cerr << "[CreateThreeWidget] 运行./config设置授权文件" << endl;
        return NULL;
    }

    cout << "[CreateThreeWidget] 注册三维插件窗体" << endl;
    ThreeWidget* tw = new ThreeWidget;
    return tw;
}

void DestoryThreeWidget(ThreeWidget* w) {
    cout << "[DestoryThreeWidget] 注销三维插件窗体" << endl;
    delete w;
}

#if defined (OSGEARTH)
ThreeEarthWidget* CreateThreeEarthWidget() {
    if(!Authorization::verity()) {
        cerr << "[CreateThreeEarthWidget] 运行./config设置授权文件" << endl;
        return NULL;
    }

    cout << "[CreateThreeEarthWidget] 注册三维插件窗体" << endl;
    ThreeEarthWidget* tew = new ThreeEarthWidget;
    return tew;
}

void DestoryThreeWidget(ThreeEarthWidget* tew) {
    cout << "[DestoryThreeWidget] 注销三维插件窗体" << endl;
    delete tew;
}


#endif
}
