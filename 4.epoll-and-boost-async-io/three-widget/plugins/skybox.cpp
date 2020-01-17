#include "skybox.h"
#include "threewidget.h"

#include <osgViewer/Viewer>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Quat>
#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Material>
#include <osg/NodeCallback>
#include <osg/Depth>
#include <osg/CullFace>
#include <osg/TexMat>
#include <osg/TexGen>
#include <osg/TexEnv>
#include <osg/TextureCubeMap>
#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/ComputeBoundsVisitor>
#include <osgUtil/Optimizer>

using namespace std;
using namespace osg;

//读取立方图

namespace  ThreeQt {
    osg::ref_ptr<osg::TextureCubeMap> readCubeMap(string dirPath)
    {

        osg::ref_ptr<osg::TextureCubeMap> cubemap = new osg::TextureCubeMap;

        osg::ref_ptr<osg::Image> imagePosX = osgDB::readImageFile(dirPath + "front.jpg");
        osg::ref_ptr<osg::Image> imageNegX = osgDB::readImageFile(dirPath + "back.jpg");
        osg::ref_ptr<osg::Image> imagePosY = osgDB::readImageFile(dirPath + "down.jpg");
        osg::ref_ptr<osg::Image> imageNegY = osgDB::readImageFile(dirPath + "up.jpg");
        osg::ref_ptr<osg::Image> imagePosZ = osgDB::readImageFile(dirPath + "right.jpg");
        osg::ref_ptr<osg::Image> imageNegZ = osgDB::readImageFile(dirPath + "left.jpg");

        if(imageNegX.get() && imagePosX.get() && imagePosY.get() && imageNegY.get() && imagePosZ.get() && imageNegZ.get())
        {
            //设置立方体的6个面的贴图
            cubemap->setImage(osg::TextureCubeMap::POSITIVE_X,imagePosX.get());
            cubemap->setImage(osg::TextureCubeMap::NEGATIVE_X,imageNegX.get());
            cubemap->setImage(osg::TextureCubeMap::POSITIVE_Y,imagePosY.get());
            cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Y,imageNegY.get());
            cubemap->setImage(osg::TextureCubeMap::POSITIVE_Z,imagePosZ.get());
            cubemap->setImage(osg::TextureCubeMap::NEGATIVE_Z,imageNegZ.get());

            //设置纹理环绕模式
            cubemap->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
            cubemap->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
            cubemap->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);

