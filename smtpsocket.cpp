#include "smtpsocket.h"

SMTPSocket::SMTPSocket(QObject *parent) : QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &SMTPSocket::handleNewConnection);
}


bool SMTPSocket::isNewLine(QByteArray *byteArray)
{
    return byteArray->endsWith("\n");
}

void SMTPSocket::processNewCommand(QTcpSocket* client, QByteArray *byteArray)
{
    bool handled = false;

    if (clientState[client].beforeParse != nullptr) {
        handled = clientState[client].beforeParse(client, *byteArray);
    }
    if (handled) return;

    QString command(*byteArray);
    command = command.trimmed();

    emit(onReceiveFromClient(client, command.replace('\x00', '-').toLatin1()));

    qDebug() << "(" << reinterpret_cast<uintptr_t>(client) << ") < " << command;

    if (clientState[client].beforeHandle != nullptr) {
        handled = clientState[client].beforeHandle(client, command);
    }
    if (handled) return;

    for (auto& i: commands) {
        if(command.startsWith(i.command)) {
            handled = i.callback(client, command);
            if(handled) break;
        }
    }

    if(!handled) onNOSUCHMETHOD(client, command);
}

bool SMTPSocket::assertNoParams(QTcpSocket * client, QString params)
{
    auto list = params.split(" ", QString::SkipEmptyParts);
    if (list.length() != 1) {
        sendCommand(client, 501, {"Syntax error in command or arguments"});
        return true;
    }
    return false;
}

void SMTPSocket::sendCommand(QTcpSocket *client, int code, const QList<QString>& list)
{
    QString message = list.join(" ");
    writeToClient(client, QByteArray::number(code) + " " + message.toLatin1() + "\r\n");
}

void SMTPSocket::sendCommandSequence(QTcpSocket *client, int code, const QList<QString>& list)
{
    QString message = list.join(" ");
    writeToClient(client, QByteArray::number(code) + "-" + message.toLatin1() + "\r\n");
}

void SMTPSocket::closeClient(QTcpSocket *client)
{
    if(!clients.contains(client)) return;
    qDebug() << "closing client" << reinterpret_cast<uintptr_t>(client);
    onConnectionClose(client);
    client->close();
}

void SMTPSocket::writeToClient(QTcpSocket *client, const QByteArray &array)
{
    qDebug() << "(" << reinterpret_cast<uintptr_t>(client) << ") > " << array;
    emit(onWriteToClient(client, array));
    client->write(array);
}

bool SMTPSocket::onRAWDATA(QTcpSocket * client, QByteArray bytes)
{
    qDebug() << bytes;
    if(clientState[client].rawDataState == 1) {
        if(bytes == ".\r\n") {
            clientState[client].rawDataState = 2;
            clientState[client].beforeParse = nullptr;
            QFile file(clientState[client].authedName.split("-")[0] + "-" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".eml");
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
                qDebug() << "[file] failed to open for write.";
            file.write(clientState[client].data);
            onDataSent(client, clientState[client].data.size(), file.fileName());
            clientState[client].data.clear();
            sendCommand(client, 250, {"OK: message queued as", "42"});
            return true;
        } else {
            clientState[client].rawDataState = 0;
            clientState[client].data.append("\n");
        }
    }
    if(bytes == "\r\n") {
        clientState[client].rawDataState = 1;
    } else {
        clientState[client].data.append(bytes.replace("\r\n", "\n"));
    }
    return true;
}

bool SMTPSocket::onAUTH(QTcpSocket * client, QString auth)
{
    if (clientState[client].authed == 2) {
        QString pass = Base64::decode(auth).replace('\x00', '-');
        sendCommand(client, 235, {"Auth Successful with ", pass});
        clientState[client].authedName = pass;
        clientState[client].authed = 1;
        clientState[client].beforeHandle = nullptr;
        return true;
    } else if (clientState[client].authed == 0) {
        auto array = auth.split(" ", QString::SkipEmptyParts);
        if (array.length() < 2) return false;
        if (array[1].toLower() == "plain") {
            if(array.length() == 3) {
                QString pass = Base64::decode(array[2]).replace('\x00', '-');
                sendCommand(client, 235, {"Auth Successful with ", pass});
                clientState[client].authedName = pass;
                clientState[client].authed = 1;
                return true;
            }
            else if (array.length() == 2) {
                clientState[client].authed = 2;
                clientState[client].beforeHandle = std::bind(&SMTPSocket::onAUTH, this, std::placeholders::_1, std::placeholders::_2);
                sendCommand(client, 334, {});
                return true;
            }
        }
    }
    sendCommand(client, 503, {"Bad sequence of commands"});
    return true;
}

bool SMTPSocket::onDATA(QTcpSocket * client, QString data)
{
    if(assertNoParams(client, data)) return true;
    clientState[client].beforeParse = std::bind(&SMTPSocket::onRAWDATA, this, std::placeholders::_1, std::placeholders::_2);
    sendCommand(client, 354, {"End data with <CR><LF>.<CR><LF>"});
    return true;
}

