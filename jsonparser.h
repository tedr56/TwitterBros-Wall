#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

class JsonParser
{
public:
    JsonParser(QJsonDocument* JsonDoc);
    ~JsonParser();

    QString parseJson(QString Path);
private:
    QJsonDocument* JsonDocument;
    QJsonValue parseObject(QJsonObject object, QString path);
    QJsonValue parseArray(QJsonArray array, QString path);
};

#endif // JSONPARSER_H
