#include "editor.h"
#include "threewidget.h"

#include <sstream>
using namespace std;

#include <osg/PolygonMode>
#include <osg/LineWidth>
#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/BlendFunc>
#include <osg/AutoTransform>
using namespace osg;

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
using namespace osgDB;

#include <osgUtil/PolytopeIntersector>
using namespace osgUtil;

#include <osgManipulator/TranslateAxisDragger>
using namespace osgManipulator;

namespace ThreeQt {

struct Editor::Plugin {
    ThreeWidget* tw;
    ref_ptr<Camera> hudCamera;
    ref_ptr<Group> scene;
};

struct Editor::LookingForVisitor : public NodeVisitor {
    LookingForVisitor() : NodeVisitor(TRAVERSE_ALL_CHILDREN),outT(nullptr),outMt(nullptr) {}
    virtual void apply(Transform& node) {
        auto radius = node.getBound().radius();
        auto name = node.getName();

        if(name == inName && 0 < radius) {
            if(outT == nullptr) outT = &node;
            if(outMt == nullptr) outMt = dynamic_cast<MatrixTransform*>(&node);
        }

        traverse(node);
    }

    string inName;
    Transform* outT;
    MatrixTransform* outMt;
};

Editor::Editor(ThreeWidget* tw){
    md = new Plugin;
    md->tw = tw;

    md->hudCamera = new Camera;
    md->hudCamera->setProjectionMatrix(Matrix::ortho2D(-1.0,1.0,-1.0,1.0));
    md->hudCamera->setReferenceFrame(Transform::ABSOLUTE_RF);
    md->hudCamera->setViewMatrixAsLookAt(Vec3(0,-1,0), Vec3(0,0,0), Vec3(0,0,1));
    md->hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
    md->hudCamera->setRenderOrder(Camera::POST_RENDER);
    md->hudCamera->setAllowEventFocus(false);
    tw->initScene->addChild(md->hudCamera);
}

Editor::~Editor() {
    md->tw->initScene->removeChild(md->hudCamera);
    delete md;
}

string Editor::AddModel(string modelPath,string name) {
    auto model = readRefNodeFile(modelPath);
    if(!model.valid()) return "";

    if(name.empty()) {
        stringstream fmt; fmt << "model_" << hex << model.get();
        name = fmt.str();
    }

    auto mtModel = new MatrixTransform;
    mtModel->addChild(model);
    mtModel->setName(name);
    md->tw->worldScene->addChild(mtModel);
    return name;
}

void Editor::RemoveModel(string name) {
    if(name.empty()) return;
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outT) return;
    md->tw->worldScene->removeChild(nv.outT);
}

void Editor::RotateModel(string name,vec3 xyz,double degrees) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outMt) return;
    nv.outMt->postMult(Matrix::rotate(DegreesToRadians(degrees),Vec3d(xyz[0],xyz[1],xyz[2])));
}

void Editor::ModelPos(string name,vec3 xyz) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outMt) return;
    nv.outMt->getWorldMatrices()[0].setTrans(xyz[0],xyz[1],xyz[2]);
}

vec3 Editor::ModelPos(string name) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outMt) return vec3 {};
    auto pos = nv.outMt->getWorldMatrices()[0].getTrans();
    return vec3 {pos[0],pos[1],pos[2]};
}

void Editor::ModelTrans(string name, vec3 xyz) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outMt) return;
    auto mt = nv.outMt->getMatrix();
    nv.outMt->setMatrix(Matrix::scale(mt.getScale()) * Matrix::rotate(mt.getRotate()) * Matrix::translate(xyz[0],xyz[1],xyz[2]));
}

vec3 Editor::ModelTrans(string name) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outMt) return vec3 {};
    auto pos = nv.outMt->getMatrix().getTrans();
    return vec3 {pos[0],pos[1],pos[2]};
}

void Editor::ModelRotate(string name,vec3 axis,double degrees) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outMt) return;
    auto mt = nv.outMt->getMatrix();
    nv.outMt->setMatrix(Matrix::scale(mt.getScale()) * Matrix::rotate(DegreesToRadians(degrees),axis[0],axis[1],axis[2]) * Matrix::translate(mt.getTrans()));
}

