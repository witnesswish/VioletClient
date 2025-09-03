#include "unlogincenter.h"
#include "clickabletextbrowser.h"
#include "messagedispatcher.h"
#include "ui_unlogincenter.h"
#include "protocol.h"

UnloginCenter::UnloginCenter(QSslSocket *socket, int uid, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UnloginCenter)
{
    setWindowIcon(QIcon(":/resources/violet.ico"));
    setAttribute(Qt::WA_Mapped);
    ui->setupUi(this);
    setWindowTitle("匿名模式,你的临时ID: " + QString::number(uid));
    sock = socket;
    ui->send->hide();
    ui->text->hide();
    ui->lwHome->addItem(QString::number(uid));
    displayAppend(
        ui->display,
        "系统提示",
        "",
        "匿名模式的群组聊天默认关闭，点击下方按钮开启 | 给<a href='user:/admin'>自己</a>发送的消息",
        Qt::AlignCenter);
    connect(sock, &QSslSocket::readyRead, this, &UnloginCenter::read_cb);
    connect(ui->display, &ClickableTextBrowser::textDoubleClicked, this, [](const QString &text) {
        qDebug() << "双击了文本:" << text;
    });
    ui->display->setOpenLinks(false); // 禁用自动打开链接
    ui->display->setOpenExternalLinks(false);
}

UnloginCenter::~UnloginCenter()
{
    delete ui;
}

void UnloginCenter::closeEvent(QCloseEvent *event)
{
    SRHelper sr;
    QString text = ui->text->text();
    VioletProtNeck neck = {};
    neck.unlogin = true;
    strcpy(neck.command, "nonqg");
    neck.unlogin = true;
    sr.sendMsg(sock, neck, text.toStdString());
    if (1) { // 自定义条件检查
        disconnect(sock, &QSslSocket::readyRead, this, &UnloginCenter::read_cb);
        MessageDispatcher::instance()->closeAll();
        event->accept(); // 允许关闭
        emit sclose();
    } else {
        event->ignore(); // 阻止关闭
    }
}

void UnloginCenter::on_send_pressed()
{
    qDebug()<<sock->bytesToWrite();
    SRHelper sr;
    QString text = ui->text->text();
    if (text.isEmpty() || text.isNull()) {
        ui->text->setPlaceholderText("you need to type some word");
        return;
    }
    VioletProtNeck neck = {};
    neck.unlogin = true;
    strcpy(neck.command, "nong");
    sr.sendMsg(sock, neck, text.toStdString());
    qDebug()<<sock->bytesToWrite();
    displayAppend(ui->display, "me", ":/avtars/resources/user1.png", text, Qt::AlignRight);
    ui->text->clear();
}
void UnloginCenter::displayAppend(QTextBrowser *textBrowser,
                                  const QString &username,
                                  const QString &avatarPath,
                                  const QString &otext,
                                  Qt::Alignment align)
{
    QString text;
    qDebug()<< "u: " << otext.length() << "-" << sizeof(otext);
    if(otext.length() < 30)
    {
        int n = 39 - otext.length();
        text =  otext + QString(n, '_');
    }
    else
    {
        text = otext;
    }

    // 保存当前文本光标位置
    QTextCursor cursor = textBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);

    // 创建表格来布局头像、用户名和消息
    QTextTableFormat tableFormat;
    tableFormat.setBorder(0);
    //tableFormat.setCellPadding(4);
    tableFormat.setAlignment(align);
    tableFormat.setLeftMargin(0);

    // 根据对齐方式设置表格宽度和浮动属性
    if (align & Qt::AlignRight) {
        tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
        //tableFormat.setColumnWidthConstraints({QTextLength(QTextLength::PercentageLength, 100),QTextLength(QTextLength::FixedLength, 32)});
    } else {
        tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    }

    // 创建2行2列的表格
    QTextTable *table = cursor.insertTable(1, 2, tableFormat);

    // 设置文字对齐
    QTextCharFormat charFormat;
    charFormat.setVerticalAlignment(QTextCharFormat::AlignMiddle);
    if (align & Qt::AlignRight) {
        // 自己发的消息头像在右侧
        QTextTableCell cell = table->cellAt(0, 1);
        QTextCursor cellCursor = cell.firstCursorPosition();
        QTextBlockFormat rightBlockFormat;
        rightBlockFormat.setAlignment(Qt::AlignRight); // 头像右对齐
        cellCursor.setBlockFormat(rightBlockFormat);
        // 插入头像
        if (avatarPath != nullptr) {
            cellCursor.insertHtml(
                QString("<a href=user:/%2 style='text-align: center;'><img src='%1' width='32' height='32' style='display: inline-block;' /></a>")
                    .arg(avatarPath, username));
        }
        // 消息内容在左侧
        QTextTableCell tcell = table->cellAt(0, 0);
        QTextCursor tcellCursor = tcell.firstCursorPosition();
        QTextBlockFormat leftBlockFormat;
        leftBlockFormat.setAlignment(Qt::AlignRight);  // 内容右对齐
        leftBlockFormat.setLeftMargin(0);              // 清除单元格左侧边距
        tcellCursor.setBlockFormat(leftBlockFormat);
        tcellCursor.insertHtml(QString("<div>%2</div><div "
                                      "style='background-color: #CCFFCC; font-size: 15px;"
                                      "'>%1</div>")
                                  .arg(text, QTime::currentTime().toString()));
    } else {
        // 头像在左侧
        QTextTableCell cell = table->cellAt(0, 0);
        QTextCursor cellCursor = cell.firstCursorPosition();
        // 插入头像
        if (avatarPath != nullptr) {
            cellCursor.insertHtml(
                QString("<a href=user:/%2><img src='%1' width='40' height='40'/><div>%2</div></a>")
                    .arg(avatarPath, username));
        }
        // 消息内容在右侧
        cell = table->cellAt(0, 1);
        cellCursor = cell.firstCursorPosition();
        cellCursor.insertHtml(
            QString("%2<div style='background-color:#CCFFFF; border-radius: 5px; "
                    "padding: 3px;'>%1</div>")
                .arg(text, QTime::currentTime().toString()));
    }
    textBrowser->moveCursor(QTextCursor::End);
    textBrowser->ensureCursorVisible();
}

