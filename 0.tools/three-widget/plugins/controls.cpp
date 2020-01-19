#include "controls.h"
#include "threewidget.h"
#if defined(OSGEARTH)
#include "threeearthwidget.h"
#endif

#include <osgGA/GUIEventHandler>
using osgGA::GUIEventHandler;
using osgGA::GUIEventAdapter;
using osgGA::GUIActionAdapter;
#include <osgUtil/LineSegmentIntersector>
using osgUtil::LineSegmentIntersector;
using osgUtil::Intersector;
#include <osgUtil/IntersectVisitor>
using osgUtil::IntersectionVisitor;
using osgViewer::View;
using osg::Vec3;
using osg::NodePath;
using osg::Transform;

#include <iostream>
using namespace std;

namespace ThreeQt {


struct Controls::Plugin {
    ThreeWidget* tw;
    ref_ptr<EventHandler> handler;
};

struct Controls::EventHandler : public GUIEventHandler {
    function<bool(vec2 xy)> mouseOverCallback;
    function<bool(MouseButton,MouseEventType,vec2 xy,vec2 xyNormalized)> mouseClickCallback;
    function<bool(ScrollingMotion,double)> mouseWheelCallback;
    function<void()> frameCallback;

    bool scroll;
    ScrollingMotion sm;
    double t;
    MouseButton mb;

    EventHandler() : GUIEventHandler(), scroll(false), t(0) {}

    virtual bool handle(const GUIEventAdapter& ea,GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor*) {
        View* view = dynamic_cast<View*>(&aa);
        auto et = ea.getEventType();
        auto bm = ea.getButtonMask();

        switch(et) {
        case GUIEventAdapter::MOVE:
            if(!mouseOverCallback) break;
            return mouseOverCallback({ea.getX(),ea.getY()});
        case GUIEventAdapter::PUSH:
            if(!mouseClickCallback) break;
            mb = static_cast<MouseButton>(bm);
            if((ea.getTime() - t) < 0.25)
                t = 0.0,mouseClickCallback(mb,static_cast<MouseEventType>(GUIEventAdapter::DOUBLECLICK),{ea.getX(),ea.getY()},{ea.getXnormalized(),ea.getYnormalized()});
            else
                t = ea.getTime();
        case GUIEventAdapter::DRAG:
        case GUIEventAdapter::RELEASE:
            if(!mouseClickCallback) break;
            return mouseClickCallback(mb,static_cast<MouseEventType>(et),{ea.getX(),ea.getY()},{ea.getXnormalized(),ea.getYnormalized()});
        case GUIEventAdapter::SCROLL: {
            if(!mouseWheelCallback) break;
            ThreeManipulator* tm = dynamic_cast<ThreeManipulator*>(view->getCameraManipulator());
            if(tm != NULL) return mouseWheelCallback(sm,tm->getDistance());
#if defined (OSGEARTH)
            else if(em != nullptr) {
                auto vp = em->getViewpoint();
                return mouseWheelCallback(sm,vp.getRange());
            }
#endif
        }
        case GUIEventAdapter::FRAME:
            if(!frameCallback) break;
            frameCallback();
            break;
        default:break;
        }

        return false;
    };
};

Controls::Controls(ThreeWidget* tw) {
    md = new Plugin;
    md->tw = tw;
    md->handler = new EventHandler;
    md->tw->viewer->addEventHandler(md->handler);
}
Controls::~Controls() {
    md->tw->viewer->removeEventHandler(md->handler);
    delete md;
}
void Controls::SetMouseButton(MouseButton mouseCenter, MouseButton mousePan,MouseButton mouseRotate) {
    md->tw->mouseLeft = mouseCenter;
    md->tw->mouseMid = mousePan;
    md->tw->mouseRight = mouseRotate;
}
void Controls::SetMouseWheel(ScrollingMotion mouseWheelUp, ScrollingMotion mouseWheelDown) {
    md->tw->mouseWheelUp = mouseWheelUp;
    md->tw->mouseWheelDown = mouseWheelDown;
}
void Controls::SetMouseOverEventHandler(function<bool (vec2 xy)> f) {
    md->handler->mouseOverCallback = f;
}
void Controls::SetMouseClickEventHandler(function<bool (MouseButton, MouseEventType,vec2 xy,vec2 xyNormalized)> f) {
    md->handler->mouseClickCallback = f;
}
void Controls::SetMouseWheelEventHandler(function<bool(ScrollingMotion,double distance)> f) {
    md->handler->mouseWheelCallback = f;
}
void Controls::SetFrameEventhandler(function<void()> f) {
    md->handler->frameCallback = f;
}

}
