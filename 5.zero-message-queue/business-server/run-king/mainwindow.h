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
#include <QTimer>

#include <tuple>
using std::tuple;
using std::get;
using std::make_tuple;


#include <zmq.h>

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

private slots:
    void syncStatus();

private:    
    Ui::MainWindow *ui;
    QTimer t;
    void *context,*sock;

    float xyz[3];
    QMap<QString,tuple<float,float,float>> runners;

    ThreeWidget* tw;
    Basic* bp;
    Editor* ep;
    Animations* ap;
    Shadow* sp;
    Controls* cp;
    Observer* op;
};

#endif // MAINWINDOW_H
