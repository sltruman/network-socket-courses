#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include "json/reader.h"
#include "json/value.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tw = CreateThreeWidget();
    reinterpret_cast<QWidget*>(tw)->setFocusPolicy(Qt::ClickFocus);
    ui->verticalLayout_3->replaceWidget(ui->widget_opengl,reinterpret_cast<QWidget*>(tw));

    bp = new Basic(tw);
    ep = new Editor(tw);
    sp = new Shadow(tw);
    sp->Lighting(true);

    xyz[0] = 0,xyz[1] = 0,xyz[2] = 0;
    ep->MakeCube("./cube.osg",xyz,0.3);
    float basePos[3]{-0.5f,-0.5f,0.f};
    ep->MakeBase("./base.osg",basePos,100);
    bp->OpenScene("./base.osg");

    context = zmq_ctx_new();
    sock = zmq_socket(context,ZMQ_REQ);
    //zmq_connect(sock,"tcp://47.75.207.201:27017");
    zmq_connect(sock,"tcp://127.0.0.1:27017");

    connect(&t,SIGNAL(timeout()),SLOT(syncStatus()));
    t.setInterval(1000);
    t.start();
}

MainWindow::~MainWindow()
{
    t.stop();
    delete (bp);
    delete (ep);
    delete (sp);
    delete ui;
}

void MainWindow::syncStatus() {
    QApplication::processEvents();
    string req = R"({ "type":"get" })",res(64,'\0');

    REQ:
    zmq_send(sock,req.data(),req.size(),0);

    int size = 0;
    while(true) {
        QApplication::processEvents();
        size = zmq_recv(sock,const_cast<char*>(res.data()),res.size(),ZMQ_DONTWAIT);
        if(size > 0) {
            break;
        }
    }

    std::cout << res << std::endl;
    if(size > res.size()) {
        res.resize(size);
        goto REQ;
    }

    Json::Reader reader;
    Json::Value resJson;
    reader.parse(res,resJson);

    if(!resJson["error"].isNull()) return;
    auto retJson = resJson["ret"];

    for(auto it=retJson.begin();it != retJson.end();it++) {
        auto id = it.key().asString();
        auto steps = (*it).asInt();
        vec3 pos(3);

        auto runner = runners.find(id.data());
        if(runner == runners.end()) {
            pos[0] = xyz[0],pos[1] = 0.1f * steps,pos[2]=0.3;
            ep->AddModel("./cube.osg",id);
            xyz[0]++;
            std::cout << id << std::endl;
        } else {
            pos[0] = get<0>(runner.value());
            pos[1] = 0.1f * steps;
            pos[2] = get<2>(runner.value());
        }

        runners.insert(id.data(),make_tuple(pos[0],pos[1],pos[2]));
        ep->ModelTrans(id,pos);
    }

    bp->Bubbles(true);
}
