#include <QUrl>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QWebView>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "jsonparser.h"

#include "tweetwidget.h"
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //UI Setup
    ui->setupUi(this);
    //this->setFixedSize(1024,768);
    this->setMaximumSize(2000,2000);
    //Keyboard shortcut
    new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(close()));
    new QShortcut(QKeySequence(Qt::Key_Enter), this, SLOT(clearTweets()));
    new QShortcut(QKeySequence(Qt::Key_F), this, SLOT(showFullScreen()));



    //Hashtag setup
    QFile* hashtag = new QFile(":/HASHTAG.txt");
    if (hashtag->open(QFile::ReadOnly | QFile::Text))
    {
        ui->HashTag->setText("<html><head/><body><p><span style=\" font-size:20pt; color:#35bfbf;\">" + hashtag->readAll() + "</span></p></body></html>");
        hashtag->close();
    }
    //Content Resolution
    QFile* resolutionContenu = new QFile(":/RESOLUTION-CONTENU.txt");
    if (resolutionContenu->open(QFile::ReadOnly | QFile::Text))
    {
        QByteArray resolution = resolutionContenu->readAll();
        QList<QByteArray> resolutions = resolution.split('\n');
        if (resolutions.size() >= 2)
        {
            contentWidth = resolutions[0].toInt();
            contentHeigh = resolutions[1].toInt();
        }
        else
        {
            contentWidth = 250;
            contentHeigh = 200;
        }
    }
    resolutionContenu->close();


    //TCP Server Conenction
    manager = new QNetworkAccessManager(this);
    server.listen(QHostAddress::Any, 8889);
    connect(&server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    rawTweetSize = 0;

    //Twitter Connection
//    o1 = new O1Twitter(this);
//    o1->setClientId("cy6lWa5BeemCpl6Xa727HwRil");
//    o1->setClientSecret("ThcsiU4SogKYaauY5l2jP3Eqb7awB1jH9amQxGVqJBoPlAUYWt");

//    o1->setLocalPort(9999);
//    connect(o1, SIGNAL(linkedChanged()), this, SLOT(onLinkedChanged()));
//    connect(o1, SIGNAL(linkingFailed()), this, SLOT(onLinkingFailed()));
//    connect(o1, SIGNAL(linkingSucceeded()), this, SLOT(onLinkingSucceeded()));
//    connect(o1, SIGNAL(openBrowser(QUrl)), this, SLOT(onOpenBrowser(QUrl)));
//    connect(o1, SIGNAL(closeBrowser()), this, SLOT(onCloseBrowser()));

//    o1->link();


//    if (o1->linked())
//        makeRequest();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::makeRequest()
{
    O1Requestor* requestor = new O1Requestor(manager, o1, this);

    QList<O1RequestParameter> reqParams = QList<O1RequestParameter>();
    //O1RequestParameter user = O1RequestParameter("with", "user");
    O1RequestParameter length = O1RequestParameter("delimited", "length");
    QByteArray user_id = settings.value("user_id", "").toByteArray();
    O1RequestParameter follow = O1RequestParameter("follow", user_id);

    reqParams << length << follow;

    QByteArray postData = O1::createQueryParams(reqParams);

    QUrl url = QUrl("https://stream.twitter.com/1.1/statuses/filter.json");

    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    reply = requestor->post(request, reqParams, postData);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(readFeed(qint64,qint64)));
}

void MainWindow::onLinkedChanged() {
    // Linking (login) state has changed.
    // Use o1->linked() to get the actual state
    qDebug() << "onLinkedChanged";
    if (o1->linked())
        makeRequest();
}

void MainWindow::onLinkingFailed() {
    // Login has failed
    qDebug() << "onLinkingFailed";
}

void MainWindow::onLinkingSucceeded() {
    // Login has succeeded
    QVariantMap extraToken = o1->extraTokens();
    qDebug() << extraToken;
    if (extraToken.isEmpty())
        return;
    if  (!extraToken.contains("user_id"))
        return;
    if  (!extraToken.contains("screen_name"))
        return;

    QSettings settings;
    settings.setValue("user_id", extraToken["user_id"]);
    settings.setValue("screen_name", extraToken["screen_name"]);
}

void MainWindow::onOpenBrowser(const QUrl &url) {
    // Open a web browser or a web view with the given URL.
    // The user will interact with this browser window to
    // enter login name, password, and authorize your application
    // to access the Twitter account
    qDebug() << "Opening browser with url" << url.toString();
    QDesktopServices::openUrl(url);
}

void MainWindow::onCloseBrowser() {
    // Close the browser window opened in openBrowser()
    qDebug() << "onCloseBrowser";
}