bool SMTPSocket::onEHLO(QTcpSocket * client, QString ehlo)
{
    auto list = ehlo.split(" ");
    list.removeFirst();
    sendCommandSequence(client, 250, {SERVER_INFO, "greets", list.join(" ")});
    QList<QString> ehlo_hint {
        "AUTH PLAIN",
        "SIZE 14680064",
        "HELP",
    };
    for (auto& i: ehlo_hint) {
        if (&i != &ehlo_hint.last()) {
            sendCommandSequence(client, 250, {i});
        } else {
            sendCommand(client, 250, {i});
        }
    }
    return true;
}

bool SMTPSocket::onQUIT(QTcpSocket * client, QString data)
{
    if(assertNoParams(client, data)) return true;
    sendCommand(client, 221, {"Bye"});
    client->flush();
    closeClient(client);
    return true;
}

bool SMTPSocket::onHELP(QTcpSocket * client, QString)
{
    for (auto& i: commands) {
        if (&i != &commands.last()) {
            sendCommandSequence(client, 250, {i.command});
        } else {
            sendCommand(client, 250, {i.command});
        }
    }
    return true;
}

bool SMTPSocket::onMAILFROM(QTcpSocket * client, QString from)
{
    if (clientState[client].authed == 1){
        clientState[client].mailFrom = from;
        sendCommand(client, 250, {"Ok"});
        return true;
    }
    return false;
}

bool SMTPSocket::onRECPTO(QTcpSocket * client, QString to)
{
    if (clientState[client].authed == 1){
        clientState[client].recpTo = to;
        sendCommand(client, 250, {"Ok"});
        return true;
    }
    return false;
}

bool SMTPSocket::onRSET(QTcpSocket * client, QString data)
{
    if(assertNoParams(client, data)) return true;
    clientState[client] = SMTPClientState();
    sendCommand(client, 250, {"OK"});
    return true;
}

bool SMTPSocket::onNOSUCHMETHOD(QTcpSocket *client, QString)
{
    sendCommand(client, 500, {"Syntax error"});
    return true;
}

SMTPSocket* SMTPSocket::getSocket()
{
    if (SMTPSocket::_socket == nullptr) {
        SMTPSocket::_socket = new SMTPSocket();
    }
    return SMTPSocket::_socket;
}

SMTPClients SMTPSocket::getClients() const
{
    return clients;
}

void SMTPSocket::sendToAllClients(int code, QList<QString> message)
{
    for (auto& i: clients) {
        sendCommand(i, code, message);
    }
}

bool SMTPSocket::startListen()
{
    const int success = server->listen(QHostAddress::Any, 587);
    if (!success) {
        QString log = QStringLiteral("SMTP Server Started failed. ") + server->errorString();
        qDebug() << log;
        emit(onServerLog(log));
        return false;
    }
    QString log = QStringLiteral("Server's Ready at: ") + server->serverAddress().toString() + QString::number(server->serverPort());
    qDebug() << "Server's Ready at: " << server->serverAddress().toString() << server->serverPort();
    emit(onServerLog(log));
    return true;
}

bool SMTPSocket::resetAllClients()
{
    shouldAcceptNewConnection = false;
    for (auto& i: clients) {
        sendCommand(i, 421, {"Service not available, closing transmission channel"});
        QMetaObject::invokeMethod(this, [=](){this->closeClient(i);}, Qt::QueuedConnection, nullptr);
    }
    shouldAcceptNewConnection = true;
    QMetaObject::invokeMethod(this, [this](){this->handleNewConnection();},Qt::QueuedConnection, nullptr);
    return true;
}


void SMTPSocket::handleNewConnection()
{
    if (!shouldAcceptNewConnection || !server->hasPendingConnections()) return;
    QTcpSocket* client = server->nextPendingConnection();
    clients.insert(client);
    clientState[client] = SMTPClientState();
    emit onNewClient(client);
    uintptr_t clientId = reinterpret_cast<uintptr_t>(client);
    emit clientChanged(clients);
    sendCommand(client, 220, {SERVER_INFO, "ESMTP", QString::number(clientId)});
    connect(client, &QTcpSocket::readyRead, this, [=](){
        QByteArray* byteArray = new QByteArray();
        do{
            char buf[1024];
            qint64 bytesRead = client->readLine(buf, 1024);
            if (bytesRead < 0)
                client->close();
            if (bytesRead < 1)
                break;
            byteArray->append(buf);
            if (isNewLine(byteArray)) {
                QMetaObject::invokeMethod(this, [=](){this->processNewCommand(client, byteArray);}, nullptr);
                byteArray = new QByteArray();
            } else {
                if(!client->waitForReadyRead(10000)){
                    client->close();
                };
            }
        } while (true);
    });
    connect(client, &QTcpSocket::disconnected, this, [=](){
        client->close();
    });
    connect(client, &QTcpSocket::aboutToClose, this, [=](){
        QString log = QStringLiteral("[cleanup] ") + QString::number(clientId);
        qDebug() << log;
        emit(onServerLog(log));
        if(clients.contains(client) && client->isOpen()) {
           client->deleteLater();
        }
        clients.remove(client);
    });
    QHostAddress address = client->peerAddress();
    QString connectionInfo;
    if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        connectionInfo += QStringLiteral("[") + address.toString() + "]:" + QString::number(client->peerPort());
    } else {
        connectionInfo += address.toString() + ":" + QString::number(client->peerPort());
    }
    emit(onServerLog(QStringLiteral("[connection] ") + QString::number(clientId) + " from " + connectionInfo));
    qDebug() << "Got connection from" << connectionInfo << "clientId" << clientId;
}