            //设置滤波：线形和mipmap
            cubemap->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
            cubemap->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
        }
        return  cubemap.get();
    }

    struct SkyBox::Plugin {
        ThreeWidget* tw;
        osg::ref_ptr<osg::ClearNode> skyNode;
    };


    struct SkyBox::TexMatCallback:public osg::NodeCallback
    {
    public:
        TexMatCallback(osg::TexMat & tm):_texMat(tm)
        {}
        virtual void operator()(osg::Node * node,osg::NodeVisitor * nv)
        {
            osgUtil::CullVisitor * cv = dynamic_cast<osgUtil::CullVisitor *>(nv);
            if(cv)
            {
                //得到模型视图矩阵设置旋转角度
                const osg::Matrix & MV = *(cv->getModelViewMatrix());
                const osg::Matrix R =osg::Matrix::rotate(osg::DegreesToRadians(112.0f),0.0f,0.0f,1.0f) *
                                        osg::Matrix::rotate(osg::DegreesToRadians(90.0f),1.0f,0.0f,0.0f);
                osg::Quat q = MV.getRotate();
                const osg::Matrix C = osg::Matrix::rotate(q.inverse());

                //设置纹理矩阵
                _texMat.setMatrix(C * R);
            }
            traverse(node,nv);
        }
        //纹理矩阵
        osg::TexMat & _texMat;
    };

    //一个变换类，使天空盒绕视点旋转
    class SkyBox::MoveEarthySkyWithEyePointTransform:public osg::Transform
    {
    public:
        //局部矩阵计算成世界矩阵
        virtual bool computeLocalToWorldMatrix(osg::Matrix & matrix,osg::NodeVisitor *nv)const
        {
            osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if(cv)
            {
                osg::Vec3 eyePointLocal = cv->getEyeLocal();
                matrix.preMult(osg::Matrix::translate(eyePointLocal));
            }
            return  true;
        }
        //世界矩阵计算成局部矩阵
        virtual bool computeWorldToLocaleMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv)const
        {
            osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if(cv)
            {
                osg::Vec3 eyePointLocal = cv->getEyeLocal();
                matrix.postMult(osg::Matrix::translate(-eyePointLocal));
            }
            return  true;
        }
    };

    SkyBox::SkyBox(ThreeWidget* tw) {
        md = new Plugin();
        md->tw = tw;
    }

    SkyBox::~SkyBox() {
        md->tw->initScene->removeChild(md->skyNode);
        delete md;
    }

    //创建天空盒
    void SkyBox::CreateSkyBox(string dirPath)
    {
        osg::ref_ptr<osg::StateSet>stateset = new osg::StateSet();

        //设置纹理映射方式，指定为替代方式，即纹理中的颜色代替原来的颜色
        osg::ref_ptr<osg::TexEnv> te = new osg::TexEnv;
        te->setMode(osg::TexEnv::REPLACE);
        stateset->setTextureAttributeAndModes(0,te.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        //自动生成纹理坐标，反射方式(REFLECTION_MAP)
        /*
         * NORMAL_MAP 标准模式-立方图纹理
         * REFLECTION_MAP 反射模式-球体纹理
         * SPHERE_MAP 球体模型-球体纹理
         */
        osg::ref_ptr<osg::TexGen> tg = new osg::TexGen;
        tg->setMode(osg::TexGen::NORMAL_MAP);
        stateset->setTextureAttributeAndModes(0,tg.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        //设置纹理矩阵
        osg::ref_ptr<osg::TexMat> tm = new osg::TexMat;
        stateset->setTextureAttribute(0,tm.get());


        //设置立方图纹理
        osg::ref_ptr<osg::TextureCubeMap>skymap = readCubeMap(dirPath);
        stateset->setTextureAttributeAndModes(0,skymap.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        stateset->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
        //将深度设置为远平面
        osg::ref_ptr<osg::Depth> depth = new osg::Depth;
        depth->setFunction(osg::Depth::ALWAYS);
        depth->setRange(1.0,1.0); //远平面
        stateset->setAttributeAndModes(depth,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        //设置渲染顺序为-1，先渲染
        stateset->setRenderBinDetails(-1,"RenderBin");
       BoundingSphere bound = md->tw->worldScene->getBound();
       Vec3 bc = bound.center();
       float br = bound.radius();
       md->tw->tm->setHomePosition(Vec3(br,-br,br) + bc,bc,Vec3(0,0,1));
       md->tw->tm->home(0);

//        ref_ptr<MatrixTransform> mt = new MatrixTransform();
//        osg::Matrix m;
//        m.makeTranslate(bc.x(),bc.y(),bc.z()-br);
//        mt->setMatrix(m);
//        mt->addChild(p->tw->worldScene);
        osg::ref_ptr<osg::Drawable> drawable = new osg::ShapeDrawable(new osg::Sphere(bc,br * 1.5 ));
        //把球体加入到叶节点
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->setCullingActive(false);
        geode->setStateSet(stateset.get());
        geode->addDrawable(drawable.get());
        //设置变换
        osg::ref_ptr<osg::Transform> transform = new MoveEarthySkyWithEyePointTransform();
        transform->setCullingActive(false);
        transform->addChild(geode.get());

        osg::ref_ptr<osg::ClearNode> skyNode = new osg::ClearNode;
        skyNode->setCullCallback(new TexMatCallback(*tm));
        skyNode->addChild(transform.get());
        skyNode->setNodeMask(0x1 << 31);
        md->tw->initScene->addChild(skyNode.get());
     //   return  clearNode.get();
    }

}
