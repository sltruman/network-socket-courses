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

void MainWindow::error(QAbstractSocket::SocketError e) {
    std::cout << "errno:" << e << std::endl;

    for(auto tcp : tcpList.keys()) {
        if(!tcp->isValid() || !tcp->waitForConnected(0)) {
            disconnect(tcp, SIGNAL(connected()), this, SLOT(connected()));
            disconnect(tcp, SIGNAL(readyRead()), this, SLOT(readyRead()));
            disconnect(tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
            tcp->disconnectFromHost();
            tcp->reset();
            tcpList.remove(tcp);
            delete tcp;
            QApplication::processEvents();
        }
    }
}

enum msg_type { run,peek };

struct msg {
    msg_type flag;
    char id[24];
    unsigned int steps;
};

void MainWindow::connected() {
    for(auto tcp : tcpList.keys()) {
        auto hasWritten = tcpList[tcp];
        if(!tcp->waitForConnected(0)) continue;
        if(hasWritten) continue;

        msg req;
        req.flag = msg_type::peek;
        tcp->write(reinterpret_cast<char*>(&req),sizeof(req));
        tcpList[tcp] = true;
    }
}

void MainWindow::readyRead() {
    QList<msg> runnerList;
    for(auto tcp : tcpList.keys()) {
        auto hasWritten = tcpList[tcp];
        if(0 == tcp->bytesAvailable()) continue;
        if(!hasWritten) continue;

        int runnerNum = 0;

        for(int j=0;j < 4;) {
            j += tcp->read(reinterpret_cast<char*>(&runnerNum) + j,4 - j);
            QApplication::processEvents();
        }

        msg res;
        for(int i=0;i < runnerNum;i++) {
            for(int j=0;j < sizeof(res);) {
                j += tcp->read(reinterpret_cast<char*>(&res) + j,sizeof(res) - j);
                runnerList.push_back(res);
                QApplication::processEvents();
            }
        }

        tcpList[tcp] = false;
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

    connected();
}


void MainWindow::on_pushButton_connect_clicked()
{
    for(auto server : ui->lineEdit_servers->text().trimmed().split(' ')) {
        auto address = server.split(':');
        auto tcp = new QTcpSocket(this);

        connect(tcp, SIGNAL(connected()), this, SLOT(connected()));
        connect(tcp, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

        tcp->connectToHost(address[0].trimmed(),address[1].trimmed().toUShort());
        tcpList[tcp] = false;
        QApplication::processEvents();
    }
}

void MainWindow::on_pushButton_clicked()
{
    for(auto tcp : tcpList.keys()) {
        disconnect(tcp, SIGNAL(connected()), this, SLOT(connected()));
        disconnect(tcp, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(tcp, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
        tcp->disconnectFromHost();
        tcp->reset();
        tcpList.remove(tcp);
        delete tcp;
        QApplication::processEvents();
    }
}
