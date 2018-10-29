#ifndef MAILVIEW_H
#define MAILVIEW_H

#include "quotedprintable.h"
#include "base64.h"

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDebug>
#include <QMap>
#include <QTextDocument>

namespace Ui {
class MailView;
}

class MailView : public QWidget
{
    Q_OBJECT

public:
    explicit MailView(QString filename, QWidget *parent = nullptr);
    ~MailView();

protected:
    QMap<QString, QString> getHeader(const QString& whole) const;

    QByteArray getBody(const QString& whole, const QString& encoding);

    void handlePartialContent(const QString& content, const bool&);

private:
    Ui::MailView *ui;
    QString filename;
    QMap<QString, QByteArray> bodypart;
    qint32 tmp = 0;
    QTextDocument * doc;
};



#endif // MAILVIEW_H
