#include "usermainwindow.h"
#include "ui_usermainwindow.h"
#include "messagedispatcher.h"

std::map<std::string, UserRecvBufferCache> UserMainWindow::userRecvBuffCache;

UserMainWindow::UserMainWindow(QSslSocket *socket, std::string uname, std::vector<std::string> iuserfriend, std::vector<std::string> iusergroup, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserMainWindow)
{
    setWindowIcon(QIcon(":/resources/violet.ico"));
    setAttribute(Qt::WA_Mapped);
    ui->setupUi(this);
    ui->tabWidget->setStyleSheet(
                "QTabBar::tab {"
                "    min-width: 0px;"      // 取消最小宽度限制
                "    width: 50%;"          // 两个 tab 各占 50%
                "    padding: 10px;"       // 内边距
                "    text-align: center;"  // 文字居中
                "}"
                );
    setWindowTitle(QString::fromStdString(uname));
    sock = socket;
    userfriend = iuserfriend;
    usergroup = iusergroup;
    m_username = uname;
    setWindowTitle(QString::fromStdString(m_username));
    for(const auto &f : userfriend)
    {
        //qDebug()<< "fri: " << f;
        ui->uFriendList->addItem(QString::fromStdString(f));
    }
    for(const auto &f : iusergroup)
    {
        ui->uGroupList->addItem(QString::fromStdString(f));
    }
    connect(sock, &QSslSocket::readyRead, this, &UserMainWindow::read_cb);
    path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    externalPrivatePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);    ///Android/data/<package_name>/
    externalPublicPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);     ///storage/emulated/0/
    if(!db.openDatabase(path+"/v"+uname.c_str()+".db"))
    {
        qDebug()<< "open db error";
    }
    {
        db.createTable("userinfo", "name TEXT NOT NULL, avat TEXT");
        QVariantMap me;
        me["name"] = uname.c_str();
        me["avat"] = "defalut right now";
        db.insertRecord("userinfo", me);
    }
    if(db.isOpen())
    {
        db.closeDatabase();
    }
}

UserMainWindow::~UserMainWindow()
{
    delete ui;
}

