#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <memory>
#include <iostream>
#include <fstream>
#include <thread>
using namespace std;

#include <boost/process.hpp>
namespace bp = boost::process;

#include <boost/format.hpp>
using boost::format;

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
namespace bi = boost::interprocess;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bp::ipstream in_mediasoup_client;
    bp::opstream out_mediasoup_client;
    shared_ptr<bp::child> mediasoup_client;
    QTimer t;

private slots:
    void onFrame();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
