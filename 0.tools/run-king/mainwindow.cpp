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

bool MainWindow::send(QTcpSocket& tcp) {
    msg req;
    req.flag = msg_type::peek;
    if(0 < tcp.write(reinterpret_cast<char*>(&req),sizeof(req))) return true;
    return false;
}

bool MainWindow::receive(QTcpSocket& tcp) {
    int runnerNum = 0;

    for(int j=0;j < 4;) {
        auto len = tcp.read(reinterpret_cast<char*>(&runnerNum) + j,4 - j);
        if(len <= 0) return false;
        j += len;
        QApplication::processEvents();
    }

    QList<msg> runnerList;
    msg res;
    for(int i=0;i < runnerNum;i++) {
        for(int j=0;j < sizeof(res);) {
            auto len = tcp.read(reinterpret_cast<char*>(&res) + j,sizeof(res) - j);
            if(len <= 0) return false;
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
    QStringList address;
    if(ui->lineEdit_server0->text() != "") {
        address = ui->lineEdit_server0->text().trimmed().split(':');
        connect(&tcp[0],SIGNAL(connected()),this,SLOT(connected0()));
        connect(&tcp[0],SIGNAL(readyRead()),this,SLOT(readyRead0()));
        connect(&tcp[0],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error0(QAbstractSocket::SocketError)));
        tcp[0].connectToHost(address[0].trimmed(),address[1].trimmed().toUShort());
    }
    if(ui->lineEdit_server1->text() != "") {
        address = ui->lineEdit_server1->text().trimmed().split(':');
        connect(&tcp[1],SIGNAL(connected()),this,SLOT(connected1()));
        connect(&tcp[1],SIGNAL(readyRead()),this,SLOT(readyRead1()));
        connect(&tcp[1],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error1(QAbstractSocket::SocketError)));
        tcp[1].connectToHost(address[0].trimmed(),address[1].trimmed().toUShort());
    }
    if(ui->lineEdit_server2->text() != "") {
        address = ui->lineEdit_server2->text().trimmed().split(':');
        connect(&tcp[2],SIGNAL(connected()),this,SLOT(connected2()));
        connect(&tcp[2],SIGNAL(readyRead()),this,SLOT(readyRead2()));
        connect(&tcp[2],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error2(QAbstractSocket::SocketError)));
        tcp[2].connectToHost(address[0].trimmed(),address[1].trimmed().toUShort());
    }
    if(ui->lineEdit_server3->text() != "") {
        address = ui->lineEdit_server3->text().trimmed().split(':');
        connect(&tcp[3],SIGNAL(connected()),this,SLOT(connected3()));
        connect(&tcp[3],SIGNAL(readyRead()),this,SLOT(readyRead3()));
        connect(&tcp[3],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error3(QAbstractSocket::SocketError)));
        tcp[3].connectToHost(address[0].trimmed(),address[1].trimmed().toUShort());
    }
    if(ui->lineEdit_server4->text() != "") {
        address = ui->lineEdit_server4->text().trimmed().split(':');
        connect(&tcp[4],SIGNAL(connected()),this,SLOT(connected4()));
        connect(&tcp[4],SIGNAL(readyRead()),this,SLOT(readyRead4()));
        connect(&tcp[4],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error4(QAbstractSocket::SocketError)));
        tcp[4].connectToHost(address[0].trimmed(),address[1].trimmed().toUShort());
    }
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    auto address = ui->lineEdit_server0->text().trimmed().split(':');
    if(!address.empty()) {
        tcp[0].reset();
        disconnect(&tcp[0],SIGNAL(connected()),this,SLOT(connected0()));
        disconnect(&tcp[0],SIGNAL(readyRead()),this,SLOT(readyRead0()));
        disconnect(&tcp[0],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error0(QAbstractSocket::SocketError)));
    }
    address = ui->lineEdit_server1->text().trimmed().split(':');
    if(!address.empty()) {
        tcp[1].reset();
        disconnect(&tcp[1],SIGNAL(connected()),this,SLOT(connected1()));
        disconnect(&tcp[1],SIGNAL(readyRead()),this,SLOT(readyRead1()));
        disconnect(&tcp[1],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error1(QAbstractSocket::SocketError)));
    }
    address = ui->lineEdit_server2->text().trimmed().split(':');
    if(!address.empty()) {
        tcp[2].reset();
        disconnect(&tcp[2],SIGNAL(connected()),this,SLOT(connected2()));
        disconnect(&tcp[2],SIGNAL(readyRead()),this,SLOT(readyRead2()));
        disconnect(&tcp[2],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error2(QAbstractSocket::SocketError)));
    }
    address = ui->lineEdit_server3->text().trimmed().split(':');
    if(!address.empty()) {
        tcp[3].reset();
        disconnect(&tcp[3],SIGNAL(connected()),this,SLOT(connected3()));
        disconnect(&tcp[3],SIGNAL(readyRead()),this,SLOT(readyRead3()));
        disconnect(&tcp[3],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error3(QAbstractSocket::SocketError)));
    }
    address = ui->lineEdit_server4->text().trimmed().split(':');
    if(!address.empty()) {
        tcp[4].reset();
        disconnect(&tcp[4],SIGNAL(connected()),this,SLOT(connected4()));
        disconnect(&tcp[4],SIGNAL(readyRead()),this,SLOT(readyRead4()));
        disconnect(&tcp[4],SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(error4(QAbstractSocket::SocketError)));
    }
}
