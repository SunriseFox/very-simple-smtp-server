#include "simplesmtp.h"
#include "mailview.h"
SMTPSocket* SMTPSocket::_socket = nullptr;

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SimpleSMTP w;
    w.show();

    return a.exec();
}
