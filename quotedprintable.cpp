#include "quotedprintable.h"

QString QuotedPrintable::decode(const QByteArray &input)
{
    QByteArray output;

    int begin = 0;
    int end = input.size();

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
