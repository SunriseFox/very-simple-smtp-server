#include "smtpperclient.h"
#include "ui_smtpperclient.h"

SMTPPerClient::SMTPPerClient(QTcpSocket* client, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SMTPPerClient)
{

    // 该类为每个客户端的交互日志窗体
    // 用于显示每个客户端的交互日志

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
