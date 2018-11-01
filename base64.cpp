#include "base64.h"

QByteArray Base64::decode(const QString& base64)
{
    // Base64 是将 3 * 8 位转变为 4 * 6 位字符的编码方式
    // 我们这里做它的逆，只需要将 4 (*6) 个字符变回 3 (*8) 个字符即可
    // 我们的邮件编码每 176 字符会加入 \r\n，必须忽略
    // 首先查可见字符表 获得其编号，然后根据每 4 个的编号还原为 3 个字符。

    static const QString convert = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    static QMap<QChar, char> mapping;
    if(mapping.size() == 0) {
        for (char i = 0; i < convert.length(); i ++) {
            mapping[convert[i]] = i;
        }
    }
    QByteArray result;

    int bias = 0;

    for (int i = 0; i + bias < base64.length(); i += 4) {
        char tmp[4];
        int size = 4;
        for (int j = 0; j < size; j++) {
            int idx = i + j + bias;
            if (idx >= base64.length()) {
                size = j;
                break;
            }
            QChar tChar = base64.at(idx);
            if(!mapping.contains(tChar)) {
                bias++;
                j--;
                continue;
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
    return result;
}
