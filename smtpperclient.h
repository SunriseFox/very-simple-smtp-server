#ifndef SMTPPERCLIENT_H
#define SMTPPERCLIENT_H

#include "mailview.h"

#include <QWidget>
#include <QTcpSocket>
#include <QCloseEvent>

namespace Ui {
class SMTPPerClient;
}

class SMTPPerClient : public QWidget
{
    Q_OBJECT

public:
    explicit SMTPPerClient(QTcpSocket*, QWidget *parent = nullptr);
    ~SMTPPerClient();
    QString filename;
    Ui::SMTPPerClient *ui;

protected:
    void closeEvent (QCloseEvent *event);

private slots:
    void on_closeClient_clicked();
    void on_downloadMail_clicked();

signals:
    void closeClient(QTcpSocket*);

private:
    QTcpSocket* _socket;
};

#endif // SMTPPERCLIENT_H
