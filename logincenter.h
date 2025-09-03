#ifndef LOGINCENTER_H
#define LOGINCENTER_H

#include <QWidget>
#include <QSslSocket>
#include <QCloseEvent>

#include "registerviolet.h"
#include "protocol.h"
#include "usermainwindow.h"

namespace Ui {
class LoginCenter;
}

class LoginCenter : public QWidget
{
    Q_OBJECT

public:
    explicit LoginCenter(QSslSocket *socket, QWidget *parent = nullptr);
    ~LoginCenter();
signals:
    void sclose();
    void sloged(UserMainWindow *umw);

private slots:
    void on_vlogin_pressed();
    void on_vregister_pressed();

    void on_password_returnPressed();

private:
    Ui::LoginCenter *ui;
    RegisterViolet *reg;
    QSslSocket *sock;
    SRHelper sr;
    UserMainWindow *user;
    bool isLogin;
    std::string serializeTwoVector(const std::vector<std::string>& vec1, const std::vector<std::string>& vec2);
    std::pair<std::vector<std::string>, std::vector<std::string>> deserializeVectors(const std::string& data);
private:
    void read_cb();
    void closeEvent(QCloseEvent *ev);
};

#endif // LOGINCENTER_H
