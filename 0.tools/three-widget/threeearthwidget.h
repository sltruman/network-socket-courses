#ifndef THREEEARTHWIDGET_H
#define THREEEARTHWIDGET_H

#if defined(OSGEARTH)

#include <osg/Timer>
using osg::Timer;
#include <osg/ref_ptr>
using osg::ref_ptr;
#include <osg/Group>
using osg::Group;
#include <osg/MatrixTransform>
using osg::MatrixTransform;
#include <osg/Node>
using osg::Node;
#include <osg/StateSet>
using osg::StateSet;
#include <osg/Program>
using osg::Program;
#include <osgViewer/GraphicsWindow>
using osgViewer::GraphicsWindowEmbedded;
#include <osgViewer/Viewer>
using osgViewer::Viewer;

#include <osgEarth/Notify>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ExampleResources>
using namespace osgEarth;
using namespace osgEarth::Util;

#if defined(QOPENGLWIDGET)
#include <QOpenGLWidget>
class ThreeEarthWidget : public QOpenGLWidget
#else
#include <QGLWidget>
class ThreeEarthWidget : public QGLWidget
#endif
{
public:
    ThreeEarthWidget(QWidget* parent = NULL);
    void models();

    virtual void resizeGL(int,int);
    virtual void paintGL();
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void wheelEvent(QWheelEvent*);

public:
    ref_ptr<GraphicsWindowEmbedded> gw;
    ref_ptr<Viewer> viewer;
    ref_ptr<EarthManipulator> om;
    ref_ptr<Group> initScene;
    ref_ptr<Group> worldScene;
    bool frameHandle;
};

#endif
#endif // THREEEARTHWIDGET_H