void UserMainWindow::read_cb()
{
    while(sock->bytesAvailable() >= 1)
    {
        std::optional<Msg> ret;
        if(userRecvBuffCache.size() >= 1)
        {
            qDebug()<< "cache size >= 1";
            auto ixt = userRecvBuffCache.begin();
            if(ixt != userRecvBuffCache.end())
            {
                auto &it = ixt->second;
                ret = sr.recvMsg(sock, it.expectLen-it.actuaLen);
                if(ret != std::nullopt)
                {
                    it.actuaLen += ret->header.checksum;
                    it.recvBuffer.insert(it.recvBuffer.end(), ret->content.begin(), ret->content.end());
                    qDebug()<< "actualen: " << it.actuaLen << " expectLen: " << it.expectLen << " recvBuffer len: " << it.recvBuffer.size();
                    if(it.actuaLen < it.expectLen)
                    {
                        continue;
                    }
                    else
                    {
                        ret = Msg::deserialize(it.recvBuffer.data(), it.recvBuffer.size());
                        userRecvBuffCache.erase(userRecvBuffCache.begin());
                        qDebug()<< "sizeof user recv buffer cache: " << userRecvBuffCache.size();
                    }
                }
            }
        }
        else
        {
            ret = sr.recvMsg(sock, -1);
            qDebug()<< "debug on recv:  " << ret->header.checksum << " , total: " << ntohl(ret->header.length);
        }
        if(ret->header.checksum < ntohl(ret->header.length))
        {
            UserRecvBufferCache urbf;
            urbf.uname = std::string(ret->neck.name);
            urbf.actuaLen = ret->header.checksum;
            urbf.expectLen = ntohl(ret->header.length);
            qDebug()<< "size of ret first recv: " << sizeof(ret);
            urbf.recvBuffer = ret.value().serialize();
            qDebug()<< "insert into user buffer cache recvbuffer size: " << urbf.recvBuffer.size();
            userRecvBuffCache[std::string(ret->neck.name)] = urbf;
            qDebug()<< "not recv all data, use user cache,recv:  " << ret->header.checksum << " , total: " << ntohl(ret->header.length);
            continue;
        }
        QString command(ret->neck.command);
        QString content(QString::fromStdString(std::string(ret->content.begin(), ret->content.end())));
        const QString name = QString(ret->neck.name);
        const QString pass = QString(ret->neck.pass);
        qDebug()<< "command on mainwidow: " << command << "--" << content;
        if(command == "vaddfsucc")
        {
            ui->uFriendList->addItem(content);
        }
        if(command == "vaddgsucc")
        {
            ui->uGroupList->addItem(content);
        }
        if(command == "vcrtgsucc")
        {
            ui->uGroupList->addItem(content);
        }
        if(command == "vpcerr")
        {
            qDebug()<<"vpcerr: " << content;
        }
        if(command == "vbul")
        {
            foreach (QListWidgetItem *item, ui->uFriendList->findItems(name, Qt::MatchExactly))
            {
                if (item->foreground().color() == Qt::green) {
                    qDebug() << "用户 " << name << " 已经登录，不要重复广播";
                }
                else
                {
                    qDebug() << "用户 " << name << " 登录，改变颜色，并回复服务器收到";
                    int rowid = ui->uFriendList->row(item);
                    if(rowid != 0)
                    {
                        item = ui->uFriendList->takeItem(rowid);
                        ui->uFriendList->insertItem(0, item);
                    }
                    item->setForeground(Qt::green);
                    VioletProtNeck neck = {};
                    strcpy(neck.command, "vbulre");
                    memcpy(neck.name, m_username.c_str(), sizeof(neck.name));
                    memcpy(neck.pass, name.toStdString().c_str(), sizeof(neck.pass));
                    std::string tmp("violet");
                    sr.sendMsg(sock, neck, tmp.c_str());
                }
            }
        }
        if(command == "vbol")
        {
            foreach (QListWidgetItem *item, ui->uFriendList->findItems(name, Qt::MatchExactly)) {
                int rowid = ui->uFriendList->row(item);
                if(rowid != 0)
                {
                    item = ui->uFriendList->takeItem(rowid);
                    ui->uFriendList->addItem(item);
                }
                item->setForeground(Qt::black);
            }
        }
        if(command == "vpcb")
        {
            auto it = std::find_if(pvpWindows.begin(), pvpWindows.end(), [name](const LoginUserWindows &lwu) { return lwu.uid == name; });
            if (it == pvpWindows.end())
            {
                QApplication::alert(this, 10000);
                openWindow(name, ChatType::LGP, true);
            }
            MessageDispatcher::instance()->dispatchMessage(QString(ret->neck.name), "", content);
        }
        if(command == "vpcache")
        {
            auto it = std::find_if(pvpWindows.begin(), pvpWindows.end(), [name](const LoginUserWindows &lwu) { return lwu.uid == name; });
            if (it == pvpWindows.end())
            {
                QApplication::alert(this, 10000);
                openWindow(name, ChatType::LGP, true);
            }
            MessageDispatcher::instance()->dispatchMessage(QString(ret->neck.name), pass, content);
        }
        if(command == "vgcb")
        {
            auto it = std::find_if(pvpWindows.begin(), pvpWindows.end(), [name](const LoginUserWindows &lwu) { return lwu.uid == name; });
            if (it == pvpWindows.end()) {
                QApplication::alert(this, 10000);
                openWindow(name, ChatType::LGG, true);
            }
            MessageDispatcher::instance()->dispatchMessage(QString(ret->neck.name), pass, content);
        }
        if(command == "vcrtgerr")
        {
            if(content == "exists")
            {
                QMessageBox::information(this, "错误", "创建群组失败，群组已经存在");
            }
            else
            {
                QMessageBox::information(this, "错误", "创建群组失败，未知错误，请重试");
            }
        }
        if(command == "vafed")
        {
            ui->uFriendList->addItem(name);
            foreach (QListWidgetItem *item, ui->uFriendList->findItems(name, Qt::MatchExactly))
            {
                int rowid = ui->uFriendList->row(item);
                if(rowid != 0)
                {
                    item = ui->uFriendList->takeItem(rowid);
                    ui->uFriendList->insertItem(0, item);
                }
                item->setForeground(Qt::green);
            }
            QApplication::alert(this, 10000);
        }
        if(command == "vaddgerr")
        {
            QMessageBox::information(this, "添加群组错误", content+":"+name);
        }
        if(command == "vaddferr")
        {
            QMessageBox::information(this, "添加好友错误", content);
        }
        if(command == "vtfspot")
        {
            QString sport = content;
            //启动另外一个线程，链接返回的端口，并发送文件
        }
    }
}

/**
 * @brief UserMainWindow::openWindow
 * @param name
 * @param isSrv 服务器发来的消息和自己点开消息的提示不一样，true为服务器发来的消息
 * @param type gchat, pchat, unloginchat, unloginPchat
 */
