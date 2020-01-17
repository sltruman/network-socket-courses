#include "observer.h"
#include "threewidget.h"
#include "threemanipulator.h"
#include "teleportmanipulator.h"

#include <osg/MatrixTransform>
#include <osg/NodeVisitor>
#include <osg/Depth>
using namespace osg;

#include <osgDB/ReadFile>
using namespace osgDB;

#include <osgText/Text>
using namespace osgText;

#include <osgViewer/ViewerBase>
using namespace osgViewer;
#include <osgUtil/Optimizer>

#include <osgGA/FirstPersonManipulator>
using osgGA::FirstPersonManipulator;

#include <set>
using namespace std;

#include <QSettings>

namespace ThreeQt {

static QSettings settings(GLOBAL_COMPANY,GLOBAL_NAME);

struct Observer::Plugin {
    ThreeWidget* tw;
    ref_ptr<ThreeManipulator> tm;
    ref_ptr<TeleportManipulator> tpm;
};

struct Observer::LookingForVisitor : public osg::NodeVisitor {
    LookingForVisitor() : NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
    virtual void apply(MatrixTransform& node) {
        if(node.getName() == inName && 0 < node.getBound().radius()) {
            outNode = &node;
        }

        traverse(node);
    }

    string inName;
    MatrixTransform* outNode;
};

Observer::Observer(ThreeWidget* tw) {
    md = new Plugin;
    md->tw = tw;
    md->tm = tw->tm;
    md->tpm = new TeleportManipulator();
    md->tpm->setAllowThrow(false);
}

Observer::~Observer() {
    delete md;
}

void Observer::SwitchFirstPerson(bool enable) {
    osg::Vec3d e,c,u;
    osg::Quat r;
    if(enable) {
        md->tm->getTransformation(e,r);
        md->tw->viewer->setCameraManipulator(md->tpm);
        md->tpm->setTransformation(e,r);

        md->tm->getHomePosition(e,c,u);
        md->tpm->setHomePosition(e,c,u);
    } else {
        md->tpm->getTransformation(e,r);
        md->tw->viewer->setCameraManipulator(md->tm);
        md->tm->setTransformation(e,r);

//            p->tpm->getTransformation(e,c,u);
//            double distance = (p->tm->getCenter() - c).length();
//            p->tm->setCenter(c);
    }
}

void Observer::LookingFor(string name) {
    if(name.empty()) return;

    LookingForVisitor nv;
    nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(NULL == nv.outNode) return;
    if(0 == nv.outNode->getNumChildren()) return;

    osg::BoundingSphere b = nv.outNode->getBound();
    if(0 == nv.outNode->getNumChildren()) return;
    Vec3 center = nv.outNode->getChild(0)->getBound().center() * nv.outNode->getWorldMatrices()[0];
    float distance = b.radius() * 4;
    md->tm->teleport(md->tw->viewer->getEventQueue(),center,distance);
}

void Observer::LookingFor(vec3 xyz,double distance) {
    if(xyz.empty()) return;
    Vec3 center(xyz[0],xyz[1],xyz[2]);
    md->tm->teleport(md->tw->viewer->getEventQueue(),center,distance == 0.0 ? md->tm->getDistance():distance);
}

void Observer::Home() {
    md->tm->home(0);
}

void Observer::HomePosition(float eye[3],float center[3],float up[3]) {
    Vec3d e,c,u;
    md->tm->getHomePosition(e,c,u);
    eye[0] = e[0],eye[1] = e[1],eye[2] = e[2];
    center[0] = c[0],center[1] = c[1],center[2] = c[2];
    up[0] = u[0],up[1] = u[1],up[2] = u[2];
}

void Observer::SetHomePosition(float eye[3],float center[3],float up[3]) {
    md->tm->setHomePosition(Vec3(eye[0],eye[1],eye[2]),Vec3(center[0],center[1],center[2]),Vec3(up[0],up[1],up[2]));
}

void Observer::CameraPosition(float eye[3],float center[3],float up[3]) {
    Vec3d e,c,u;
    md->tm->getTransformation(e,c,u);
    eye[0] = e[0],eye[1] = e[1],eye[2] = e[2];
    center[0] = c[0],center[1] = c[1],center[2] = c[2];
    up[0] = u[0],up[1] = u[1],up[2] = u[2];
}

void Observer::SetCameraPosition(float eye[3],float center[3],float up[3]) {
    md->tm->setTransformation(Vec3(eye[0],eye[1],eye[2]),Vec3(center[0],center[1],center[2]),Vec3(up[0],up[1],up[2]));
}
}
