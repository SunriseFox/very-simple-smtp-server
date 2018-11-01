#include "quotedprintable.h"

QString QuotedPrintable::decode(const QByteArray &input)
{
    // QuotedPrintable 是将所有特殊字符转变为 =<ASCII> 的编码方式
    // 邮件会每 176 个字符加入 =\r\n，因此遇到 =\r\n 将其忽略
    // 遇到其他的 =<ASCII> 即将其还原为 1 个字符即可。

    QByteArray output;

    int begin = 0;
    int end = input.size();

    // 这里处理的是 MIME，我们无法解码 UTF-8 以外编码，因此忽略编码

    if(input.startsWith("=?") && input.endsWith("?=")) {
        for (int i = 0; i < 3; i++) {
            begin = input.indexOf('?', begin) + 1;
        }
        end = end - 2;
    }

    for (int i = begin; i < end; ++i)
    {
        if (input.at(i) == '=' && i+2 < end)
        {
            if(input.at(i+1) == '\r' && input.at(i+2) == '\n') {
                i+=2;
                continue;
            }
            QString strValue = input.mid((++i)++, 2);
            bool converted;
            char character = static_cast<char>(strValue.toUInt(&converted, 16));
            if( converted )
                output.append(character);
            else
                output.append( "=" + strValue);
        }
        else
        {
            output.append(input.at(i));
        }
    }

    return QString::fromUtf8(output);
}
