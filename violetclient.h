#ifndef VIOLETCLIENT_H
#define VIOLETCLIENT_H

#include <QMainWindow>
#include <cstdlib>
#include <QMessageBox>
#include <QCloseEvent>
#include <QStackedLayout>
#include <QStandardPaths>
#include <QDir>
#include <QtNetwork>
#include <QSslSocket>
#include <QtGlobal>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

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
    void connectNetwork(QSslSocket *sock);

private slots:
    void on_btn_reConnect_pressed();

    void on_btnLogin_pressed();

    void on_btnWild_pressed();

private:
    Ui::VioletClient *ui;
    QSslSocket *sslSock;
    UnloginCenter *unlogin;
    LoginCenter *loginCenter;
    sqliteHelper db;
    QString path;
    QString externalPrivatePath;
    QString externalPublicPath;
    static QMutex logMutex;
    const QString PreStoredServerCertFingerprint = "C2D69078B46A737D73D044BB493B9EA3734B2FD52278D25F82BF1C47704FB67D";
private:
    void closeEvent(QCloseEvent *event);
    void read_cb();
    void gotoUnlogin(int);
    void sslErrorHandle(const QList<QSslError> &errors);
    static void fileMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};
#endif // VIOLETCLIENT_H