void MainWindow::readFeed(qint64 bytesReceived, qint64 bytesTotal)
{
    Q_UNUSED(bytesReceived);
    Q_UNUSED(bytesTotal);

    QString rawFragment = reply->readAll();
    rawResponse.append(rawFragment);
    parseFeed();
}

void MainWindow::parseFeed()
{
    QStringList responseSplit = QString(rawResponse).split(QRegExp("[\r\n]"),QString::SkipEmptyParts);

    foreach (QString split, responseSplit) {
        if (responseTweetSize == 0)
        {
            responseTweetSize = split.toInt();
        }
        else
        {
            parsedResponse.append(split);
            if (parsedResponse.size() >= responseTweetSize - 3)
            {
                QJsonDocument* response = new QJsonDocument(QJsonDocument::fromJson(parsedResponse.toLatin1()));
                if (!response->isNull())
                {
//                    qDebug() << JsonParser(response).parseJson("text");
                    //qDebug() << response->toJson();

                    //loadTweet(response);
                }
                responseTweetSize = 0;
                parsedResponse.clear();
            }
        }
    }
    rawResponse.clear();
}

void MainWindow::loadReTweet(QJsonDocument *Tweet)
{
    QByteArray tweet_id = JsonParser(Tweet).parseJson("id_str").toLatin1();
    qDebug() << "tweet_id : " << tweet_id;
    O1Requestor* requestor = new O1Requestor(manager, o1, this);

    QByteArray paramName("status");

    QList<O1RequestParameter> reqParams = QList<O1RequestParameter>();
    O1RequestParameter tweetId = O1RequestParameter("id", tweet_id);

    reqParams << tweetId;

    QByteArray postData = O1::createQueryParams(reqParams);

    QUrl url = QUrl("https://api.twitter.com/1.1/statuses/oembed.json");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *replyEmbed = requestor->post(request, reqParams, postData);
    connect(replyEmbed, SIGNAL(finished()), this, SLOT(addReTweet()));

}


void MainWindow::addReTweet()
{
    QJsonDocument* embedJsonTweet = new QJsonDocument();

    QNetworkReply *replyEmbed = qobject_cast<QNetworkReply *>(sender());
    if (replyEmbed->error() != QNetworkReply::NoError) {
        qDebug() << "ERROR:" << replyEmbed->errorString();
        qDebug() << "content:" << replyEmbed->readAll();
    } else {
        qDebug() << "embed Json retrieved!";
        QByteArray rawEmbedTweet = replyEmbed->readAll();
        qDebug() << rawEmbedTweet;
        embedJsonTweet->fromJson(rawEmbedTweet);
    }

    QString embedHtmlTweet = JsonParser(embedJsonTweet).parseJson("html");

    QWebView* TweetView = new QWebView(ui->Widget);
    TweetView->setHtml(embedHtmlTweet);
    //ui->TwitterRetweet->addWidget(TweetView);
    reply->deleteLater();
}

void MainWindow::loadOEmbed(QString tweet_id)
{
    O1Requestor* requestor = new O1Requestor(manager, o1, this);
    QList<O1RequestParameter> reqParams = QList<O1RequestParameter>();
//    O1RequestParameter tweetId = O1RequestParameter("id", tweet_id);
//    reqParams << tweetId;
    QByteArray postData = O1::createQueryParams(reqParams);
    QUrl url = QUrl(QString("https://api.twitter.com/1.1/statuses/oembed.json?id=" + tweet_id));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *replyEmbed = requestor->get(request, reqParams);
    connect(replyEmbed, SIGNAL(finished()), this, SLOT(addTweet()));

    qDebug() << "Request headers: ";

    QList<QByteArray> reqHeaders = request.rawHeaderList();
    foreach( QByteArray reqName, reqHeaders )
    {
    QByteArray reqValue = request.rawHeader( reqName );
    qDebug() << reqName << ": " << reqValue;
    }
}

void MainWindow::acceptConnection()
{
    client = server.nextPendingConnection();
    qDebug() << "New Client!";
    connect(client, SIGNAL(readyRead()), this, SLOT(startRead()));
}

void MainWindow::startRead()
{
    QDataStream in(client);
    in.setVersion(QDataStream::Qt_4_0);

    if (rawTweetSize == 0)
    {
        if (client->bytesAvailable() < (int)sizeof(quint32))
            return;
        in >> rawTweetSize;
    }

    if (client->bytesAvailable() < rawTweetSize)
        return;
//    bool message_complete = false;
//    if (client->bytesAvailable() + rawTweet.size() >= rawTweetSize)
//        message_complete = true;

    in >> rawTweet;

    qDebug() << "rawsize  : " << rawTweetSize;
    qDebug() << "rawTweet : " << rawTweet;
//    if (message_complete)
//    {
//    }
    readTweet(rawTweet);
    rawTweetSize = 0;
    rawTweet.clear();
//    while(client->bytesAvailable())
//    {
//        Json.append(client->readLine());
//        qDebug() << Json;
//    }
//    readTweet(Json);
}

