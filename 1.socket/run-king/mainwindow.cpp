#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    tcp(this)
{
    ui->setupUi(this);
    tw = CreateThreeWidget();
    reinterpret_cast<QWidget*>(tw)->setFocusPolicy(Qt::ClickFocus);
    ui->verticalLayout->insertWidget(0,reinterpret_cast<QWidget*>(tw));
    ui->verticalLayout->removeWidget(ui->widget_opengl);

    bp = Basic::CreateThreePlugin(tw);
    ep = Editor::CreateThreePlugin(tw);
    ap = Animations::CreateThreePlugin(tw);
    sp = Shadow::CreateThreePlugin(tw);
    cp = Controls::CreateThreePlugin(tw);
    op = Observer::CreateThreePlugin(tw);

    xyz[0] = 0,xyz[1] = 0,xyz[2] = 0;
    Editor::MakeSphere("./sphere.osg",xyz,0.3);
    float basePos[3]{-0.5f,-0.5f,0.f};
    Editor::MakeBase("./base.osg",basePos,100);
    Basic::OpenScene(bp,"./base.osg");

    connect(&tcp, SIGNAL(connected()), this, SLOT(peek()));
    connect(&tcp, SIGNAL(readyRead()), this, SLOT(sync()));
    connect(&tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(syncErr(QAbstractSocket::SocketError)));
    //tcp.connectToHost("127.0.0.1",6000);
    tcp.connectToHost("47.75.207.201",6000);
}

MainWindow::~MainWindow()
{
    Basic::DestoryThreePlugin(bp);
    Editor::DestoryThreePlugin(ep);
    Animations::DestoryThreePlugin(ap);
    Shadow::DestoryThreePlugin(sp);
    Controls::DestoryThreePlugin(cp);
    Observer::DestoryThreePlugin(op);
    delete ui;
}

void MainWindow::syncErr(QAbstractSocket::SocketError e) {
    std::cout << "errno:" << e << std::endl;
}


enum msg_type { run,peek };

struct msg {
    msg_type flag;
    char id[24];
    unsigned int steps;
};


void MainWindow::peek() {
    msg r;
    r.flag = msg_type::peek;
    tcp.write((char*)&r,sizeof(msg));
}

void MainWindow::sync() {
    int runnerNum = 0;

    for(int j=0;j < 4;) {
        j += tcp.read(reinterpret_cast<char*>(&runnerNum),4 - j);
        QApplication::processEvents();
    }

    for(int i=0;i<runnerNum;i++) {
        msg r;

        for(int j=0;j < sizeof(msg);) {
            j += tcp.read(reinterpret_cast<char*>(&r) + j,sizeof(msg) - j);
            QApplication::processEvents();
        }

        float pos[3];

        auto it = runners.find(r.id);
        if(it == runners.end()) {
            pos[0] = xyz[0],pos[1] = 0.1f * r.steps,pos[2]=xyz[2];
            Editor::AddModel(ep,"./sphere.osg",r.id);
            xyz[0]++;
            std::cout << i << " " << r.id << std::endl;
        } else {
            pos[0] = get<0>(it.value());
            pos[1] = 0.1f * r.steps;
            pos[2] = get<2>(it.value());
        }

        runners.insert(r.id,make_tuple(pos[0],pos[1],pos[2]));
        Editor::SetModelPos(ep,r.id,pos);
    }

    Basic::Bubbles(bp,true,20);

    peek();
}