vec3 Editor::ModelRotate(string name) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outMt) return vec3 {};
    auto v = nv.outMt->getMatrix().getRotate();
    return vec3 {v[0],v[1],v[2],RadiansToDegrees(v[3])};
}

void Editor::ModelScale(string name,vec3 xyz) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outMt) return;
    auto mt = nv.outMt->getMatrix();
    nv.outMt->setMatrix(Matrix::scale(xyz[0],xyz[1],xyz[2]) * Matrix::rotate(mt.getRotate()) * Matrix::translate(mt.getTrans()));
}

vec3 Editor::ModelScale(string name) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outMt) return vec3 {};
    auto v = nv.outMt->getMatrix().getScale();
    return vec3 {v[0],v[1],v[2]};
}

void Editor::MaskModel(string name,unsigned int nodeMask) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outT) return;
    nv.outT->setNodeMask(nodeMask);
}

string Editor::CopyModel(string name) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outT) return "";
    auto model = dynamic_cast<Node*>(nv.outT->clone(CopyOp::SHALLOW_COPY));
    stringstream fmt; fmt << "model_" << hex << model;
    auto newName = fmt.str();
    model->setName(newName);
    md->tw->worldScene->addChild(model);
    return newName;
}

set<string> Editor::PlaneIntersect(vec2 lt,vec2 rd,unsigned int mask) {
    set<string> names;

    auto picker = new PolytopeIntersector(Intersector::WINDOW,lt[0],rd[1],rd[0],lt[1]);
    IntersectionVisitor iv(picker);
    iv.setTraversalMask(mask);

    auto camera = md->tw->viewer->getCamera();
    camera->accept(iv);

    if (picker->containsIntersections()) {
        auto lis = picker->getIntersections();
        for(auto li : lis) {
            NodePath np = li.nodePath;
            for(auto n : np) {
                if(n->getName().empty() || nullptr == n->asTransform()) continue;
                names.insert(n->getName());
                break;
            }
        }
    }

    return names;
}

pair<vec3,float> Editor::BoundBox(string name) {
    LookingForVisitor nv; nv.inName = name;
    md->tw->worldScene->accept(nv);
    if(nullptr == nv.outT) return make_pair(vec3(),0);
    auto b = nv.outT->getBound();
    return make_pair(vec3{b.center()[0],b.center()[1],b.center()[2]},b.radius());
}

string Editor::MakeTranslateAxisDragger(vec3 xyz,float radius,unsigned int mask) {
    auto dragger = new TranslateAxisDragger();
    auto scale = radius / dragger->getPickCylinderRadius() / 40;

    dragger->setupDefaultGeometry();
    dragger->setConeHeight(0.3);
    dragger->setMatrix(Matrix::scale(Vec3(scale,scale,scale)) * Matrix::translate(xyz[0],xyz[1],xyz[2]));
    dragger->setHandleEvents(true);
    dragger->setNodeMask(mask);

    stringstream fmt;fmt << "dragger_" << hex << dragger;
    dragger->setName(fmt.str());

    md->tw->worldScene->addChild(dragger);
    return dragger->getName();
}

