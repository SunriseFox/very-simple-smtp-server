#ifndef SIMPLESMTP_H
#define SIMPLESMTP_H

#include "smtpsocket.h"
#include "smtpperclient.h"

#include <QWidget>
#include <QMap>

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
    void on_startListen_clicked();
    void on_resetClients_clicked();

private:
    Ui::SimpleSMTP *ui;
    SMTPSocket* socket;
    QMap<QTcpSocket*, SMTPPerClient*> window;

    qint32 nConn = 0;
};

#endif // SIMPLESMTP_H
