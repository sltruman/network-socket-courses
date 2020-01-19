#ifndef THREEMANIPULATOR_H
#define THREEMANIPULATOR_H

#include <osgGA/OrbitManipulator>
using osgGA::OrbitManipulator;
#include <osgGA/EventQueue>
using osgGA::EventQueue;
#include <osg/Vec3>
using osg::Vec3;

class ThreeManipulator : public OrbitManipulator
{
public:
    ThreeManipulator();
    void teleport(EventQueue* eq,Vec3 center,float distance);
protected:
    virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
    virtual bool handleMousePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
    virtual bool performMouseDeltaMovement( const float dx, const float dy );
    virtual bool performMovementLeftMouseButton( const double eventTimeDelta, const double dx, const double dy );
    virtual bool performMovementRightMouseButton( const double eventTimeDelta, const double dx, const double dy );

    bool startAnimationByCenterAndDistance(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
    virtual bool startAnimationByMousePointerIntersection(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
    virtual void applyAnimationStep( const double currentProgress, const double prevProgress );

    struct ThreeAnimationData : public AnimationData {
        const Referenced* userData;
        osg::Vec3d centerMovement;
        float distanceMovement;

        ThreeAnimationData(const Referenced* userData):userData(userData) {}
        void start( const osg::Vec3d& movement,float distanceMovement, const double startTime );
    };
};

#endif // DEFAULTMANIPULATOR_H
