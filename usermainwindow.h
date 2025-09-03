#ifndef USERMAINWINDOW_H
#define USERMAINWINDOW_H

#include <QWidget>
#include <QtNetwork>
#include <QSslSocket>
#include <QCloseEvent>
#include <string>
#include <vector>
#include <QInputDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QList>
#include <QPaintEvent>
#include <QTabWidget>
#include <QStandardPaths>
#include <QDir>

#include "protocol.h"
#include "pvp.h"
#include "sqlitehelper.h"

struct LoginUserWindows
{
    PvP *pvp;
    QString uid;
};

struct  UserRecvBufferCache
{
     std::string uname;
     ssize_t expectLen;
     ssize_t actuaLen;
     std::vector<char> recvBuffer;
};

namespace Ui {
class UserMainWindow;
}

class UserMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit UserMainWindow(QSslSocket *socket, std::string uname, std::vector<std::string> userfriend, std::vector<std::string> usergroup, QWidget *parent = nullptr);
    ~UserMainWindow();
public:
signals:
    void sclose();
private slots:

    void on_ubAddFriend_pressed();

    void on_ubAddGroup_pressed();

    void on_ubCreateGroup_pressed();

    void on_uFriendList_itemDoubleClicked(QListWidgetItem *item);

    void on_uGroupList_itemDoubleClicked(QListWidgetItem *item);

    void on_uRecentList_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::UserMainWindow *ui;
    QSslSocket *sock;
    SRHelper sr;
    std::string m_username;
    std::vector<std::string> userfriend, usergroup;
    PvP *priChat;
    sqliteHelper db;
    QString path;
    QString externalPrivatePath;
    QString externalPublicPath;
    QList<LoginUserWindows> pvpWindows;
    static std::map<std::string, UserRecvBufferCache> userRecvBuffCache;
private:
    void read_cb();
    void openWindow(QString name, ChatType itype, bool isSrv);
protected:
    void closeEvent(QCloseEvent *ev) override;
    void paintEvent(QPaintEvent* event) override
    {
        QWidget::paintEvent(event);
    }
};

#endif // USERMAINWINDOW_H
