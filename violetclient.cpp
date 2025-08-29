/**
 * 选择使用匿名模式还是登录模式
 * 根据选择将sock传递过去，并且把自身connect的readready信号断开，由接下来的部件处理
 * 应用初始化，确保需求目录目录存在
 * 创建数据表，每次链接都更新或者创建，因为每一次的id都可能不同
**/
#include "violetclient.h"
#include "ui_violetclient.h"
#include "messagedispatcher.h"

VioletClient::VioletClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VioletClient)
{
    setWindowIcon(QIcon(":/resources/violet.ico"));
    setAttribute(Qt::WA_Mapped);
    ui->setupUi(this);
    path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    externalPrivatePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);    ///Android/data/<package_name>/
    externalPublicPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);     ///storage/emulated/0/
    QDir().mkpath(path);
    QDir().mkpath(externalPrivatePath);

    sock = new QTcpSocket(this);
    connectNetwork(sock);
    ui->networkSuccess->hide();
    ui->btn_reConnect->hide();
}
void VioletClient::gotoUnlogin(int uid) {
    qDebug()<< uid;
    unlogin = new UnloginCenter(sock, uid);
    if(!db.openDatabase(path+"/Violet.db"))
    {
        qDebug()<< "open db error";
    }
    {
        db.createTable("me", "id INTEGER PRIMARY KEY, name TEXT NOT NULL, email TEXT UNIQUE, age INTEGER");
        qDebug()<< uid;
        db.deleteRecord("me", "1=1;");
        QVariantMap me;
        me["id"] = uid;
        me["name"] = "self";
        me["email"] = "vu1@elveso.asia";
        me["age"] = 29;
        db.insertRecord("me", me);
    }
    if(db.isOpen())
    {
        db.closeDatabase();
    }
    connect(unlogin, &UnloginCenter::sclose, this, [=](){this->show();connect(sock, &QTcpSocket::readyRead, this, &VioletClient::read_cb);});
    disconnect(sock, &QTcpSocket::readyRead, this, &VioletClient::read_cb);
    this->hide();
    unlogin->show();
}
/**
 * @brief VioletClient::~VioletClient
 * 析构函数中手动关闭socket以防万一
 */
VioletClient::~VioletClient()
{
    delete ui;
}

void VioletClient::connectNetwork(QTcpSocket *sock)
{
    ui->lb_netStatus->setText("connecting...");
    connect(sock, &QAbstractSocket::errorOccurred, [=](QAbstractSocket::SocketError error) {
        qDebug() << "Socket error:" << error; // 枚举值
        qDebug() << "Error details:" << sock->errorString(); // 错误描述
        ui->lb_netStatus->setText(sock->errorString());
        ui->networkSuccess->hide();
        ui->btn_reConnect->show();
    });
    sock->QTcpSocket::connectToHost("elveso.asia", 46836);
    connect(sock, &QTcpSocket::connected, this, [this](){ui->networkFailure->hide();ui->networkSuccess->show();ui->lb_netStatus->setText("connect success");});
    connect(sock, &QTcpSocket::disconnected, this, [this](){
        foreach(QWidget* widget, QApplication::topLevelWidgets()) {
            if(widget != this) {
                widget->close();
            }
        }
        this->show();
        ui->networkFailure->show();
        ui->btn_reConnect->show();
        ui->lb_netStatus->setText("connection broken");
    });
    connect(sock, &QTcpSocket::readyRead, this, &VioletClient::read_cb);
}

void VioletClient::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this,
                                                               "quit",
                                                               "Are you sure to quit?",
                                                               QMessageBox::Cancel
                                                                   | QMessageBox::Yes,
                                                               QMessageBox::Yes);

    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        if(sock)
        {
            sock->disconnectFromHost();
            sock->deleteLater();
        }
        event->accept();
    }
}

void VioletClient::read_cb()
{
    SRHelper sr;
    auto packet = sr.recvMsg(sock, -1);
    if(packet == std::nullopt)
    {
        qDebug()<<"error read";
        return;
    }
    qDebug()<< "read: " << QString::fromStdString(std::string(packet->content.data(), packet->content.size()));
    if(QString(packet->neck.command) == QString("nonsucc"))
    {
        int x = QString(packet->neck.name).toInt();
        gotoUnlogin(x);
    }
}

void getEnv()
{
    const char *db_name = std::getenv("db_name");
    if (db_name != nullptr) {
        qDebug() << db_name;
    }
}

void VioletClient::on_btn_reConnect_pressed() {
    ui->btn_reConnect->hide();
    connectNetwork(sock);
}

void VioletClient::on_btnWild_pressed() {
    VioletProtNeck neck = {};
    neck.unlogin = true;
    strcpy(neck.command, "nonreq");
    SRHelper sr;
    sr.sendMsg(sock, neck, "violet");
}


void VioletClient::on_btnLogin_pressed() {
    disconnect(sock, &QTcpSocket::readyRead, this, &VioletClient::read_cb);
    loginCenter = new LoginCenter(sock);
    //登录失败或者没登录成功直接退出发送sclose，登录成功的话需要监听另一个sclose，怎么做呢，在另一边做吧，可是这里注定是要解绑的啊，如果登录成功了
    connect(loginCenter, &LoginCenter::sclose, this, [=](){this->show();connect(sock, &QTcpSocket::readyRead, this, &VioletClient::read_cb);});
    //sloged代表登录成功，窗体已经删除，监听另一个信号
    connect(loginCenter, &LoginCenter::sloged, this, [=](UserMainWindow *umw){
        connect(umw, &UserMainWindow::sclose, this, [=](){this->show();connect(sock, &QTcpSocket::readyRead, this, &VioletClient::read_cb);});
    });
    this->hide();
    loginCenter->show();
}
