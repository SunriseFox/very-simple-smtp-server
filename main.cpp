#include "simplesmtp.h"
#include "mailview.h"
SMTPSocket* SMTPSocket::_socket = nullptr;

#include <QApplication>

int main(int argc, char *argv[])
{
    // main.cpp:  程序的入口点
    // SimpleSMTP: 界面入口点
    // Base64 Class: 编码解码
    // QuotePrintable Class: 编码解码
    // SMTPSocket: 维护所有收信发信的事件队列
    // SMTPPerClient: 每个客户端的交互窗体和事件日志
    // MailView: 每个客户端的邮件内容展示

    QApplication a(argc, argv);
    SimpleSMTP w;
    w.show();

    return a.exec();
}
