#include "logincenter.h"
#include "ui_logincenter.h"

#include <string.h>
#include <sstream>

LoginCenter::LoginCenter(QSslSocket *socket, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginCenter)
{
    setWindowIcon(QIcon(":/resources/violet.ico"));
    ui->setupUi(this);
    setAttribute(Qt::WA_Mapped);
    sock =  socket;
    connect(sock, &QSslSocket::readyRead, this, &LoginCenter::read_cb);
    setAttribute(Qt::WA_DeleteOnClose);
    isLogin = false;
    QAction *toggleAction = new QAction(this);
    toggleAction->setIcon(QIcon(":/resources/icons8-closeEye-24.png"));
    toggleAction->setCheckable(true);
    toggleAction->setToolTip("显示密码");

    // 将动作添加到行编辑框的右侧
    ui->password->addAction(toggleAction, QLineEdit::TrailingPosition);

    // 连接信号槽
    connect(toggleAction, &QAction::toggled, this, [this, toggleAction]() {
        if (toggleAction->isChecked()) {
            ui->password->setEchoMode(QLineEdit::Normal);
            toggleAction->setIcon(QIcon(":/resources/icons8-openEye-24.png"));
            toggleAction->setToolTip("隐藏密码");
        } else {
            ui->password->setEchoMode(QLineEdit::Password);
            toggleAction->setIcon(QIcon(":/resources/icons8-closeEye-24.png"));
            toggleAction->setToolTip("显示密码");
        }
    });
}

LoginCenter::~LoginCenter()
{
    delete ui;
}

void LoginCenter::on_vlogin_pressed()
{
    QString username = ui->username->text().trimmed();
    QString password = ui->password->text().trimmed();
    if(username.isEmpty() || password.isEmpty())
    {
        ui->result->setAlignment(Qt::AlignCenter);
        ui->result->setStyleSheet(QString("QLabel { color: red; }"));
        ui->result->setText("账号/密码为空，请重试");
        return;
    }
    VioletProtNeck neck = {};
    strcpy(neck.command, "vlogin");
    memcpy(neck.name, username.toStdString().c_str(), sizeof(neck.name));
    memcpy(neck.pass, password.toStdString().c_str(), sizeof(neck.pass));
    QString tmp("violet");
    sr.sendMsg(sock, neck, tmp.toStdString());
}

void LoginCenter::on_vregister_pressed() {
    disconnect(sock, &QSslSocket::readyRead, this, &LoginCenter::read_cb);
    reg = new RegisterViolet(sock);
    connect(reg, &RegisterViolet::sclose, this, [this](){this->show();connect(sock, &QSslSocket::readyRead, this, &LoginCenter::read_cb);});
    this->hide();
    reg->show();
}

void LoginCenter::read_cb() {
    auto ret = sr.recvMsg(sock, -1);
    QString command(ret->neck.command);
    QString content = QString::fromStdString(std::string(ret->content.begin(), ret->content.end()));
    if(command == "vloginerr")
    {
        ui->result->setAlignment(Qt::AlignCenter);
        ui->result->setStyleSheet(QString("QLabel { color: red; }"));
        ui->result->setText(QString("登录失败 %1").arg(content));
        return;
    }
    if(command == "vloginsucc")
    {
        QString uname(ret->neck.name);
        std::string content(ret->content.begin(), ret->content.end());
        std::vector<std::string> userfriend, usergroup;
        //这里要记得先好友再群组，服务器的时候，如果出问题希望自己能看到这个注释
        auto userinfo = deserializeVectors(content);
        userfriend = userinfo.first;
        usergroup = userinfo.second;
        user = new UserMainWindow(sock, uname.toStdString(), userfriend, usergroup);
        emit sloged(user);
        isLogin = true;
        user->show();
        this->deleteLater();
        this->close();
    }
}

void LoginCenter::closeEvent(QCloseEvent *ev) {
    disconnect(sock, &QSslSocket::readyRead, this, &LoginCenter::read_cb);
    if(!isLogin)
    {
        emit sclose();
    }
    ev->accept();
}


std::string LoginCenter::serializeTwoVector(const std::vector<std::string> &vec1, const std::vector<std::string> &vec2)
{
    std::ostringstream oss;
    uint32_t size1 = htonl(vec1.size());
    oss.write(reinterpret_cast<const char*>(&size1), sizeof(size1));
    for(const auto &str : vec1)
    {
        uint32_t len = htonl(str.size());
        oss.write(reinterpret_cast<const char*>(&len), sizeof(len));
        oss.write(str.data(), str.size());
    }
    uint32_t size2 = htonl(vec2.size());
    oss.write(reinterpret_cast<const char*>(&size2), sizeof(size2));
    for(const auto &str : vec2)
    {
        uint32_t len = htonl(str.size());
        oss.write(reinterpret_cast<const char*>(&len), sizeof(len));
        oss.write(str.data(), str.size());
    }
    return oss.str();
}



std::pair<std::vector<std::string>, std::vector<std::string> > LoginCenter::deserializeVectors(const std::string &data)
{
    std::vector<std::string> vec1, vec2;
    const char* ptr = data.data();
    const char* end = ptr + data.size();
    if(ptr + sizeof(uint32_t) > end)
    {
        return {vec1, vec2};
    }
    uint32_t size1;
    memcpy(&size1, ptr, sizeof(size1));
    ptr += sizeof(size1);
    size1 = ntohl(size1);
    vec1.reserve(size1);
    for(uint32_t i=0; i<size1; ++i)
    {
        if (ptr + sizeof(uint32_t) > end)
            break;
        uint32_t len;
        memcpy(&len, ptr, sizeof(len));
        ptr += sizeof(len);
        len = ntohl(len);
        if (ptr + len > end) break;
        vec1.emplace_back(ptr, len);
        ptr += len;
    }
    if(ptr + sizeof(uint32_t) > end)
    {
        return {vec1, vec2};

    }
        uint32_t size2;
        memcpy(&size2, ptr, sizeof(size2));
        ptr += sizeof(size2);
        size2 = ntohl(size2);
        vec2.reserve(size2);
        for(uint32_t i=0; i<size2; ++i)
        {
            if (ptr + sizeof(uint32_t) > end)
                break;
            uint32_t len;
            memcpy(&len, ptr, sizeof(len));
            ptr += sizeof(len);
            len = ntohl(len);
            if (ptr + len > end)
                break;
            vec2.emplace_back(ptr, len);
            ptr += len;
        }
    return {vec1, vec2};
}


void LoginCenter::on_password_returnPressed()
{
    on_vlogin_pressed();
}