void UserMainWindow::openWindow(QString name, ChatType itype, bool isSrv)
{
    auto it = std::find_if(pvpWindows.begin(), pvpWindows.end(), [name](const LoginUserWindows &wu) { return wu.uid == name; });
    if (it != pvpWindows.end())
    {
        if(it->pvp->isMaximized() || it->pvp->isMaximized())
        {
            //it still not work, leave it alone, just, go away, nobody needs you
            it->pvp->showNormal();
            it->pvp->setWindowState(it->pvp->windowState() & ~Qt::WindowMinimized);
            it->pvp->setWindowState(Qt::WindowActive);
            it->pvp->activateWindow();
            it->pvp->show();
            QApplication::alert(it->pvp, 10000);
        }
        it->pvp->activateWindow();
        it->pvp->raise();
        return;
    }
    QMessageBox::StandardButton reply;
    if (isSrv)
    {
        reply = QMessageBox::question(this, "recv message from " + name, "Go to chat on " + name + " ?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
    } else
    {
        reply = QMessageBox::question(this, "chat on " + name, "Go to chat on " + name + " ?", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);
    }
    if (reply == QMessageBox::Yes)
    {

        priChat = new PvP(sock, name, QString::fromStdString(m_username), itype);
        LoginUserWindows lwu;
        lwu.pvp = priChat;
        lwu.uid = name;
        pvpWindows.append(lwu);
        connect(priChat, &PvP::m_close, [this, name](){ auto s = pvpWindows.removeIf([name](const LoginUserWindows uw) { return uw.uid == name; });});
        bool tmpbool = true;
        foreach (QListWidgetItem *item, ui->uRecentList->findItems(name, Qt::MatchExactly))
        {
            tmpbool = false;
        }
        if(tmpbool)
        {
            ui->uRecentList->addItem(name);
        }
        priChat->show();
    } else if (reply == QMessageBox::No)
    {
        // Discard changes
    }
    else
    {
        // Cancel action
    }
}

void UserMainWindow::closeEvent(QCloseEvent *ev)
{
    if (1) { // 自定义条件检查
        MessageDispatcher::instance()->closeAll();
        emit sclose();
        disconnect(sock, &QSslSocket::readyRead, this, &UserMainWindow::read_cb);
        this->deleteLater();
        ev->accept();
    } else {
        ev->ignore(); // 阻止关闭
    }
}


void UserMainWindow::on_ubAddFriend_pressed()
{
    bool ok;
    QString text = QInputDialog::getText(
                this,
                "添加好友",
                "请在下方输入好友名称或者好友ID",
                QLineEdit::Normal,
                "",
                &ok
                );
    if (ok && !text.trimmed().isEmpty()) {
        VioletProtNeck neck = {};
        strcpy(neck.command, "vaddf");
        memcpy(neck.name, m_username.c_str(), sizeof(neck.name));
        sr.sendMsg(sock, neck, text.toStdString());
    }
    else
    {
        QMessageBox::information(this, "用户名为空", "请检查后输入");
    }
}


void UserMainWindow::on_ubAddGroup_pressed()
{
    bool ok;
    QString text = QInputDialog::getText(
                this,
                "添加群组",
                "请在下方输入群组名称或者群组ID",
                QLineEdit::Normal,
                "",
                &ok
                );
    if (ok && !text.trimmed().isEmpty()) {
        VioletProtNeck neck = {};
        strcpy(neck.command, "vaddg");
        memcpy(neck.name, m_username.c_str(), sizeof(neck.name));
        sr.sendMsg(sock, neck, text.toStdString());
    }
    else
    {
        QMessageBox::information(this, "群组名为空", "请检查后输入");
    }
}


void UserMainWindow::on_ubCreateGroup_pressed()
{
    bool ok;
    QString text = QInputDialog::getText(
                this,
                "创建群组",
                "请在下方输入群组名称",
                QLineEdit::Normal,
                "",
                &ok
                );
    if (ok && !text.trimmed().isEmpty()) {
        VioletProtNeck neck = {};
        strcpy(neck.command, "vcrtg");
        memcpy(neck.name, m_username.c_str(), sizeof(neck.name));
        sr.sendMsg(sock, neck, text.toStdString());
    }
    else
    {
        QMessageBox::information(this, "群组名为空", "请检查后输入");
    }
}


void UserMainWindow::on_uFriendList_itemDoubleClicked(QListWidgetItem *item)
{
    QString tmp = item->text();
    openWindow(tmp, ChatType::LGP, false);
}

void UserMainWindow::on_uGroupList_itemDoubleClicked(QListWidgetItem *item)
{
    QString tmp = item->text();
    openWindow(tmp, ChatType::LGG, false);
}

void UserMainWindow::on_uRecentList_itemDoubleClicked(QListWidgetItem *item)
{
    QString name = item->text();
    auto it = ui->uGroupList->findItems(name, Qt::MatchExactly);
    if(it.isEmpty())
    {
        openWindow(name, ChatType::LGG, false);
    }
    else
    {
        openWindow(name, ChatType::LGP, false);
    }
}

