#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "interfaces.h"
#include "plugins/basic.h"
#include "plugins/editor.h"
#include "plugins/animations.h"
#include "plugins/shadow.h"
#include "plugins/controls.h"
#include "plugins/observer.h"

#include <QMainWindow>
#include <QList>
#include <QMap>
#include <QTcpSocket>

#include <tuple>
using std::tuple;
using std::get;
using std::make_tuple;

using namespace ThreeQt;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool send(QTcpSocket&);
    bool receive(QTcpSocket&);

private slots:
    void connected0() {send(tcp[0]);}
    void readyRead0() {receive(tcp[0]);send(tcp[0]);}
    void error0(QAbstractSocket::SocketError) {tcp[0].disconnectFromHost();}
    void connected1() {send(tcp[1]);}
    void readyRead1() {receive(tcp[1]);send(tcp[1]);}
    void error1(QAbstractSocket::SocketError) {tcp[1].disconnectFromHost();}
    void connected2() {send(tcp[2]);}
    void readyRead2() {receive(tcp[2]);send(tcp[2]);}
    void error2(QAbstractSocket::SocketError) {tcp[2].disconnectFromHost();}
    void connected3() {send(tcp[3]);}
    void readyRead3() {receive(tcp[3]);send(tcp[3]);}
    void error3(QAbstractSocket::SocketError) {tcp[3].disconnectFromHost();}
    void connected4() {send(tcp[4]);}
    void readyRead4() {receive(tcp[4]);send(tcp[4]);}
    void error4(QAbstractSocket::SocketError) {tcp[4].disconnectFromHost();}

    void on_pushButton_connect_clicked();
    void on_pushButton_disconnect_clicked();

private:
    Ui::MainWindow *ui;

    float xyz[3];
    QMap<QString,tuple<float,float,float>> runners;

    ThreeWidget* tw;
    Basic* bp;
    Editor* ep;
    Animations* ap;
    Shadow* sp;
    Controls* cp;
    Observer* op;
    QTcpSocket tcp[5];
};

#endif // MAINWINDOW_H
