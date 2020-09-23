#include "cdlgopensession.h"
#include "ui_cdlgopensession.h"

CDlgOpenSession::CDlgOpenSession(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgOpenSession)
{
    ui->setupUi(this);
}

CDlgOpenSession::~CDlgOpenSession()
{
    delete ui;
}
