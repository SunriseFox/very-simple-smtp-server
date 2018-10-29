#ifndef QUOTEPRINTABLE_H
#define QUOTEPRINTABLE_H

#include <QString>
#include <QByteArray>
#include <QList>
#include <QDebug>

class QuotedPrintable
{
public:
    QuotedPrintable() = delete;
    static QString decode(const QByteArray &input);
};

#endif // QUOTEPRINTABLE_H
