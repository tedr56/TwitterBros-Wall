#include "tweetwidget.h"
#include "ui_tweetwidget.h"

TweetWidget::TweetWidget(QNetworkAccessManager* netmanager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TweetWidget)
{
    networkManager = netmanager;
//    connect( networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(avatarReceived(QNetworkReply*)));
    ui->setupUi(this);
}

TweetWidget::~TweetWidget()
{
    delete ui;
//    avatarReply->deleteLater();
//    contentReply->deleteLater();

}

void TweetWidget::setData(QUrl Avatar, QString User, QString ReUser, QString Text)
{
    ui->User->setText("<html><head/><body><p><span style=\" font-weight:600; color:#35bfbf;\">" + User + "</span></p></body></html>");
    if (ReUser.size())
        ui->Retweeter->setText("<html><head/><body><p>Retweeté par <span style=\" font-weight:600; color:#35bfbf;\">" + ReUser + "</span></p></body></html>");
    else
        ui->Retweeter->setText("");

    Text = parseHashTag(Text);
    Text = parseArobace(Text);

    //ui->Tweet->setText(Text);
    ui->Tweet->setText("<html><head/><body><p><span style=\" font-size:14pt;\">" + Text + "</span></p></body></html>");

    ui->Avatar->setText(User);
    setAvatar(Avatar);
}

void TweetWidget::setAvatar(QUrl Avatar)
{
    avatarReply = networkManager->get(QNetworkRequest(Avatar));
    connect(avatarReply, SIGNAL(finished()), this, SLOT(avatarReceived()));
}

void TweetWidget::setContent(QUrl Content)
{
    contentReply = networkManager->get(QNetworkRequest(Content));
    connect(contentReply, SIGNAL(finished()), this, SLOT(contentReceived()));
}

void TweetWidget::setContentSize(int w, int h)
{
    ui->Content->setMaximumSize(w,h);
}

void TweetWidget::avatarReceived()
{
    if (QNetworkReply::NoError != avatarReply->error()) {
        qDebug() << Q_FUNC_INFO << "pixmap receiving error" << avatarReply->error();
        avatarReply->deleteLater();
        return;
    }

    const QByteArray data(avatarReply->readAll());
    if (!data.size())
        qDebug() << Q_FUNC_INFO << "received pixmap looks like nothing";

    avatarPix.loadFromData(data);
    ui->Avatar->setPixmap(avatarPix);
    ui->Avatar->setScaledContents(true);
    disconnect(avatarReply, SIGNAL(finished()), this, SLOT(avatarReceived()));
    avatarReply->deleteLater();
}

void TweetWidget::contentReceived()
{
    if (QNetworkReply::NoError != contentReply->error()) {
        qDebug() << Q_FUNC_INFO << "pixmap receiving error" << contentReply->error();
        contentReply->deleteLater();
        return;
    }

    const QByteArray data(contentReply->readAll());
    if (!data.size())
        qDebug() << Q_FUNC_INFO << "received pixmap looks like nothing";

    contentPix.loadFromData(data);
    contentPix = contentPix.scaled(ui->Content->maximumSize(), Qt::KeepAspectRatio);
    ui->Content->setPixmap(contentPix);

    disconnect(contentReply, SIGNAL(finished()), this, SLOT(contentReceived()));
    contentReply->deleteLater();
}

QString TweetWidget::parseHashTag(QString text)
{
    QString reg;
    QString out;
    reg = "((?:#)(\\w+))";
    out = "<span style=\" font-weight:600; color:#35bfbf;\">\\1</span>";
    QString replaced = text.replace(QRegExp(reg), out);
    return replaced;
}

QString TweetWidget::parseArobace(QString text)
{
    QString reg;
    QString out;
    reg = "((?:@)(\\w+))";
    out = "<span style=\" font-weight:600; color:#35bfbf;\">\\1</span>";
    QString replaced = text.replace(QRegExp(reg), out);
    return replaced;
}
