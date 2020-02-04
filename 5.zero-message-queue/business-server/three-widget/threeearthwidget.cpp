#if defined(OSGEARTH)
#include "threeearthwidget.h"

#include <osgDB/Registry>
using namespace osgDB;
#include <osgViewer/ViewerBase>
using namespace osgViewer;
#include <osg/Viewport>
using namespace osg;
#include <osgText/Text>
using namespace osgText;
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>

#include <osgEarthUtil/LatLongFormatter>
#include <osgEarthUtil/Controls>
#include <osgEarthUtil/MouseCoordsTool>


#include <iostream>
using namespace std;

#include <QDir>
#include <QSettings>
#include <QMouseEvent>
#include <QKeyEvent>


#if defined(QOPENGLWIDGET)
ThreeEarthWidget::ThreeEarthWidget(QWidget* parent) : QOpenGLWidget (parent)
#else
ThreeEarthWidget::ThreeEarthWidget(QWidget* parent) : QGLWidget (parent)
#endif
  , gw(new GraphicsWindowEmbedded(0, 0,width(),height()))
  , viewer(nullptr)
  , initScene(new Group())
  , worldScene(new Group())
  , frameHandle(false)
{
#if defined(QOPENGLWIDGET)
    setUpdateBehavior(QOpenGLWidget::UpdateBehavior::PartialUpdate);
#endif
    // load an earth file, and support all or our example command-line options
    // and earth file <external> tags
    QSettings settings(GLOBAL_COMPANY,GLOBAL_NAME);
    QString modelDirectoryPath = settings.value(GLOBAL_MODELDIRECTORYPATH).toString();
    QString earthPath = QDir::currentPath() + '/' + modelDirectoryPath + "/main.earth";
    QByteArray earthPathStr = earthPath.toLocal8Bit();
    osgDB::Registry::instance()->getDataFilePathList().push_back(modelDirectoryPath.toLocal8Bit().data());

    cout << "earth file:" << earthPathStr.data() << endl;
    int argc = 2;
    char* argv[] = {"three-earth-test", earthPathStr.data(), "--sky"};
    osg::ArgumentParser arguments(&argc,argv);
    viewer = new Viewer(arguments);

    Camera* camera = viewer->getCamera();
    camera->setGraphicsContext(gw);
    camera->setViewport(new Viewport(0, 0,width(),height()));
    camera->setProjectionMatrixAsPerspective(35, 1. * width() / height(), 1, 1000.0);
    camera->setSmallFeatureCullingPixelSize(-1.0f);

    om = new EarthManipulator();
    om->setHomeViewpoint(osgEarth::Viewpoint("init viewpoint", 110.0, 30.0, 0.0, 0.0, -90.0, 8000000.0 ));
    viewer->setThreadingModel(ViewerBase::AutomaticSelection);
    viewer->setCameraManipulator(om);

    osg::Node* node = MapNodeHelper().load(arguments, viewer );
    osgEarth::MapNode* mapNode = osgEarth::MapNode::findMapNode(node);
    worldScene->addChild(node);
    initScene->addChild(worldScene);

    viewer->getDatabasePager()->setUnrefImageDataAfterApplyPolicy( false, false );
    viewer->setSceneData(initScene);

    om->getSettings()->bindMouse(EarthManipulator::ACTION_ROTATE,osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON);
    om->getSettings()->bindMouse(EarthManipulator::ACTION_PAN,osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON);
    om->getSettings()->bindMouseClick(EarthManipulator::ACTION_GOTO,osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
    //om->getSettings()->bindKey(EarthManipulator::ACTION_HOME,osgGA::GUIEventAdapter::KEY_Space);
}

void ThreeEarthWidget::models() {
    osgEarth::MapNode* mapNode = osgEarth::MapNode::findMapNode(worldScene);

    class InfoVisitor: public osg::NodeVisitor
    {
    public:
        InfoVisitor()
            :osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), _indent(0)
        {}

        virtual void apply(osg::Node& node)
        {
            for(int i = 0; i < _indent; i++)  cout << "    ";
            cout << "[" << _indent << "]"<< node.libraryName()
                   << "::" << node.className() << " " << node.getName() << endl;

            _indent++;
            traverse(node);
            _indent--;

           for(int i = 0; i < _indent; i++)  cout << "    ";
            cout << "[" << _indent << "] "<< node.libraryName()
                   << "::" << node.className() << endl;
        }

        virtual void apply(osg::Geode& node)
        {
            for(int i = 0; i < _indent; i++)  cout << "    ";
            cout << "[" << _indent << "] "<< node.libraryName()
                 << "::" << node.className() << " " << node.getName() << endl;

            _indent++;

            for(unsigned int n = 0; n < node.getNumDrawables(); n++)
            {
                osg::Drawable* draw = node.getDrawable(n);
                if(!draw)
                    continue;
                for(int i = 0; i <  _indent; i++)  cout << "    ";
                cout << "[" << _indent << "]" << draw->libraryName() << "::"
                       << draw->className() << " " << node.getName() << endl;
            }

            traverse(node);
            _indent--;

            for(int i = 0; i < _indent; i++)  cout << "    ";
            cout << "[" << _indent << "]"<< node.libraryName()
                    << "::" << node.className() << " " << node.getName() << endl;
        }
    private:
        int _indent;
    }nv;

    mapNode->accept(nv);

    for(auto ex : mapNode->getExtensions()) {
        cout << ex->className() << endl;
    }
}

void ThreeEarthWidget::resizeGL(int width, int height)
{
    gw->getEventQueue()->windowResize(0,0,width,height);
    gw->resized(0,0,width,height);
}

void ThreeEarthWidget::paintGL()
{
    if(frameHandle) return;
    if(viewer.valid()) viewer->frame();
    update();
}

void ThreeEarthWidget::mousePressEvent(QMouseEvent *ev)
{
    gw->getEventQueue()->mouseButtonPress(ev->x(), ev->y(), ev->button() == Qt::RightButton ? 3 : ev->button() == Qt::MidButton ? 2 : ev->button() == Qt::LeftButton ? 1 : 0);
}

void ThreeEarthWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    gw->getEventQueue()->mouseButtonRelease(ev->x(), ev->y(), ev->button() == Qt::RightButton ? 3 : ev->button() == Qt::MidButton ? 2 : ev->button() == Qt::LeftButton ? 1 : 0);
}

void ThreeEarthWidget::mouseMoveEvent(QMouseEvent *ev)
{
    gw->getEventQueue()->mouseMotion(ev->x(), ev->y());
}

void ThreeEarthWidget::wheelEvent(QWheelEvent *ev)
{
    gw->getEventQueue()->mouseScroll(ev->delta() < 0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN);
}

void ThreeEarthWidget::keyPressEvent(QKeyEvent* ev)
{
    if(!ev->isAutoRepeat()) gw->getEventQueue()->keyPress((osgGA::GUIEventAdapter::KeySymbol) ev->key());
}

void ThreeEarthWidget::keyReleaseEvent(QKeyEvent* ev)
{
    if(!ev->isAutoRepeat()) gw->getEventQueue()->keyRelease((osgGA::GUIEventAdapter::KeySymbol) ev->key());
}
#endif
