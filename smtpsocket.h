#ifndef SMTPSOCKET_H
#define SMTPSOCKET_H

#include "base64.h"

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QTimer>
#include <QSet>
#include <QMap>
#include <QStringList>

typedef QSet<QTcpSocket*> SMTPClients;

class SMTPSocket : public QObject
{
    Q_OBJECT
private:
    struct Command {
        QString command;
        std::function<bool(QTcpSocket*, QString)> callback;
    };
    static SMTPSocket* _socket;
    QTcpServer* server;
    SMTPClients clients;

    struct SMTPClientState {
        int authed = 0;
        int rawDataState = 0;
        QString authedName;
        QString mailFrom;
        QString recpTo;
        QByteArray data;
        std::function<bool(QTcpSocket*, QByteArray)> beforeParse = nullptr;
        std::function<bool(QTcpSocket*, QString)> beforeHandle = nullptr;
    };

    QMap<QTcpSocket*, SMTPClientState> clientState;
    QList<Command> commands {
        { QStringLiteral("AUTH"), std::bind(&SMTPSocket::onAUTH, this, std::placeholders::_1, std::placeholders::_2) },
        { QStringLiteral("DATA"), std::bind(&SMTPSocket::onDATA, this, std::placeholders::_1, std::placeholders::_2) },
        { QStringLiteral("EHLO"), std::bind(&SMTPSocket::onEHLO, this, std::placeholders::_1, std::placeholders::_2) },
        { QStringLiteral("QUIT"), std::bind(&SMTPSocket::onQUIT, this, std::placeholders::_1, std::placeholders::_2) },
        { QStringLiteral("HELP"), std::bind(&SMTPSocket::onHELP, this, std::placeholders::_1, std::placeholders::_2) },
        { QStringLiteral("MAIL FROM"), std::bind(&SMTPSocket::onMAILFROM, this, std::placeholders::_1, std::placeholders::_2) },
        { QStringLiteral("RCPT TO"), std::bind(&SMTPSocket::onRECPTO, this, std::placeholders::_1, std::placeholders::_2) },
        { QStringLiteral("RSET"), std::bind(&SMTPSocket::onRSET, this, std::placeholders::_1, std::placeholders::_2) },
    };

    QString SERVER_INFO = QStringLiteral("smtp.vampire.rip");

    SMTPSocket(QObject* parent = nullptr);

protected:
    bool isNewLine(QByteArray* byteArray);
    void processNewCommand(QTcpSocket* client, QByteArray *byteArray);

    bool assertNoParams(QTcpSocket*, QString);

    void sendCommand(QTcpSocket* client, int code, const QList<QString>& message);
    void sendCommandSequence(QTcpSocket* client, int code, const QList<QString>& message);
    void closeClient(QTcpSocket* client);
    void writeToClient(QTcpSocket* client, const QByteArray& array);

    bool onRAWDATA(QTcpSocket*, QByteArray);

    bool onAUTH(QTcpSocket*, QString);
    bool onDATA(QTcpSocket*, QString);
    bool onEHLO(QTcpSocket*, QString);
    bool onQUIT(QTcpSocket*, QString);
    bool onHELP(QTcpSocket*, QString);
    bool onMAILFROM(QTcpSocket*, QString);
    bool onRECPTO(QTcpSocket*, QString);
    bool onRSET(QTcpSocket*, QString);
    bool onNOSUCHMETHOD(QTcpSocket*, QString);

public:
    static SMTPSocket* getSocket();
    SMTPClients getClients() const;

    void sendToAllClients(int code, QList<QString> message);

signals:
    void clientChanged(SMTPClients);

public slots:
    void handleNewConnection();
};


#endif // SMTPSOCKET_H
