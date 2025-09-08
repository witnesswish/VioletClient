/**
 * 选择使用匿名模式还是登录模式
 * 根据选择将sock传递过去，并且把自身connect的readready信号断开，由接下来的部件处理
 * 应用初始化，确保需求目录目录存在
 * 创建数据表，每次链接都更新或者创建，因为每一次的id都可能不同
**/
#include "violetclient.h"
#include "ui_violetclient.h"
#include "common.h"

QMutex VioletClient::logMutex;

VioletClient::VioletClient(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VioletClient)
{
    qInstallMessageHandler(fileMessageHandler);
    setWindowIcon(QIcon(":/resources/violet.ico"));
    setAttribute(Qt::WA_Mapped);
    ui->setupUi(this);
    path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    externalPrivatePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);    ///Android/data/<package_name>/
    externalPublicPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);     ///storage/emulated/0/
    QDir().mkpath(path);
    QDir().mkpath(externalPrivatePath);
    sslSock = new QSslSocket(this);

    // 加载客户端证书
    QFile certFile(":/certs/resources/certs/client.crt");
    if (!certFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open client certificate file";
        return;
    }
    QSslCertificate clientCertificate(&certFile);
    certFile.close();
    // 加载客户端私钥
    QFile keyFile(":/certs/resources/certs/client.key");
    if (!keyFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open client private key file";
        return;
    }
    QSslKey clientPrivateKey(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
    keyFile.close();
    // 检查是否加载成功
    if (clientCertificate.isNull()) {
        qDebug() << "Failed to load client certificate";
        return;
    }
    if (clientPrivateKey.isNull()) {
        qDebug() << "Failed to load client private key";
        return;
    }
    // 将证书和私钥设置到QSslSocket
    sslSock->setLocalCertificate(clientCertificate);
    sslSock->setPrivateKey(clientPrivateKey);

    // // 将证书数据转换为QSslCertificate对象
    // QSslCertificate certificate(certData, QSsl::Pem); // 明确指定PEM格式
    // if (certificate.isNull())
    // {
    //     qCritical() << "加载的证书无效或格式不正确。";
    //     // 错误处理
    // }
    QSslConfiguration sslConfig = sslSock->sslConfiguration();
    // 将证书添加到CA证书列表（用于验证服务器）
    //QList<QSslCertificate> caCerts = sslConfig.caCertificates();
    // caCerts.append(certificate);
    //sslConfig.setCaCertificates(caCerts);
    // sslConfig.setProtocol(QSsl::TlsV1_2); // 设置协议版本
    // 设置对等验证模式为VerifyPeer（要求验证服务器证书）
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
    // 对于自签名证书，你可能还需要设置验证深度
    // 自签名证书没有中间CA，深度设为1
    sslConfig.setPeerVerifyDepth(1);
    sslSock->setSslConfiguration(sslConfig);

    connect(sslSock, &QSslSocket::encrypted, this, []() {qDebug() << "SSL加密连接已成功建立！";});
    connectNetwork(sslSock);
    ui->networkSuccess->hide();
    ui->btn_reConnect->hide();
}
void VioletClient::gotoUnlogin(int uid) {
    qDebug()<< uid;
    unlogin = new UnloginCenter(sslSock, uid);
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
    connect(unlogin, &UnloginCenter::sclose, this, [=](){this->show();connect(sslSock, &QSslSocket::readyRead, this, &VioletClient::read_cb);});
    disconnect(sslSock, &QSslSocket::readyRead, this, &VioletClient::read_cb);
    this->hide();
    unlogin->show();
}

void VioletClient::sslErrorHandle(const QList<QSslError> &errors)
{
    bool foundFatalError = false;
    bool certificateVerified = false;
    for (const QSslError &error : errors)
    {
        qDebug() << "SSL Error:" << error.errorString();
        // 检查错误类型，如果是证书不匹配等严重错误，记录下来
        if (error.error() != QSslError::SelfSignedCertificate && error.error() != QSslError::HostNameMismatch && error.error() != QSslError::CertificateUntrusted)
        {
            foundFatalError = true;
            qCritical() << "Fatal SSL Error:" << error.errorString();
        }
    }
    // 如果不是致命的错误（自签名证书），则进行指纹验证
    if (!foundFatalError)
    {
        // 获取对等方（服务器）的证书链
        QList<QSslCertificate> peerCerts = sslSock->peerCertificateChain();
        if (!peerCerts.isEmpty())
        {
            // 通常链中的第一个证书是服务器自身的证书
            QSslCertificate serverCert = peerCerts.first();
            // 计算服务器证书的SHA256指纹
            QByteArray certDigest = serverCert.digest(QCryptographicHash::Sha256).toHex().toUpper();
            QString receivedFingerprint = QString::fromLatin1(certDigest);

            qDebug() << "Received Server Certificate Fingerprint (SHA256):" << receivedFingerprint;
            qDebug() << "Stored Server Certificate Fingerprint (SHA256):" << PreStoredServerCertFingerprint;

            // 比较指纹
            if (receivedFingerprint == PreStoredServerCertFingerprint)
            {
                qDebug() << "Server certificate fingerprint matches! Proceeding.";
                certificateVerified = true;
                // 明确忽略这些SSL错误，继续连接
                sslSock->ignoreSslErrors(); // 忽略所有错误，因为完成指纹，就算再离谱，也验证过指纹
            }
            else
            {
                qCritical() << "Server certificate fingerprint DOES NOT MATCH! Aborting connection.";
            }
        }
        else
        {
            qCritical() << "No server certificate received!";
        }
    }
    if (foundFatalError || !certificateVerified)
    {
        qCritical() << "计划有变，严重错误或者证书指纹不匹配，取消交易";
        sslSock->abort();
    }
}

