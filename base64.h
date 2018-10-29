#ifndef BASE64_H
#define BASE64_H

#include <QByteArray>
#include <QChar>
#include <QVector>
#include <QMap>
#include <QDebug>

class Base64
{
public:
    Base64() = delete;
    static QByteArray decode(const QString& base64);
};

#endif // BASE64_H
