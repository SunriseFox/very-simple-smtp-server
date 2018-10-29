#ifndef SIMPLESMTP_H
#define SIMPLESMTP_H

#include "smtpsocket.h"
#include <QWidget>

namespace Ui {
class SimpleSMTP;
}

class SimpleSMTP : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleSMTP(QWidget *parent = nullptr);
    ~SimpleSMTP();

private slots:
    void on_pushButton_clicked();

private:
    Ui::SimpleSMTP *ui;
    SMTPSocket* socket;
};

#endif // SIMPLESMTP_H
