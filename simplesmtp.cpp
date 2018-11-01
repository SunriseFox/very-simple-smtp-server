#include "simplesmtp.h"
#include "ui_simplesmtp.h"
#include "ui_smtpperclient.h"
#include <QMessageBox>


SimpleSMTP::SimpleSMTP(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimpleSMTP)
{

    // 该类是程序主窗体主类
    // 但只是通过事件和槽机制，用于显示每个客户端的交互日志

    ui->setupUi(this);
    socket = SMTPSocket::getSocket();
    connect(socket, &SMTPSocket::onServerLog, this, [this](QString log){
        ui->serverLog->append(log);
    });
    connect(socket, &SMTPSocket::onNewClient, this, [=](QTcpSocket* client){
        window[client] = new SMTPPerClient(client);
        window[client]->setWindowTitle("Client " + QString::number(reinterpret_cast<uintptr_t>(client)));
        window[client]->showNormal();
        connect(window[client], &SMTPPerClient::closeClient, this, [this](QTcpSocket* client){
           socket->closeClient(client);
           disconnect(window[client]);
           window.remove(client);
        });

        ui->nConn->setText(QString::number(++nConn));
    });
    connect(socket, &SMTPSocket::onWriteToClient, this, [=](QTcpSocket* client, QByteArray data){
        if(window.contains(client))
            window[client]->ui->clientLog->append("S: " + data);
    });
    connect(socket, &SMTPSocket::onReceiveFromClient, this, [=](QTcpSocket* client, QByteArray data){
        if(window.contains(client))
            window[client]->ui->clientLog->append("C: " + data);
    });
    connect(socket, &SMTPSocket::onDataSent, this, [=](QTcpSocket* client, qint64 size, QString filename){
        if(window.contains(client)) {
            window[client]->ui->clientLog->append("<received data " + QString::number(size) + " bytes, saved as " + filename + ">");
            window[client]->filename = filename;
        }
    });
    connect(socket, &SMTPSocket::onConnectionClose, this, [=](QTcpSocket* client){
        if(window.contains(client))
            window[client]->ui->clientLog->append("<closed>");
        ui->nConn->setText(QString::number(--nConn));
    });
}

SimpleSMTP::~SimpleSMTP()
{
    delete ui;
}

void SimpleSMTP::on_startListen_clicked()
{
    if(socket->startListen())
        ui->nConn->setText("0");
}

void SimpleSMTP::on_resetClients_clicked()
{
    for(const auto& iter: window.keys())
    {
        const auto& i =  window.value(iter);
        window.remove(iter);
        i->close();
        i->deleteLater();
    }
    socket->resetAllClients();
    ui->nConn->setText("0");
}
