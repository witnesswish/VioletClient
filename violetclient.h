#ifndef VIOLETCLIENT_H
#define VIOLETCLIENT_H

#include <QMainWindow>
#include <cstdlib>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTcpsocket>
#include <QStackedLayout>
#include <QStandardPaths>
#include <QDir>

#include "protocol.h"
#include "unlogincenter.h"
#include "sqlitehelper.h"
#include "logincenter.h"
#include "usermainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class VioletClient;
}
QT_END_NAMESPACE

class VioletClient : public QMainWindow
{
    Q_OBJECT

public:
    VioletClient(QWidget *parent = nullptr);
    ~VioletClient();

public:
    void connectNetwork(QTcpSocket *sock);

private slots:
    void on_btn_reConnect_pressed();

    void on_btnLogin_pressed();

    void on_btnWild_pressed();

private:
    Ui::VioletClient *ui;
    QTcpSocket *sock;
    UnloginCenter *unlogin;
    LoginCenter *loginCenter;
    sqliteHelper db;
    QString path;

private:
    void closeEvent(QCloseEvent *event);
    void read_cb();
    void gotoUnlogin(int);
};
#endif // VIOLETCLIENT_H
