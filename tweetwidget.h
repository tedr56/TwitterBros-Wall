#ifndef TWEETWIDGET_H
#define TWEETWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Ui {
class TweetWidget;
}

class TweetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TweetWidget(QNetworkAccessManager* netmanager, QWidget *parent = 0);
    ~TweetWidget();

    void setData(QUrl Avatar, QString User, QString ReUser, QString Text);
    void setAvatar(QUrl Avatar);
    void setContent(QUrl content);
    void setContentSize(int w, int h);

public slots:
    void avatarReceived();
    void contentReceived();

protected:
    QString parseHashTag(QString text);
    QString parseArobace(QString text);
private:
    Ui::TweetWidget *ui;
    QNetworkAccessManager* networkManager;
    QNetworkReply* avatarReply;
    QNetworkReply* contentReply;
    QPixmap avatarPix;
    QPixmap contentPix;
};

#endif // TWEETWIDGET_H
