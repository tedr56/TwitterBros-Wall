#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include "OAuth2Qt/src/o1twitter.h"
#include "OAuth2Qt/src/o1requestor.h"
#include <QtNetwork>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLayoutItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onLinkedChanged();
    void onLinkingFailed();
    void onLinkingSucceeded();
    void onOpenBrowser(const QUrl &url);
    void onCloseBrowser();

    void makeRequest();
    void readFeed(qint64 bytesReceived, qint64 bytesTotal);

    void addReTweet();

    void acceptConnection();
    void startRead();

    void removeTweet(QLayoutItem* item, int position = 0);
    void clearTweets();

protected:
    void parseFeed();
    void loadReTweet(QJsonDocument* Tweet);

    void readTweet(QByteArray tweet_id);
    void parseTweet();
    void addTweet(QJsonDocument* embedJsonTweet);

    void loadOEmbed(QString tweet_id);

private:
    Ui::MainWindow *ui;

    QSettings settings;

    O1Twitter *o1;
    QNetworkAccessManager* manager;
    QNetworkReply *reply;
    QString rawResponse;
    QString parsedResponse;
    int responseTweetSize;

    QTcpServer server;
    QTcpSocket* client;

    QByteArray rawTweet;
    qint32 rawTweetSize;
    QString parsedTweet;

    int contentWidth;
    int contentHeigh;
};

#endif // MAINWINDOW_H
