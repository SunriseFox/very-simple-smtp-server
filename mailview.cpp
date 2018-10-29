#include "mailview.h"
#include "ui_mailview.h"

MailView::MailView(QString filename, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MailView)
{
    this->filename = filename;
    ui->setupUi(this);

    QFile file(filename);
    file.open(QFile::ReadOnly);
    QString whole = file.readAll();

    doc = ui->textBrowser->document();

    handlePartialContent(whole, true);
}

MailView::~MailView()
{
    delete ui;
}

QByteArray MailView::getBody(const QString &whole, const QString &encoding) {
    QMap<QString, QString> map;
    const int index = whole.indexOf("\r\n\r\n");
    QString body = whole.mid(index + 4);
    body.chop(2);
    if (encoding.contains("quoted-printable")) {
        return QuotedPrintable::decode(body.toUtf8()).toUtf8();
    } else if (encoding.contains("base64")) {
        return Base64::decode(body.toUtf8());
    }
    return body.toUtf8();
}

void MailView::handlePartialContent(const QString &content, const bool& isMain = false) {
    QMap<QString, QString> header = getHeader(content);

    if(isMain) {
        QString title = header["subject"];
        if(title == nullptr || title.length() == 0)
            this->setWindowTitle(QuotedPrintable::decode("No Subject"));
        this->setWindowTitle(QuotedPrintable::decode(title.toLatin1()));
    }

    QString contentType = header["content-type"];
    QString contentTransferEncoding = header["content-transfer-encoding"];
    QString contentId = header["content-id"];

    if (contentType.contains("multipart")) {
        auto boundryExpr = QRegularExpression("boundary=\\\"(.+?)\\\"");
        auto match = boundryExpr.match(contentType);
        auto boundry = match.captured(1);

        auto body = QString(getBody(content, contentTransferEncoding));
        auto bodyArr = body.split(QString("--") + boundry, QString::SkipEmptyParts);
        for (auto &i: bodyArr) {
            if(&i == bodyArr.last()) break;
            handlePartialContent(i);
        }
    } else if(contentType.contains("text/html")) {
        auto body = QString(getBody(content, contentTransferEncoding));
        ui->textBrowser->insertHtml(QString(body));
    } else if(contentType.contains("image/")){
        auto id = header["content-id"];
        QRegularExpression exp("<(.+?)>");
        QRegularExpressionMatch match = exp.match(id);
        if(match.hasMatch()) {
            auto name = match.captured(1);
            auto body = getBody(content, contentTransferEncoding);
            doc->addResource(QTextDocument::ImageResource, QUrl("cid:"+name), body);
        }
    } else {
        auto body = getBody(content, contentTransferEncoding);
        auto nameExpr = QRegularExpression("name=\\\"(.+?)\\\"");
        auto match = nameExpr.match(contentType);
        auto name = match.hasMatch() ? match.captured(1) : filename + "_tmp_" + QString::number(++tmp);
        bodypart[name] = body;
    }
}

QMap<QString, QString> MailView::getHeader(const QString &whole) const {
    QMap<QString, QString> map;
    const int index = whole.indexOf("\r\n\r\n");
    QString header = whole.left(index);
    QStringList list = header.split("\n");
    QString lastKey;
    for (int i = 0; i < list.length(); i++){
        QStringList pair = list.at(i).split(":");
        if(pair.length() > 1) {
            const QString key = lastKey = pair.at(0).trimmed().toLower();
            pair.removeFirst();
            const QString value = pair.join(":").trimmed();
            map[key] = value;
        } else {
            map[lastKey].append(pair.at(0));
        }
    }
    return map;
}
