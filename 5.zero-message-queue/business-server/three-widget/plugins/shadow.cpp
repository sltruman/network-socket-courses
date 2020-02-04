#include "shadow.h"
#include "threewidget.h"

#include <osg/MatrixTransform>
#include <osg/NodeVisitor>
using namespace osg;

#include <osgDB/ReadFile>
using namespace osgDB;
#include <osgText/Text>
using namespace osgText;
#include <osgAnimation/Animation>
#include <osgAnimation/BasicAnimationManager>
using namespace osgAnimation;
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowMap>
using namespace osgShadow;

#include <set>
using namespace std;

#include <QSettings>

namespace ThreeQt {
    struct Shadow::Plugin {
        ThreeWidget* tw;
        ref_ptr<ShadowedScene> scene;
    };

    Shadow::Shadow(ThreeWidget* tw) {
        md = new Plugin();
        md->tw = tw;

        LightSource* ls = new LightSource;
        ls->getLight()->setLightNum(1);
        Vec3 pos(5000.0,-5000.0,10000.0);
        ls->getLight()->setPosition(Vec4(pos,0.0));
        Vec3 dir = -pos;
        dir.normalize();
        ls->getLight()->setDirection(dir);
        ls->getLight()->setAmbient(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ls->getLight()->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ls->getLight()->setSpecular(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

        ShadowMap* shadowMap = new ShadowMap;
        shadowMap->setTextureSize(Vec2s(640 * 8,480 * 8));
        shadowMap->setAmbientBias(Vec2(0.8f,0.5f));
        shadowMap->setLight(ls);

        md->scene = new ShadowedScene(shadowMap);
        md->scene->addChild(ls);
        md->scene->addChild(md->tw->worldScene);
    }

    Shadow::~Shadow() {
        delete md;
    }

    void Shadow::Lighting(bool enable) {
        StateSet* ss = md->scene->getOrCreateStateSet();

        if(enable) {
            md->tw->initScene->replaceChild(md->tw->worldScene,md->scene);
            ss->setMode(GL_LIGHT1,StateAttribute::ON);
        } else {
            md->tw->initScene->replaceChild(md->scene,md->tw->worldScene);
            ss->setMode(GL_LIGHT1,StateAttribute::OFF);
        }
    }
}
