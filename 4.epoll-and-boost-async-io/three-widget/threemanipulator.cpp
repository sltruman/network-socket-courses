#include "threemanipulator.h"

#include <osg/Vec3>
using namespace osg;

#include <osgUtil/LineSegmentIntersector>
using namespace osgUtil;

#include <osgViewer/View>
using namespace osgViewer;

#include <osgGA/GUIEventAdapter>
using namespace osgGA;
#include <iostream>

struct UserData_LookingFor : public Referenced {
    UserData_LookingFor(Vec3 center,float distance):center(center),distance(distance) {}
    Vec3 center;
    float distance;
};

ThreeManipulator::ThreeManipulator() : OrbitManipulator(DEFAULT_SETTINGS) {}

void OrbitManipulator::OrbitAnimationData::start( const osg::Vec3d& movement, const double startTime )
{
    AnimationData::start( startTime );
    _movement = movement;
}

void ThreeManipulator::ThreeAnimationData::start( const osg::Vec3d& centerMovement,float distanceMovement, const double startTime )
{
    AnimationData::start( startTime );
    this->centerMovement = centerMovement;
    this->distanceMovement = distanceMovement;
}

void ThreeManipulator::teleport(EventQueue* eq,Vec3 center,float distance) {
    eq->userEvent(new UserData_LookingFor(center,distance));
}

bool ThreeManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) {
    switch(ea.getEventType()) {
    case GUIEventAdapter::USER: {
        if(NULL != dynamic_cast<const UserData_LookingFor*>(ea.getUserData())) {
            _animationData = new ThreeAnimationData(ea.getUserData());
            setAnimationTime(0.25);
            startAnimationByCenterAndDistance(ea,us);
        }
        return true;}
    }

    return OrbitManipulator::handle(ea,us);
}

bool ThreeManipulator::handleMousePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) {
    switch(ea.getButtonMask()) {
    case GUIEventAdapter::LEFT_MOUSE_BUTTON:
        allocAnimationData();
        setAnimationTime(0.25);
        startAnimationByMousePointerIntersection(ea,us);
        return true;
    }

    return OrbitManipulator::handleMousePush(ea,us);
}

bool ThreeManipulator::performMouseDeltaMovement( const float dx, const float dy ) {
    return false;
}

bool ThreeManipulator::performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy ) {
    return false;
}

// doc in parent
bool ThreeManipulator::performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy )
{
    // rotate camera
    if( getVerticalAxisFixed() )
        rotateWithFixedVertical( dx, dy );
    else
        rotateTrackball( _ga_t0->getXnormalized(), _ga_t0->getYnormalized(),
                         _ga_t1->getXnormalized(), _ga_t1->getYnormalized(),
                         getThrowScale( eventTimeDelta ) );
    return true;
}


bool ThreeManipulator::startAnimationByCenterAndDistance(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    // get current transformation
    osg::Vec3d prevCenter, prevEye, prevUp;
    getTransformation( prevEye, prevCenter, prevUp );
    float prevDistance = getDistance();

    ThreeAnimationData *ad = dynamic_cast< ThreeAnimationData*>( _animationData.get() );
    const UserData_LookingFor* ud= dynamic_cast<const UserData_LookingFor*>(ad->userData);

    // setup animation data and restore original transformation
    ad->start(ud->center - prevCenter,ud->distance - prevDistance, ea.getTime() );
    setTransformation( prevEye, prevCenter, prevUp );
    return true;
}

bool ThreeManipulator::startAnimationByMousePointerIntersection(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    // get current transformation
    osg::Vec3d prevCenter, prevEye, prevUp;
    getTransformation( prevEye, prevCenter, prevUp );

    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&us);
    LineSegmentIntersector::Intersections lis;
    // center by mouse intersection
    if( !view->computeIntersections(ea.getX(),ea.getY(),lis,0x2))
        return false;

    _center = lis.begin()->getWorldIntersectPoint();

    OrbitAnimationData *ad = dynamic_cast< OrbitAnimationData*>( _animationData.get() );

    // setup animation data and restore original transformation
    ad->start( osg::Vec3d(_center) - prevCenter, ea.getTime() );
    setTransformation( prevEye, prevCenter, prevUp );
    return true;
}

void ThreeManipulator::applyAnimationStep( const double currentProgress, const double prevProgress )
{
    ThreeAnimationData *tad = dynamic_cast< ThreeAnimationData* >( _animationData.get() );
    OrbitAnimationData *ad = dynamic_cast< OrbitAnimationData* >( _animationData.get() );

    if(tad != NULL) {
        // compute new center
        osg::Vec3d prevCenter, prevEye, prevUp;
        getTransformation( prevEye, prevCenter, prevUp );
        float prevDistance = getDistance();
        osg::Vec3d newCenter = osg::Vec3d(prevCenter) + (tad->centerMovement * (currentProgress - prevProgress));
        float newDistance = prevDistance + (tad->distanceMovement * (currentProgress - prevProgress));

        _distance = newDistance;
        _center = newCenter;
    }

    if(ad != NULL) {
        // compute new center
        osg::Vec3d prevCenter, prevEye, prevUp;
        getTransformation( prevEye, prevCenter, prevUp );
        osg::Vec3d newCenter = osg::Vec3d(prevCenter) + (ad->_movement * (currentProgress - prevProgress));
        osg::Vec3d newEye = osg::Vec3d(prevEye) + (ad->_movement * (currentProgress - prevProgress));

        _distance = (prevEye - newCenter).length();

       // apply new transformation
       setTransformation( newEye, newCenter, prevUp );
   }
}
