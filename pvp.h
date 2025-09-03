#ifndef PVP_H
#define PVP_H

#include <QCloseEvent>
#include <QSslSocket>
#include <QTextBrowser>
#include <QTime>
#include <QDateTime>
#include <QWidget>
#include <QTextTableCell>
#include <QTextTableCellFormat>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QStandardPaths>
#include <QShowEvent>

#include "protocol.h"
#include "sqlitehelper.h"

/**
 *这个类必须复用，我不想再写了
 */
/**
 * @brief The ChatType enum
 * UL: UNLOGIN
 * LG: LOGIN
 * P: PRIVTE CHAT
 * G: GROUP CHAT
 * @LPC reading prichat cache
 */
enum ChatType
{
    ULP,
    ULG,
    LGP,
    LGG,
    LPC,
};

namespace Ui {
class PvP;
}

class PvP : public QWidget
{
    Q_OBJECT

public:
    explicit PvP(QSslSocket *socket, QString &uid, const QString loginUsername, ChatType itype, QWidget *parent = nullptr);
    ~PvP();

private:
    SRHelper sr;
    Ui::PvP *ui;
    QSslSocket *sock;
    QString m_uid;
    QString m_loginUsername;
    ChatType type;
    sqliteHelper db;
    QString path;
    QString externalPrivatePath;
    QString externalPublicPath;

private slots:
    void onMessageDispatched(QObject *receiver, const QString &uid, const QString &uid2, const QString &message);
    void on_send_pressed();
    void on_text_returnPressed();
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void displayAppend(QTextBrowser *textBrowser, const QString &username, const QString &avatarPath, const QString &timeHole,
                       const QString &text, Qt::Alignment align = Qt::AlignLeft);

    void on_sned_file_pressed();
    void init(QTextBrowser *t);

signals:
    void m_close();
};

class Worker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(const QString parameter);

signals:
    void resultReady(const QString &result);
    void finished();
};


#endif // PVP_H
