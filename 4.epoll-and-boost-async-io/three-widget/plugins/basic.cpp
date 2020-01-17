#include "basic.h"
#include "threewidget.h"
#if defined(OSGEARTH)
#include "threeearthwidget.h"
#endif

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
#include <osgUtil/LineSegmentIntersector>
using namespace osgUtil;

#include <set>
using namespace std;

#include <QSettings>
static QSettings settings(GLOBAL_COMPANY,GLOBAL_NAME);

namespace ThreeQt {
struct Basic::Plugin {
    ThreeWidget* tw;
    ref_ptr<MatrixTransform> mtScene;
    ref_ptr<Node> scene;
    ref_ptr<Camera> background;
    ref_ptr<Group> bubbles;
};

struct Basic::NameVisitor : public NodeVisitor {
    NameVisitor() : NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
    virtual void apply(MatrixTransform& node) {
        if(!node.getName().empty()) {
            outNames.insert(node.getName());
            outNodes.insert(&node);
        }
        traverse(node);
    }

    set<string> outNames;
    set<MatrixTransform*> outNodes;
};

struct Basic::LookingForVisitor : public NodeVisitor {
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

struct Basic::HeightLightCallback : public Uniform::Callback {
    HeightLightCallback() : incRed(false) {}
    virtual void operator()(Uniform* uniform, NodeVisitor*) {
        if(!uniform) return;

        Vec4 color;
        uniform->get(color);

        if(incRed) {
            if(color.y() < 1.0) color.y() += 0.01;
            else incRed = false;
        } else {
            if(color.y() > 0.0) color.y() -= 0.01;
            else incRed = true;
        }

        uniform->set(color);
    }

    bool incRed;
};

Basic::Basic(ThreeWidget* tw) {
    md = new Basic::Plugin;
    md->tw = tw;
    md->mtScene = new MatrixTransform;

    md->background = new osg::Camera;
    md->background->setClearMask(0);
    md->background->setCullingActive( false );
    md->background->setAllowEventFocus( false );
    md->background->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    md->background->setRenderOrder( osg::Camera::POST_RENDER );
    md->background->setProjectionMatrix( osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0) );

    StateSet* ss = md->background->getOrCreateStateSet();
    ss->setMode( StateAttribute::LIGHT, osg::StateAttribute::OFF );
    ss->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 1.0, 1.0));

    md->tw->worldScene->addChild(md->background);
    md->bubbles = new Group();
    md->tw->initScene->addChild(md->bubbles);
}

Basic::~Basic() {
    md->tw->worldScene->removeChild(md->background);
    delete md;
}

string Basic::OpenScene(string modelPath) {
    CloseScene();
    md->scene = readRefNodeFile(modelPath);
    md->mtScene->addChild(md->scene);
    md->tw->worldScene->addChild(md->mtScene);

    BoundingSphere bound = md->tw->worldScene->getBound();
    Vec3 bc = bound.center();
    float br = bound.radius() * 1.5;
    md->tw->tm->setHomePosition(Vec3(br,-br,br) + bc,bc,Vec3(0,0,1));
    md->tw->tm->home(0);

    stringstream fmt; fmt << "scene_" << hex << md->scene.get();
    md->mtScene->setName(fmt.str());
    return fmt.str();
}

void Basic::AddScene(string modelPath) {
    md->scene = readRefNodeFile(modelPath);
    md->mtScene->addChild(md->scene);
    md->tw->worldScene->addChild(md->mtScene);

    BoundingSphere bound = md->tw->worldScene->getBound();
    Vec3 bc = bound.center();
    float br = bound.radius() * 1.5;
    md->tw->tm->setHomePosition(Vec3(br,-br,br) + bc,bc,Vec3(0,0,1));
    md->tw->tm->home(0);
}

void Basic::CloseScene(){
    md->mtScene->removeChild(0,md->mtScene->getNumChildren());
    md->bubbles->removeChild(0,md->bubbles->getNumChildren());
}

void Basic::AxisRotate(float x,float y,float z,double degrees) {
    md->mtScene->setMatrix(Matrix::rotate(DegreesToRadians(degrees),Vec3(x,y,z)));
}

set<string> Basic::Models() {
    NameVisitor nv;
    md->mtScene->accept(nv);
    return nv.outNames;
}

