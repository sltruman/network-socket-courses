#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&t,SIGNAL(timeout()),this,SLOT(onFrame()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onFrame() {
    string line;
    getline(in_mediasoup_client,line);
    cout << "onFrame:" << line << endl;
    bi::shared_memory_object shm_obj(bi::open_or_create,"shared_memory",bi::read_only);
    auto size_header = 2 * sizeof(int);

    bi::mapped_region region_header(shm_obj, bi::read_only,0,size_header);
    auto header = (unsigned int*)region_header.get_address();
    auto width = header[0];
    auto height = header[1];
    auto size = width * height * 4;

    bi::mapped_region region(shm_obj,bi::read_only);
    auto image = reinterpret_cast<unsigned char*>(region.get_address()) + size_header;

    auto buffer = new unsigned char[size];
    memcpy(buffer,image,size);
    ui->label->setPixmap(QPixmap::fromImage(*new QImage(buffer,width,height,QImage::Format_ARGB32)));
    delete buffer;
}

void MainWindow::on_pushButton_2_clicked()
{
    mediasoup_client = make_shared<bp::child>(bp::search_path(R"(mediasoup-client)"),
                                            bp::std_out > in_mediasoup_client,
                                            bp::std_in < out_mediasoup_client);

    out_mediasoup_client << "subscription" << endl;

    ui->pushButton->setDisabled(true);
    ui->pushButton_2->setDisabled(true);
    t.setSingleShot(false);
    t.setInterval(100);
    t.start();
}

void MainWindow::on_pushButton_clicked()
{
    mediasoup_client = make_shared<bp::child>(bp::search_path(R"(mediasoup-client)"),
                                              bp::std_out > in_mediasoup_client,
                                              bp::std_in < out_mediasoup_client);
    out_mediasoup_client << "publishing" << endl;
    ui->pushButton->setDisabled(true);
    ui->pushButton_2->setDisabled(true);
}
