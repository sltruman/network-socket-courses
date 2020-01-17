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
#include <QTimer>
#include <QMap>
#include <QTcpSocket>

#include <tuple>
using std::tuple;
using std::get;
using std::make_tuple;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void peek();
    void sync();
    void syncErr(QAbstractSocket::SocketError);


private:
    Ui::MainWindow *ui;


    float xyz[3];
    QMap<QString,tuple<float,float,float>> runners;

    ThreeWidget* tw;
    Basic::Plugin* bp;
    Editor::Plugin* ep;
    Animations::Plugin* ap;
    Shadow::Plugin* sp;
    Controls::Plugin* cp;
    Observer::Plugin* op;
    QTcpSocket tcp;

};

#endif // MAINWINDOW_H
