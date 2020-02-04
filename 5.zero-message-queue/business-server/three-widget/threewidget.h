#ifndef THREEWIDGET_H
#define THREEWIDGET_H

#include "threemanipulator.h"

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

#if defined(QOPENGLWIDGET)
#include <QOpenGLWidget>
#else
#include <QGLWidget>
#endif


namespace ThreeQt {

#if defined(QOPENGLWIDGET)
class ThreeWidget : public QOpenGLWidget
#else
class ThreeWidget : public QGLWidget
#endif
{
public:
    ThreeWidget(QWidget* parent = NULL);
    virtual ~ThreeWidget() {}
    void watermark();
    unsigned int mouseLeft,mouseMid,mouseRight;
    unsigned int mouseWheelUp,mouseWheelDown;

protected:
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
    ref_ptr<ThreeManipulator> tm;
    ref_ptr<Group> initScene;
    ref_ptr<Group> worldScene;
    bool frameHandle;
};

}
#endif // THREEWIDGET_H