void UnloginCenter::read_cb()
{
    qDebug() << "unlogin read from seerver";
    while (sock->bytesAvailable() >= 41) {
        SRHelper sr;
        auto packet = sr.recvMsg(sock, -1);
        if (packet == std::nullopt) {
            qDebug() << "error read on unlogin";
            return;
        }
        VioletProtNeck neck = packet->neck;
        const std::string command(neck.command);
        if (command == std::string("nongb")) {
            displayAppend(ui->display,
                          QString(neck.name),
                          ":/avtars/resources/user1.png",
                          QString::fromStdString(
                              std::string(packet->content.begin(), packet->content.end())),
                          Qt::AlignLeft);
        }
        //private chat
        if (command == std::string("nonpb") || command == std::string("nonperr")) {
            const QString username = QString(neck.name);
            auto it = std::find_if(pvpWindows.begin(),
                                   pvpWindows.end(),
                                   [username](const UserWindows &wu) { return wu.uid == username; });
            if (it == pvpWindows.end()) {
                //如果迭代器到了末尾，就是没有找到已打开的这个窗口，那么相当于手动点击这个用户，然后分发消息
                clickedUser(username, true);
            }
            qDebug() << "x: " << x++;
            MessageDispatcher::instance()->dispatchMessage(QString(packet->neck.name), "", QString::fromStdString(std::string(packet->content.begin(), packet->content.end())));
        }
        if (command == std::string("nonigsucc")) {
            ui->send->show();
            ui->text->show();
            ui->enter->hide();
        }
    }
}

void UnloginCenter::clickedUser(QString uid, bool isfromSrv)
{
    QMessageBox::StandardButton reply;
    if (isfromSrv) {
        reply = QMessageBox::question(this,
                                      "recv message from " + uid,
                                      "Go to chat with " + uid + " ?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                      QMessageBox::No);
    } else {
        reply = QMessageBox::question(this,
                                      "chat with " + uid,
                                      "Go to chat with " + uid + " ?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                      QMessageBox::No);
    }

    if (reply == QMessageBox::Yes) {
        priChat = new PvP(sock, uid, "unlogin", ChatType::ULP);
        UserWindows wu;
        wu.pvp = priChat;
        wu.uid = uid;
        pvpWindows.append(wu);
        connect(priChat, &PvP::m_close, [this, uid]() {
            auto s = pvpWindows.removeIf([uid](const UserWindows uw) { return uw.uid == uid; });
            qDebug() << "triggered remove pvpchat: " << pvpWindows.size() << s;
        });
        bool tmpbool = true;
        foreach (QListWidgetItem *item, ui->lwHome->findItems(uid, Qt::MatchExactly)) {
            //qDebug() << "Found:" << item->text();
            tmpbool = false;
        }
        if(tmpbool)
        {
            ui->lwHome->addItem(uid);
        }
        priChat->show();
    } else if (reply == QMessageBox::No) {
        // Discard changes
    } else {
        // Cancel action
    }
}

void UnloginCenter::on_text_returnPressed()
{
    on_send_pressed();
}

void UnloginCenter::on_enter_pressed()
{
    SRHelper sr;
    VioletProtNeck neck = {};
    strcpy(neck.command, "nonig");
    neck.unlogin = true;
    std::string tmp("violet");
    sr.sendMsg(sock, neck, tmp);
}

void UnloginCenter::on_lwHome_itemDoubleClicked(QListWidgetItem *item)
{
    QString username = item->text();
    auto it = std::find_if(pvpWindows.begin(),
                           pvpWindows.end(),
                           [username](const UserWindows &wu) { return wu.uid == username; });
    if (it == pvpWindows.end()) {
        //如果迭代器到了末尾，就是没有找到已打开的这个窗口，那么相当于手动点击这个用户，然后分发消息
        clickedUser(username, true);
    }
    else
    {
        if(it->pvp->isMaximized() || it->pvp->isMaximized())
        {
            it->pvp->showNormal();
        }
        it->pvp->activateWindow();
        it->pvp->raise();
    }
}

void UnloginCenter::on_display_anchorClicked(const QUrl &url)
{
    if (url.scheme() == "https" || url.scheme() == "http") {
        // 外部链接：请求用户确认后打开浏览器
        QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                  "打开外部链接",
                                                                  "即将打开外部链接:\n"
                                                                      + url.toString()
                                                                      + "\n是否继续？",
                                                                  QMessageBox::Yes
                                                                      | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QDesktopServices::openUrl(url); // 使用系统默认浏览器打开
        }
    }
    if (url.scheme() == "app") {
        // 内部动作（如跳转页面、触发功能）
        qDebug() << "执行内部动作:" << url.path();
        //handleInternalAction(url.path());  // 自定义处理函数
    }
    if (url.scheme() == "user") {
        // 内部动作（如跳转页面、触发功能）
        QString username = url.path().mid(1); // 去掉开头的"/"
        qDebug() << "点击了用户:" << username;
        clickedUser(username, false);
        //handleInternalAction(url.path());  // 自定义处理函数
    }
}
