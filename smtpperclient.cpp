#include "smtpperclient.h"
#include "ui_smtpperclient.h"

SMTPPerClient::SMTPPerClient(QTcpSocket* client, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SMTPPerClient)
{
    this->_socket = client;
    ui->setupUi(this);
}

SMTPPerClient::~SMTPPerClient()
{
    delete ui;
}

void SMTPPerClient::on_closeClient_clicked()
{
    emit(closeClient(_socket));
}

void SMTPPerClient::on_downloadMail_clicked()
{
    if(filename.length() == 0) return;
    MailView* mailview = new MailView(filename);
    mailview->show();
}

void SMTPPerClient::closeEvent (QCloseEvent *event)
{
    emit(closeClient(_socket));
    this->deleteLater();
    event->accept();
}