void VioletClient::fileMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&logMutex);
    QString externalPrivatePath;
#ifdef Q_OS_ANDROID
    externalPrivatePath = getAndroidPath(ANDROIDPATH_TYPE::EXTPRIVATE);
#else
    externalPrivatePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#endif
    QFile file(externalPrivatePath+"/app.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        return;
    }

    QTextStream stream(&file);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 根据消息类型添加前缀
    switch (type)
    {
    case QtDebugMsg: stream << timestamp << " [DEBUG] "; break;
    case QtInfoMsg: stream << timestamp << " [INFO] "; break;
    case QtWarningMsg: stream << timestamp << " [WARN] "; break;
    case QtCriticalMsg: stream << timestamp << " [ERROR] "; break;
    case QtFatalMsg: stream << timestamp << " [FATAL] "; break;
    }

    stream << msg << "\n";
    file.close();
}
/**
 * @brief VioletClient::~VioletClient
 * 析构函数中手动关闭socket以防万一
 */
VioletClient::~VioletClient()
{
    delete ui;
}

void VioletClient::connectNetwork(QSslSocket *sock)
{
    sock->connectToHostEncrypted("elveso.asia", 46836);
    ui->lb_netStatus->setText("connecting...");
    connect(sock, &QSslSocket::errorOccurred, this, [=](QSslSocket::SocketError error)
    {
        qDebug() << "Socket error:" << error;
        qDebug() << "Error details:" << sock->errorString();
        ui->lb_netStatus->setText(sock->errorString());
        ui->networkSuccess->hide();
        ui->btn_reConnect->show();
    });
    connect(sock, &QSslSocket::connected, this, [this, sock]()
    {
        ui->networkFailure->hide();
        ui->networkSuccess->show();
        ui->lb_netStatus->setText("connect success");
        if (sock->isEncrypted())
        {
            qDebug() << "SSL加密已建立";
            qDebug() << "协议版本:" << sock->sessionProtocol();
            qDebug() << "加密算法:" << sock->sessionCipher().name();
        }
        else
        {
            qDebug() << "SSL握手未完成";
            if (sock->waitForEncrypted(5000))
            {
                qDebug() << "现在加密已完成";
                ui->networkSuccess->show();
            }
            else
            {
                qDebug() << "加密握手超时:" << sock->errorString();
                ui->networkFailure->show();
            }
        }
    });
    connect(sock, &QSslSocket::disconnected, this, [this]()
    {
        foreach(QWidget* widget, QApplication::topLevelWidgets())
        {
            if(widget != this)
            {
                widget->close();
            }
        }
        this->show();
        ui->networkFailure->show();
        ui->btn_reConnect->show();
        ui->lb_netStatus->setText("connection broken");
    });
    connect(sock, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &VioletClient::sslErrorHandle);
    connect(sock, &QSslSocket::readyRead, this, &VioletClient::read_cb);
}

void VioletClient::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this,
                                                               "quit",
                                                               "Are you sure to quit?",
                                                               QMessageBox::Cancel
                                                               | QMessageBox::Yes,
                                                               QMessageBox::Yes);

    if (resBtn != QMessageBox::Yes)
    {
        event->ignore();
    } else {
        if(sslSock)
        {
            disconnect(sslSock, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, nullptr);
            disconnect(sslSock, &QAbstractSocket::errorOccurred, this, nullptr);
            disconnect(sslSock, &QSslSocket::disconnected, this, nullptr);
            sslSock->disconnectFromHost();
            sslSock->deleteLater();
        }
        event->accept();
    }
}

void VioletClient::read_cb()
{
    qDebug()<< "into main read cb";
    SRHelper sr;
    auto packet = sr.recvMsg(sslSock, -1);
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
    connectNetwork(sslSock);
}

void VioletClient::on_btnWild_pressed() {
    VioletProtNeck neck = {};
    neck.unlogin = true;
    strcpy(neck.command, "nonreq");
    SRHelper sr;
    sr.sendMsg(sslSock, neck, "violet");
}


void VioletClient::on_btnLogin_pressed() {
    disconnect(sslSock, &QSslSocket::readyRead, this, &VioletClient::read_cb);
    loginCenter = new LoginCenter(sslSock);
    //登录失败或者没登录成功直接退出发送sclose，登录成功的话需要监听另一个sclose，怎么做呢，在另一边做吧，可是这里注定是要解绑的啊，如果登录成功了
    connect(loginCenter, &LoginCenter::sclose, this, [=](){this->show();connect(sslSock, &QSslSocket::readyRead, this, &VioletClient::read_cb);});
    //sloged代表登录成功，窗体已经删除，监听另一个信号
    connect(loginCenter, &LoginCenter::sloged, this, [=](UserMainWindow *umw){
        connect(umw, &UserMainWindow::sclose, this, [=](){this->show();connect(sslSock, &QSslSocket::readyRead, this, &VioletClient::read_cb);});
    });
    this->hide();
    loginCenter->show();
}