void Editor::MakePlane(string name,vec2 lt,vec2 rd) {
    Geometry* geo = nullptr;

    for(int i=0;i < md->hudCamera->getNumChildren();i++) {
        geo = dynamic_cast<Geometry*>(md->hudCamera->getChild(i));
        if(name == geo->getName()) break;
        geo = nullptr;
    }

    if(nullptr == geo) {
        geo = new Geometry;
        geo->setNodeMask(1);
        geo->setName(name);
        geo->setDataVariance(osg::Object::DYNAMIC);
        geo->setUseDisplayList(false);

        auto colors = new Vec4Array;
        colors->push_back(Vec4(1.0,1.0,0.0,1.0));
        geo->setColorArray(colors);
        geo->setColorBinding(osg::Geometry::BIND_OVERALL);

        auto normal = new Vec3Array(1);
        (*normal)[0]=osg::Vec3(0,-1,0);
        geo->setNormalArray(normal);
        geo->setNormalBinding(osg::Geometry::BIND_OVERALL);

        auto pri = new DrawArrays(PrimitiveSet::LINE_LOOP,0,4);
        geo->addPrimitiveSet(pri);

        auto polyMode = new PolygonMode;
        polyMode->setMode(PolygonMode::FRONT_AND_BACK, PolygonMode::LINE);
        geo->getOrCreateStateSet()->setAttributeAndModes(polyMode);
        geo->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        md->hudCamera->addChild(geo);
    }

    auto vertex = new Vec3Array(4);
    (*vertex)[0]=osg::Vec3(lt[0],0,lt[1]);
    (*vertex)[1]=osg::Vec3(lt[0],0,rd[1]);
    (*vertex)[2]=osg::Vec3(rd[0],0,rd[1]);
    (*vertex)[3]=osg::Vec3(rd[0],0,lt[1]);
    geo->setVertexArray(vertex);
}

bool Editor::MakeCube(string path,float xyz[3],float radius) {
    ref_ptr<Geode> sd = new Geode;
    sd->addDrawable(new ShapeDrawable(new Box(Vec3(xyz[0],xyz[1],xyz[2]), 2 * radius)));
    return writeNodeFile(*sd,path);
}

bool Editor::MakeSphere(string path,float xyz[3],float radius) {
    ref_ptr<Geode> sd = new Geode;
    sd->addDrawable(new ShapeDrawable(new Sphere(Vec3(xyz[0],xyz[1],xyz[2]), radius)));
    return writeNodeFile(*sd,path);
}

bool Editor::MakeBase(string path,float xyz[3],int length,int interval) {
    ref_ptr<Geode> sd = new Geode;

    ref_ptr<Geometry> lines = new osg::Geometry();
    auto v_lines = new osg::Vec3Array();
    for(int i=0;i < length + 1;++i) {
        v_lines->push_back(osg::Vec3(xyz[0], xyz[1] + i * interval, xyz[2]));
        v_lines->push_back(osg::Vec3(xyz[0] + length * interval, xyz[1] + i * interval, xyz[2]));
        lines->addPrimitiveSet( new DrawArrays(PrimitiveSet::LINES, i * 4, 2));

        v_lines->push_back(osg::Vec3(xyz[0] + i * interval, xyz[1], xyz[2]));
        v_lines->push_back(osg::Vec3(xyz[0] + i * interval, xyz[1] + length * interval, xyz[2]));
        lines->addPrimitiveSet( new DrawArrays(PrimitiveSet::LINES, i * 4 + 2, 2));
    }

    lines->setVertexArray(v_lines);
    sd->addDrawable(lines);

    ref_ptr<Geometry> square = new osg::Geometry();

    auto v_square = new Vec3Array();
    v_square->push_back(Vec3(xyz[0],xyz[1],xyz[2]));
    v_square->push_back(Vec3(xyz[0] + length * interval,xyz[1],xyz[2]));
    v_square->push_back(Vec3(xyz[0] + length * interval,xyz[1] + length * interval,xyz[2]));
    v_square->push_back(Vec3(xyz[0],xyz[1] + length * interval,xyz[2]));
    square->setVertexArray(v_square);
    square->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS,0,4));

    auto colors = new Vec4Array;
    colors->push_back(Vec4(1.0f,1.0f,1.0f,0.1f)); // white
    square->setColorArray(colors, osg::Array::BIND_PER_PRIMITIVE_SET);

    auto ss = square->getOrCreateStateSet();
    auto blendFunc = new osg::BlendFunc();
    blendFunc->setSource(osg::BlendFunc::SRC_ALPHA);
    blendFunc->setDestination(osg::BlendFunc::ONE_MINUS_CONSTANT_ALPHA);
    ss->setAttributeAndModes(blendFunc);

    sd->addDrawable(square);
    sd->setName(path);
    return writeNodeFile(*sd,path);
}
}
