// #include "violetclient.h"
// #include "usermainwindow.h"
// #include <QApplication>
// int main(int argc, char *argv[])
// {
//     QApplication a(argc, argv);
//     std::vector<std::string> c, v;
//     c = {"qwe", "q123"};
//     v = {"123", "ggc"};
//     UserMainWindow w(nullptr, std::string("test"), c, v);
//     w.show();
//     return a.exec();
// }

// 测文件传输
// #include <QApplication>
// #include <QTcpSocket>
// #include "pvp.h"
// int main(int argc, char *argv[])
// {
//     QApplication a(argc, argv);
//     QTcpSocket *sock;
//     QString tmp("1");
//     PvP w(sock, tmp, "1", ChatType::LGG);
//     w.show();
//     return a.exec();
// }



#include "violetclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (!QSslSocket::supportsSsl())
    {
        qCritical() << "This system does not support SSL/TLS.";
        return -1;
    }
    // qDebug() << "QSslSocket::sslLibraryBuildVersionString(): " << QSslSocket::sslLibraryBuildVersionString();
    // qDebug() << "QSslSocket::supportsSsl(): " << QSslSocket::supportsSsl();
    VioletClient w;
    w.show();
    return a.exec();
}
