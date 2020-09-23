#ifndef CFRMSESSION_H
#define CFRMSESSION_H

#include <QDialog>

namespace Ui {
class CFrmSession;
}

class CFrmSession : public QDialog
{
    Q_OBJECT

public:
    explicit CFrmSession(QWidget *parent = nullptr);
    ~CFrmSession();

private:
    Ui::CFrmSession *ui;
};

#endif // CFRMSESSION_H
