#include "violetclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qInstallMessageHandler(VioletClient::fileMessageHandler);
    QApplication a(argc, argv);
    if (!QSslSocket::supportsSsl())
    {
        qCritical() << "This system does not support SSL/TLS.";
        return -1;
    }
    // --- 添加这几行来检查 SSL 后端 ---
    qDebug() << "=== Qt SSL Backend Information ===";
    qDebug() << "Active (currently in use) SSL Backend:" << QSslSocket::activeBackend();
    qDebug() << "Available (supported) SSL Backends:" << QSslSocket::availableBackends();
    qDebug() << "===================================";

    // qDebug() << "QSslSocket::sslLibraryBuildVersionString(): " << QSslSocket::sslLibraryBuildVersionString();
    // qDebug() << "QSslSocket::supportsSsl(): " << QSslSocket::supportsSsl();
    VioletClient w;
    w.show();
    return a.exec();
}
