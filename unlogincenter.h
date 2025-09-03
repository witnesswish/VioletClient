#ifndef UNLOGINCENTER_H
#define UNLOGINCENTER_H

#include <QCloseEvent>
#include <QDesktopServices>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslSocket>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCursor>
#include <QWidget>
#include <QTextTableCell>
#include <QTextTableCellFormat>
#include <QList>

#include "pvp.h"

namespace Ui {
class UnloginCenter;
}

struct UserWindows
{
    PvP *pvp;
    QString uid;
};

class UnloginCenter : public QWidget
{
    Q_OBJECT

public:
    explicit UnloginCenter(QSslSocket *socket, int uid, QWidget *parent = nullptr);
    ~UnloginCenter();

private:
    Ui::UnloginCenter *ui;
    QSslSocket *sock;
    PvP *priChat;
    QList<UserWindows> pvpWindows;
    int x = 0;

signals:
    void sclose();
    void textClicked(const QString &clickedText);
    void textDoubleClicked(const QString &text);
private slots:
    void on_send_pressed();

    void on_text_returnPressed();

    void on_enter_pressed();

    void on_lwHome_itemDoubleClicked(QListWidgetItem *item);

    void on_display_anchorClicked(const QUrl &arg1);

private:
    void closeEvent(QCloseEvent *event) override;
    void displayAppend(QTextBrowser *textBrowser,
                       const QString &username,
                       const QString &avatarPath,
                       const QString &text,
                       Qt::Alignment align = Qt::AlignLeft);
    void read_cb();
    void clickedUser(QString username, bool isfromSrv);
};

#endif // UNLOGINCENTER_H
