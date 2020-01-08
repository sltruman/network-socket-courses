#include "teleportmanipulator.h"

#include <osg/Vec3>
using namespace osg;

#include <osgUtil/LineSegmentIntersector>
using namespace osgUtil;

#include <osgViewer/View>
using namespace osgViewer;

#include <osgGA/GUIEventAdapter>
using namespace osgGA;

#include <iostream>

struct UserDataForTeleport : public Referenced {
    UserDataForTeleport(Vec3 center,float distance):center(center),distance(distance) {}
    Vec3 center;
    float distance;
};

TeleportManipulator::TeleportManipulator() : FirstPersonManipulator(DEFAULT_SETTINGS) {}

// doc in parent
bool TeleportManipulator::performMovementLeftMouseButton( const double /*eventTimeDelta*/, const double dx, const double dy )
{
    return true;
}

bool TeleportManipulator::performMovementRightMouseButton( const double /*eventTimeDelta*/, const double dx, const double dy )
{
   // world up vector
   CoordinateFrame coordinateFrame = getCoordinateFrame( _eye );
   Vec3d localUp = getUpVector( coordinateFrame );
   rotateYawPitch( _rotation, dx, dy, localUp );
   return true;
}

bool TeleportManipulator::handleMousePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) {
    switch(ea.getButtonMask()) {
    case GUIEventAdapter::LEFT_MOUSE_BUTTON:
        allocAnimationData();
        setAnimationTime(0.25);
        startAnimationByMousePointerIntersection(ea,us);
        return true;
    }

    return FirstPersonManipulator::handleMousePush(ea,us);
}

bool TeleportManipulator::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) {
    std::cout << ea.getKey() << std::endl;
    switch(ea.getKey()) {
        case 87:/*GUIEventAdapter::KEY_W:*/
            moveForward(_wheelMovement * (getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize : 1. ));
            return true;
        case 83:/*GUIEventAdapter::KEY_S:*/
            moveForward(-_wheelMovement * (getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize : 1. ));
            return true;
        case 65:/*GUIEventAdapter::KEY_A:*/
            moveRight(-_wheelMovement * (getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize : 1. ));
            return true;
        case 68:/*GUIEventAdapter::KEY_D:*/
            moveRight(_wheelMovement * (getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize : 1. ));
            return true;
    }
    return false;
}

bool TeleportManipulator::handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us ) {
    switch(ea.getKey()) {

    }
    return false;
}
