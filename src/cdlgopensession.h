#ifndef CDLGOPENSESSION_H
#define CDLGOPENSESSION_H

#include <QDialog>

namespace Ui {
class CDlgOpenSession;
}

class CDlgOpenSession : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgOpenSession(QWidget *parent = nullptr);
    ~CDlgOpenSession();

private:
    Ui::CDlgOpenSession *ui;
};

#endif // CDLGOPENSESSION_H
