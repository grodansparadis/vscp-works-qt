#include "cfrmsession.h"
#include "ui_cfrmsession.h"

CFrmSession::CFrmSession(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CFrmSession)
{
    ui->setupUi(this);
}

CFrmSession::~CFrmSession()
{
    delete ui;
}
