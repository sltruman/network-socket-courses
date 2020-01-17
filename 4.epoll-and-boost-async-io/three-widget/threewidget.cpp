#include "threewidget.h"
#include "threemanipulator.h"

using osgViewer::ViewerBase;

#include <osg/Viewport>
using namespace osg;
#include <osg/Depth>
#include <osgDB/ReadFile>
#include <osgText/Text>
#include <osgUtil/Optimizer>
using namespace osgText;

#include <iostream>
using namespace std;

#include <QSettings>
#include <QMouseEvent>
#include <QKeyEvent>

using namespace ThreeQt;

#if defined(QOPENGLWIDGET)
ThreeWidget::ThreeWidget(QWidget* parent) : QOpenGLWidget (parent)
#else
ThreeWidget::ThreeWidget(QWidget* parent) : QGLWidget (parent)
#endif
  , mouseLeft(1)
  , mouseMid(2)
  , mouseRight(3)
  , mouseWheelUp(osgGA::GUIEventAdapter::SCROLL_UP)
  , mouseWheelDown(osgGA::GUIEventAdapter::SCROLL_DOWN)
  , gw(new GraphicsWindowEmbedded(0, 0,width(),height()))
  , viewer(new Viewer())
  , tm(new ThreeManipulator())
  , initScene(new Group())
  , worldScene(new Group())
  , frameHandle(false)
{
#if defined(QOPENGLWIDGET)
    setUpdateBehavior(QOpenGLWidget::UpdateBehavior::PartialUpdate);
#endif
    Camera* camera = viewer->getCamera();
    camera->setGraphicsContext(gw);
    camera->setViewport(new Viewport(0, 0,width(),height()));
    camera->setProjectionMatrixAsPerspective(35, 1. * width() /height(), 1, 1000.0);
    camera->setClearColor(Vec4(0.05098f,0.160784f,89.0f/255,1));

    tm->setAllowThrow(false);

    viewer->setThreadingModel(ViewerBase::AutomaticSelection);
    viewer->setCameraManipulator(tm,false);
    viewer->setSceneData(initScene);

    LightSource* ls = new LightSource;
    ls->getLight()->setLightNum(0);
    Vec3 pos(0.0,0.0,10000.0);
    ls->getLight()->setPosition(Vec4(pos,0.0));
    Vec3 dir = -pos;
    dir.normalize();
    ls->getLight()->setDirection(dir);
    ls->getLight()->setAmbient(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    ls->getLight()->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    ls->getLight()->setSpecular(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

    initScene->addChild(ls);
    initScene->addChild(worldScene);
    watermark();
}

void ThreeWidget::watermark() {
    QSettings settings(GLOBAL_COMPANY,GLOBAL_NAME);
    QString flag = settings.value("flag").toString();
    QString fontFilePath = settings.value("fontFilePath").toString();

    Font* ff = osgText::readFontFile(fontFilePath.toLocal8Bit().data());
    Text* text = new Text();
    if(ff == NULL)
        text->setText("TEST",String::ENCODING_UTF8);
    else {
        text->setText(flag.toUtf8().data(),String::ENCODING_UTF8);
        text->setFont(ff);
    }
    text->setColor(Vec4(1,1,0,1));
    text->setAlignment(Text::LEFT_BOTTOM);
    Geode* watermark = new Geode();
    watermark->addDrawable(text);

    Camera* camera = new Camera();
    camera->setProjectionMatrixAsOrtho2D(0,1280,0,800);
    camera->setReferenceFrame(Transform::ABSOLUTE_RF);
    camera->setViewMatrix(Matrix::identity());
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(Camera::POST_RENDER);
    camera->setAllowEventFocus(false);
    camera->addChild(watermark);
    camera->setNodeMask(0x1);
    initScene->addChild(camera);
}

void ThreeWidget::resizeGL(int width, int height)
{
    gw->getEventQueue()->windowResize(0,0,width,height);
    gw->resized(0,0,width,height);
}

void ThreeWidget::paintGL()
{
    if(frameHandle) return;
    if(viewer.valid()) viewer->frame();
    update();
}

void ThreeWidget::mousePressEvent(QMouseEvent *ev)
{
    gw->getEventQueue()->mouseButtonPress(ev->x(), ev->y(), ev->button() == Qt::RightButton ? mouseRight : ev->button() == Qt::MidButton ? mouseMid : ev->button() == Qt::LeftButton ? mouseLeft : 0);
}

void ThreeWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    gw->getEventQueue()->mouseButtonRelease(ev->x(), ev->y(), ev->button() == Qt::RightButton ? mouseRight : ev->button() == Qt::MidButton ? mouseMid : ev->button() == Qt::LeftButton ? mouseLeft : 0);
}

void ThreeWidget::mouseMoveEvent(QMouseEvent *ev)
{
    gw->getEventQueue()->mouseMotion(ev->x(), ev->y());
}

void ThreeWidget::wheelEvent(QWheelEvent *ev)
{
    gw->getEventQueue()->mouseScroll(ev->delta() < 0 ? static_cast<osgGA::GUIEventAdapter::ScrollingMotion>(mouseWheelUp) : static_cast<osgGA::GUIEventAdapter::ScrollingMotion>(mouseWheelDown));
}

void ThreeWidget::keyPressEvent(QKeyEvent* ev)
{
    if(!ev->isAutoRepeat()) gw->getEventQueue()->keyPress((osgGA::GUIEventAdapter::KeySymbol) ev->key());
}

void ThreeWidget::keyReleaseEvent(QKeyEvent* ev)
{
    if(!ev->isAutoRepeat()) gw->getEventQueue()->keyRelease((osgGA::GUIEventAdapter::KeySymbol) ev->key());
}
