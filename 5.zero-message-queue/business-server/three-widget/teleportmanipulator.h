#ifndef TELEPORTMANIPULATOR_H
#define TELEPORTMANIPULATOR_H

#include <osgGA/FirstPersonManipulator>
using osgGA::FirstPersonManipulator;
#include <osgGA/EventQueue>
using osgGA::EventQueue;
#include <osg/Vec3>
using osg::Vec3;

class TeleportManipulator : public FirstPersonManipulator
{
public:
    TeleportManipulator();
protected:
    virtual bool performMovementLeftMouseButton( const double /*eventTimeDelta*/, const double dx, const double dy );
    virtual bool performMovementRightMouseButton( const double /*eventTimeDelta*/, const double dx, const double dy );
    virtual bool handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
    virtual bool handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
    virtual bool handleMousePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
};

#endif // TELEPORTMANIPULATOR_H
