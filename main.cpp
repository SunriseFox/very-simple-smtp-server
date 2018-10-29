#include "simplesmtp.h"
#include "smtpsocket.h"
SMTPSocket* SMTPSocket::_socket = nullptr;

#include <QApplication>
#include <QSet>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SimpleSMTP w;
    w.show();

    SMTPSocket* socket = SMTPSocket::getSocket();
    Q_UNUSED(socket);

    return a.exec();
}