void MainWindow::readTweet(QByteArray tweetByte)
{
//    rawTweet.append(tweetByte);
//    qDebug()<< rawTweet;
//    parseTweet();
    qDebug() << tweetByte;
    QJsonDocument* response = new QJsonDocument(QJsonDocument::fromJson(tweetByte));
    if (!response->isEmpty())
    {
        qDebug() << "Json Valid";
        qDebug() << response->toJson();
        addTweet(response);;
    }
    else
    {
        qDebug() << "Json NOT Valid";
    }
}

void MainWindow::parseTweet()
{
    QStringList responseSplit = QString(rawTweet).split(QRegExp("(\r\n)"),QString::SkipEmptyParts);
    qDebug() << "split : " << responseSplit.size();

    foreach (QString split, responseSplit) {
        if (rawTweetSize == 0)
        {
            rawTweetSize = split.toInt();
            qDebug() << "Length = " << rawTweetSize;
        }
        else
        {
            parsedTweet.append(split);
            if (parsedTweet.size() >= rawTweetSize - 3)
            {
                QJsonDocument* response = new QJsonDocument(QJsonDocument::fromJson(parsedTweet.toLatin1()));
                if (!response->isEmpty())
                {
                    qDebug() << "Json Valid";
                    qDebug() << response->toJson();
                    addTweet(response);;
                }
                else
                {
                    qDebug() << "Json NOT Valid";
                }
                rawTweetSize = 0;
                parsedResponse.clear();
            }
        }
    }
    rawTweet.clear();
}



void MainWindow::addTweet(QJsonDocument* JsonTweet)
{
//     = new QJsonDocument();

//    QNetworkReply *replyEmbed = qobject_cast<QNetworkReply *>(sender());
//    if (replyEmbed->error() != QNetworkReply::NoError) {
//        qDebug() << "ERROR:" << replyEmbed->errorString();
//        qDebug() << "content:" << replyEmbed->readAll();
//    } else {
//        qDebug() << "embed Json retrieved!";
//        QByteArray rawEmbedTweet = replyEmbed->readAll();
//        qDebug() << rawEmbedTweet;
//        embedJsonTweet->fromJson(rawEmbedTweet);
//    }

//    QByteArray tweetByteProper = QString(tweetByte).toUtf8();
//    QJsonParseError* JsonError = new QJsonParseError();
//    QJsonDocument* JsonTweet = new QJsonDocument(QJsonDocument::fromJson(tweetByteProper, JsonError));


    if (JsonTweet->isEmpty())
    {
        qDebug() << "Empty? " << JsonTweet->isEmpty();
//        qDebug() << JsonError->errorString();
        return;
    }

    JsonParser Parser = JsonParser(JsonTweet);


    QUrl avatar;

    QString tweet_user = Parser.parseJson("user/name");
    QString retweeted_user = Parser.parseJson("retweeted_status/user/name");

    QString user;
    QString rt_user;
    bool retweet = !retweeted_user.isEmpty();

    if (retweet)
    {
        avatar = QUrl(Parser.parseJson("retweeted_status/user/profile_image_url"));
        user = retweeted_user;
        rt_user = user;
    }
    else
    {
        avatar = QUrl(Parser.parseJson("user/profile_image_url"));
        user = tweet_user;
    }

    QString media = Parser.parseJson("entities/media/0/media_url");
//    qDebug() << media;

    QString Text = Parser.parseJson("text");

    TweetWidget* newTweet = new TweetWidget(manager, ui->Widget);

    newTweet->setData(avatar,user,rt_user, Text);

    newTweet->setContentSize(contentWidth, contentHeigh);


    if (media.size())
        newTweet->setContent(media);

    ui->TwitterFeed->insertWidget(0, newTweet);

    newTweet->show();

    while (ui->TwitterFeed->count() > 10) {
        int position = ui->TwitterFeed->count() - 2;
        QLayoutItem* lastTweet = ui->TwitterFeed->itemAt(position);
        removeTweet(lastTweet, position);

    }

}

void MainWindow::removeTweet(QLayoutItem* item, int position)
{
    ui->TwitterFeed->takeAt(position);
    delete item->widget();
    delete item;
}

void MainWindow::clearTweets()
{
    int children = ui->TwitterFeed->layout()->count();
    for(int i=0; i < children - 1; i++) {
        QLayoutItem* item = ui->TwitterFeed->itemAt(0);
        removeTweet(item);
    }
}