void Basic::HeightLight(string name,float rgba[4]){
    if(name.empty() || rgba[4] == NULL) return;

    LookingForVisitor nv;
    nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(NULL == nv.outNode) return;
    if(0 == nv.outNode->getNumChildren()) return;
    StateSet* ss = nv.outNode->getOrCreateStateSet();

    char vs[] = "\
            void main() {\
                gl_Position = ftransform();\
            }";
    char fs[] = "\
            uniform vec4 mainColor;\
            void main() {\
                gl_FragColor = mainColor;\
                gl_FragDepth = 0.0;\
            }";

    Program* program = new Program;
    Shader* shaderV = new Shader(Shader::VERTEX,vs);
    Shader* shaderF = new Shader(Shader::FRAGMENT,fs);
    program->addShader(shaderV);
    program->addShader(shaderF);
    Uniform* mainColor = new Uniform("mainColor",Vec4(rgba[0],rgba[1],rgba[2],rgba[3]));
    ss->addUniform(mainColor);
    ss->setAttributeAndModes(program);
}
void Basic::HeightLight(string name,bool enable){
    LookingForVisitor nv;
    nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(NULL == nv.outNode) return;
    if(0 == nv.outNode->getNumChildren()) return;
    StateSet* lookingModelStateSet = nv.outNode->getOrCreateStateSet();

    if(!enable) {
        lookingModelStateSet->removeAttribute(StateAttribute::PROGRAM);
        return;
    }

    char vs[] = "\
            varying vec3 normal;\
            void main() {\
                normal = normalize(gl_NormalMatrix * gl_Normal);\
                gl_Position = ftransform();\
            }";
    char fs[] = "\
            uniform vec4 mainColor;\
            varying vec3 normal;\
            void main() {\
                gl_FragColor = mainColor;\
                gl_FragDepth = 0.0;\
            }";

    Program* program = new Program;
    Shader* shaderV = new Shader(Shader::VERTEX,vs);
    Shader* shaderF = new Shader(Shader::FRAGMENT,fs);
    program->addShader(shaderV);
    program->addShader(shaderF);
    Uniform* mainColor = new Uniform("mainColor",Vec4(1.0,0.5,0.5,0.5));
    mainColor->setUpdateCallback(new HeightLightCallback);
    lookingModelStateSet->addUniform(mainColor);
    lookingModelStateSet->setAttributeAndModes(program);
}

void Basic::SetBackground(string imagePath) {
    if(!imagePath.empty()) {
        osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
        osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imagePath);
        texture->setImage(image.get());

        osg::ref_ptr<osg::Drawable> quad = osg::createTexturedQuadGeometry(osg::Vec3(), osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f) );
        quad->getOrCreateStateSet()->setTextureAttributeAndModes( 0,texture.get());

        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( quad.get());
        md->background->addChild(geode.get());
    } else {
        md->background->removeChild(0,md->background->getNumChildren());
    }
}
void Basic::SetBackground(float r,float g,float b,float a){
    Camera* c = md->tw->viewer->getCamera();
    c->setClearColor(Vec4(r,g,b,a));
}
void Basic::SetHandleFrame(bool enable){
    if(md->tw->frameHandle == enable) return;
    md->tw->frameHandle = enable;
    if(!enable) RunFrame();
}
void Basic::RunFrame(){
    if(md->tw->viewer.valid() && md->tw->isValid()) {
        md->tw->makeCurrent();
        md->tw->viewer->frame();
        md->tw->doneCurrent();
    }

    md->tw->update();
}
void Basic::BoundBox(float center[3],float& radius) {
    auto b = md->tw->worldScene->getBound();
    center[0] = b.center()[0];
    center[1] = b.center()[1];
    center[2] = b.center()[2];
    radius = b.radius();
}
void Basic::Bubbles(bool enable){
    md->bubbles->removeChild(0,md->bubbles->getNumChildren());
    if(enable) {
        QString fontFilePath = settings.value("fontFilePath").toString();

        NameVisitor nv;
        md->tw->worldScene->accept(nv);

        for(set<MatrixTransform*>::iterator it = nv.outNodes.begin();it != nv.outNodes.end();it++) {
            MatrixTransform* node = *it;
            Text* text = new Text();
            text->setNodeMask(0x1);
            text->setText(node->getName(),String::ENCODING_UTF8);
            text->setFont(osgText::readFontFile(fontFilePath.toLocal8Bit().data()));
            text->setColor(Vec4(1,1,0,1));
            text->setCharacterSize(40);
            text->setAxisAlignment(TextBase::SCREEN);
            text->setCharacterSizeMode(Text::SCREEN_COORDS);
            text->setAlignment(Text::CENTER_BOTTOM);

            BoundingSphere b = node->getBound();
            if(0 == node->getNumChildren()) continue;
            Vec3 center = node->getChild(0)->getBound().center() * node->getWorldMatrices()[0];
            center.z() += b.radius();
            text->setPosition(center);
            Geode* gtext = new Geode();
            gtext->addDrawable(text);
            LOD* lod = new LOD();
            lod->addChild(gtext,0, b.radius() * 7);
            md->bubbles->addChild(lod);
        }
    }
}

string Basic::Intersect(vec2 xy, unsigned int mask) {
    LineSegmentIntersector::Intersections lis;
    if(!md->tw->viewer->computeIntersections(xy[0],xy[1],lis,mask)) return "";

    NodePath np = lis.begin()->nodePath;
    Transform* n = nullptr;
    for(auto it = np.begin();np.end() != it;it++) {
        n = dynamic_cast<Transform*>(*it);
        if(n != nullptr && !n->getName().empty()) break;
    }

    return n->getName();
}

pair<vec3,string> Basic::Intersect(float x,float y, unsigned int mask) {
    LineSegmentIntersector::Intersections lis;
    if(!md->tw->viewer->computeIntersections(x,y,lis,mask)) return make_pair(vec3{},"");

    NodePath np = lis.begin()->nodePath;
    Transform* n = nullptr;
    for(auto it = np.begin();np.end() != it;it++) {
        n = dynamic_cast<Transform*>(*it);
        if(n != nullptr && !n->getName().empty()) break;
    }

    Vec3 point = lis.begin()->getWorldIntersectPoint();
    return make_pair(vec3{point[0],point[1],point[2]},n->getName());
}
}
