#include "base64.h"

QByteArray Base64::decode(const QString& base64)
{
    static const QString convert = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    static QMap<QChar, char> mapping;
    if(mapping.size() == 0) {
        for (char i = 0; i < convert.length(); i ++) {
            mapping[convert[i]] = i;
        }
    }
    QByteArray result;

    for (int i = 0; i < base64.length(); i += 4) {
        int size = base64.length() - i >= 4 ? 4 : base64.length() - i > 4;
        char tmp[4];
        for (int j = 0; j< size; j++) {
            int idx = i + j;
            QChar tChar = base64.at(idx);
            if(!mapping.contains(tChar)) {
                size = j;
                break;
            }
            tmp[j] = mapping[tChar];
        }
        if (size > 1)
            result += char((tmp[0] << 2) | (tmp[1] >> 4));

        if (size > 2)
            result += char((tmp[1] << 4) | (tmp[2] >> 2));

        if (size > 3)
            result += char((tmp[2] << 6) | (tmp[3] >> 0));
    }
    qDebug() << "base64" << result;
    return result;
}
