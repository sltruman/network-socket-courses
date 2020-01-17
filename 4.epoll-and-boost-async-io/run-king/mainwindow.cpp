#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tw = CreateThreeWidget();
    reinterpret_cast<QWidget*>(tw)->setFocusPolicy(Qt::ClickFocus);
    ui->verticalLayout->replaceWidget(ui->widget_opengl,reinterpret_cast<QWidget*>(tw));

    bp = new Basic(tw);
    ep = new Editor(tw);
    sp = new Shadow(tw);
    sp->Lighting(true);

    xyz[0] = 0,xyz[1] = 0,xyz[2] = 0;
    ep->MakeCube("./cube.osg",xyz,0.3);
    float basePos[3]{-0.5f,-0.5f,0.f};
    ep->MakeBase("./base.osg",basePos,100);
    bp->OpenScene("./base.osg");
}

MainWindow::~MainWindow()
{
    delete (bp);
    delete (ep);
    delete (sp);
    delete ui;
}

enum msg_type { run,peek };

struct msg {
    msg_type flag;
    char id[24];
    unsigned int steps;
};

bool MainWindow::send(QTcpSocket* tcp) {

    while(!tcp->waitForConnected(1000)) {
        if(QAbstractSocket::UnconnectedState == tcp->state()) return false;
        QApplication::processEvents();
    }

    msg req;
    req.flag = msg_type::peek;
    if(-1 == tcp->write(reinterpret_cast<char*>(&req),sizeof(req))) return false;
    return true;
}

bool MainWindow::receive(QTcpSocket *tcp) {
    while(!tcp->waitForReadyRead(1000)) {
        QApplication::processEvents();
        if(QAbstractSocket::UnconnectedState == tcp->state()) return false;
    }

    int runnerNum = 0;

    for(int j=0;j < 4;) {
        auto len = tcp->read(reinterpret_cast<char*>(&runnerNum) + j,4 - j);
        if(len == -1) return false;
        j += len;
        QApplication::processEvents();
    }

    QList<msg> runnerList;
    msg res;
    for(int i=0;i < runnerNum;i++) {
        for(int j=0;j < sizeof(res);) {
            auto len = tcp->read(reinterpret_cast<char*>(&res) + j,sizeof(res) - j);
            if(len == -1) return false;
            j += len;
            QApplication::processEvents();
        }
        runnerList.push_back(res);
    }


    for(auto r : runnerList) {
        vec3 pos(3);

        auto it = runners.find(r.id);
        if(it == runners.end()) {
            pos[0] = xyz[0],pos[1] = 0.1f * r.steps,pos[2]=0.3;
            ep->AddModel("./cube.osg",r.id);
            xyz[0]++;
            std::cout << r.id << std::endl;
        } else {
            pos[0] = get<0>(it.value());
            pos[1] = 0.1f * r.steps;
            pos[2] = get<2>(it.value());
        }

        runners.insert(r.id,make_tuple(pos[0],pos[1],pos[2]));
        ep->ModelTrans(r.id,pos);
    }

    bp->Bubbles(true);
    return true;
}

void MainWindow::on_pushButton_connect_clicked() {
    isDisconnected = false;
    for(auto server : ui->lineEdit_servers->text().trimmed().split(QRegExp("\\s+"))) {
        auto address = server.split(QRegExp(":"));
        auto tcp = new QTcpSocket(this);
        tcp->connectToHost(address[0].trimmed(),address[1].trimmed().toUShort());
        tcpList.push_back(tcp);
    }

    for(auto it = tcpList.begin();!isDisconnected;it++) {
        if(it == tcpList.end()) it = tcpList.begin();
        auto tcp = *it;

        if(!send(tcp) || !receive(tcp)) {
            tcp->reset();
            --tcpList.erase(it);
            delete tcp;
            continue;
        }

        QApplication::processEvents();
    }

    for(auto it = tcpList.begin();it != tcpList.end();it++) {
        auto tcp = *it;

        tcp->disconnectFromHost();
        --tcpList.erase(it);
        delete tcp;

        QApplication::processEvents();
    }
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    isDisconnected = true;
}
