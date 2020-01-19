#include "animations.h"
#include "threewidget.h"

#include <osg/MatrixTransform>
#include <osg/NodeVisitor>
using namespace osg;

#include <osgDB/ReadFile>
using namespace osgDB;
#include <osgText/Text>
using namespace osgText;
#include <osgAnimation/Animation>
#include <osgAnimation/BasicAnimationManager>
using namespace osgAnimation;

#include <set>
using namespace std;

#include <QSettings>

namespace ThreeQt {
    struct Animations::Plugin {
        ThreeWidget* tw;
        ref_ptr<MatrixTransform> mtScene;
        ref_ptr<Node> scene;
    };

    struct Animations::AnimationVisitor : public NodeVisitor {
        BasicAnimationManager* am;
        AnimationVisitor() : NodeVisitor(TRAVERSE_ALL_CHILDREN) {}
        virtual void apply(Node& node) {
            am = dynamic_cast<BasicAnimationManager*>(node.getUpdateCallback());
            if(NULL != am) return;
            traverse(node);
        }
    };

    Animations::Animations(ThreeWidget* tw) {
        md = new Plugin;
        md->tw = tw;
    }

    Animations::~Animations() {
        delete md;
    }

    void Animations::TestAnimations(bool enable) {
        AnimationVisitor av;
        md->tw->worldScene->accept(av);
        cout << "[Animations] 动画管理器：" << av.am << endl;
        if(av.am == NULL) return;
        osgAnimation::AnimationList list = av.am->getAnimationList();
        for(osgAnimation::AnimationList::iterator i=list.begin();list.end() != i;i++) {
            if(enable) av.am->playAnimation(i->get());
            else {
                av.am->update(0);
                av.am->stopAnimation(i->get());
            }
        }
    }

    void Animations::TestAnimation(string animationName,bool enable) {
        AnimationVisitor av;
        md->tw->worldScene->accept(av);
        if(NULL == av.am) return;
        osgAnimation::AnimationList list = av.am->getAnimationList();
        ref_ptr<Animation> namedAnimation = new Animation;
        namedAnimation->setName(animationName);
        for(osgAnimation::AnimationList::iterator it=list.begin();list.end() != it;it++) {
            ref_ptr<osgAnimation::Animation> ans = *it;
            osgAnimation::ChannelList channels = ans->getChannels();
            for(osgAnimation::ChannelList::iterator j =channels.begin();channels.end() != j;j++) {
                ref_ptr<osgAnimation::Channel> channel = *j;
                if(animationName == channel->getTargetName())
                {
                    namedAnimation->addChannel(channel);
                }
            }
        }

      //  namedAnimation->setPlayMode(osgAnimation::Animation::STAY);
      //  namedAnimation->setWeight(1.0);
      //  namedAnimation->setStartTime(0);

        if(enable)
        {
            av.am->registerAnimation(namedAnimation);
            av.am->playAnimation(namedAnimation);
            av.am->unregisterAnimation(namedAnimation);
        }
        else{
           av.am->stopAll();
        }

    }

    void Animations::SetModelState(string name,double value) {
        AnimationVisitor av;
        md->tw->worldScene->accept(av);
        if(NULL == av.am) return;

        osgAnimation::AnimationList list = av.am->getAnimationList();
        for(osgAnimation::AnimationList::iterator i=list.begin();list.end() != i;i++) {
            if(i->get()->getName() != name) continue;
            i->get()->update(value);
        }

        ref_ptr<Animation> namedAnimation = new Animation;
        namedAnimation->setName(name);

        for(osgAnimation::AnimationList::iterator it=list.begin();list.end() != it;it++) {
            ref_ptr<osgAnimation::Animation> ans = *it;
            osgAnimation::ChannelList channels = ans->getChannels();
            for(osgAnimation::ChannelList::iterator j =channels.begin();channels.end() != j;j++) {
                ref_ptr<osgAnimation::Channel> channel = *j;
                if(name != channel->getTargetName()) continue;
                namedAnimation->addChannel(channel);
            }
        }

        namedAnimation->setWeight(1.0);
        namedAnimation->setStartTime(0);
        namedAnimation->update(value);
    }

    set<string> Animations::AnimationNames() {
        AnimationVisitor av;
        md->tw->worldScene->accept(av);

        set<string> names;
        if(NULL == av.am) return names;

        osgAnimation::AnimationList animations = av.am->getAnimationList();
        for(osgAnimation::AnimationList::iterator i=animations.begin();animations.end()!=i;i++) {
            osgAnimation::Animation* a=*i;
            osgAnimation::ChannelList channels = a->getChannels();
            for(osgAnimation::ChannelList::iterator j=channels.begin();channels.end()!=j;j++) {
                osgAnimation::Channel* c = *j;
                std::string name = c->getTargetName();
                if(names.end() != names.find(name)) continue;
                names.insert(name);
                std::cout << "动画：" << name << std::endl;
            }
        }

        return names;
    }
}
