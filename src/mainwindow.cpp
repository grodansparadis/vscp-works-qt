#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QPixmap image("/home/akhe/development/VSCP/vscp-works-qt/vscp-works-qt/images/vscp_logo.jpg");

    ui->setupUi(this);

    ui->logo->setPixmap(image);
    ui->logo->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

