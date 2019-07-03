#ifndef JSON_H
#define JSON_H

#include <QObject>
#include <QtNetwork>

class json : public QObject
{
    Q_OBJECT
public:
    explicit json(QObject *parent = nullptr);
private:
    QNetworkAccessManager manager;
    QUrl url;
    QNetworkRequest rqs;
    QString authkey;

signals:

public slots:
    QJsonObject Authorization(QString URL, QString log, QString pass);
    QJsonArray GetProblems(int launchTime);
private slots:
    QJsonObject GetJsonResonse(QString strJsonRequest);
};

#endif // JSON_H
