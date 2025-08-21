#include "pvp.h"
#include "messagedispatcher.h"
#include "ui_pvp.h"

PvP::PvP(QTcpSocket *socket, QString &uid, const QString loginUsername, ChatType itype, QWidget *parent)
    : QWidget(parent), m_uid(uid), type(itype), m_loginUsername(loginUsername), ui(new Ui::PvP)
{
    setWindowIcon(QIcon(":/resources/violet.ico"));
    setAttribute(Qt::WA_Mapped);
    ui->setupUi(this);
    setWindowTitle(uid);
    sock = socket;
    MessageDispatcher::instance()->registerWindow(m_uid, this);
    connect(MessageDispatcher::instance(), &MessageDispatcher::messageDispatched, this, &PvP::onMessageDispatched, Qt::QueuedConnection);
}

PvP::~PvP()
{
    MessageDispatcher::instance()->unregisterWindow(m_uid, this);
    delete ui;
}

void PvP::closeEvent(QCloseEvent *event)
{
    emit m_close();
    event->accept();
}

void PvP::displayAppend(QTextBrowser *textBrowser, const QString &username, const QString &avatarPath, const QString &text, Qt::Alignment align)
{
    // 保存当前文本光标位置
    QTextCursor cursor = textBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);

    QString tmp = text;
    if(text.length() < 37)
    {
        int n = 39 - text.length();
        tmp = text + QString(n, '_');
    }

    QTextTableFormat tableFormat;
    tableFormat.setBorder(0);
    tableFormat.setCellPadding(4);
    tableFormat.setAlignment(align);

    if (align & Qt::AlignRight) {
        tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 99));
        tableFormat.setPosition(QTextFrameFormat::FloatRight);
    } else {
        tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 99));
    }

    QTextTable *table = cursor.insertTable(1, 2, tableFormat);

    QTextCharFormat charFormat;
    charFormat.setVerticalAlignment(QTextCharFormat::AlignMiddle);

    if (align & Qt::AlignRight) {
        // msg from local
        QTextTableCell cell = table->cellAt(0, 1);
        QTextCursor cellCursor = cell.firstCursorPosition();
        if (avatarPath != nullptr) {
            cellCursor.insertHtml(
                        QString("<a href=user:/%2><img src='%1' width='40' height='40'/></a>")
                        .arg(avatarPath, username));
        }
        cell = table->cellAt(0, 0);
        cellCursor = cell.firstCursorPosition();
        cellCursor.insertHtml(QString("%2 %3<div style='background-color: #CCFFCC; border-radius: 5px; "
                                      "padding: 8px;'>%1</div>")
                              .arg(tmp, username, QTime::currentTime().toString()));
    } else {
        // msg from server
        QTextTableCell cell = table->cellAt(0, 0);
        QTextCursor cellCursor = cell.firstCursorPosition();
        if (avatarPath != nullptr) {
            cellCursor.insertHtml(
                        QString("<a href=user:/%2><img src='%1' width='40' height='40'/></a>")
                        .arg(avatarPath, username));
        }
        cell = table->cellAt(0, 1);
        cellCursor = cell.firstCursorPosition();
        cellCursor.insertHtml(
                    QString("%2 %3<div style='background-color: #CCFFFF; border-radius: 5px; "
                            "padding: 3px;'>%1</div>")
                    .arg(tmp, username, QTime::currentTime().toString()));
    }
    textBrowser->moveCursor(QTextCursor::End);
    textBrowser->ensureCursorVisible();
}

void PvP::onMessageDispatched(QObject *receiver, const QString &uid, const QString &uid2, const QString &text)
{
    qDebug() << "messagee recv on dispatch： " << this->windowTitle();
    if (receiver == this) {
        QApplication::alert(this, 10000);
        if(text == QString("user not online"))
        {
            displayAppend(ui->display, "系统提示", "", "当前用户不在线，如果是匿名模式，系统不会为您保留任何记录，如果是登录模式，那么系统会在用户上线之后进行消息推送");
            return;
        }
        if(!uid2.trimmed().isEmpty())
        {
            displayAppend(ui->display, uid2, ":/avtars/resources/user1.png", text);
            return;
        }
        displayAppend(ui->display, this->windowTitle(), ":/avtars/resources/user1.png", text);
    }
}

void PvP::on_send_pressed()
{
    //本来考虑写一个判断，是不是自己和自己聊天，
    //但是我仔细想了想，就这样吧，自己和自己聊天也不是什么稀奇的事情
    //add to local file or database later, mabye
    qDebug()<<sock->bytesToWrite();
    QString text = ui->text->text();
    if (text.isEmpty() || text.isNull()) {
        ui->text->setPlaceholderText("you need to type some word");
        return;
    }
    VioletProtNeck neck = {};
    //如果登录了，m_islogin为真，走下面代码块，直接返回
    // 不改代码，直接复用
    if(type == LGP)
    {
        strcpy(neck.command, "vpc");
        memcpy(neck.name, m_loginUsername.toStdString().c_str(), sizeof(neck.name));
        memcpy(neck.pass, m_uid.toStdString().c_str(), sizeof(neck.pass));
        sr.sendMsg(sock, neck, text.toStdString());
        displayAppend(ui->display, m_loginUsername, ":/avtars/resources/user5.png", text, Qt::AlignRight);
        ui->text->clear();
        return;
    }
    if(type == LGG)
    {
        qDebug() << "type is LGG";
        strcpy(neck.command, "vgc");
        memcpy(neck.name, m_loginUsername.toStdString().c_str(), sizeof(neck.name));
        memcpy(neck.pass, m_uid.toStdString().c_str(), sizeof(neck.pass));
        sr.sendMsg(sock, neck, text.toStdString());
        displayAppend(ui->display, m_loginUsername, ":/avtars/resources/user5.png", text, Qt::AlignRight);
        ui->text->clear();
        return;
    }
    if(type == ULP)
    {
        strcpy(neck.command, "nonp");
        neck.mto = m_uid.toInt();
        if(neck.mto == 0)
        {
            displayAppend(ui->display, "系统提示", "", QString("这是一个错误，如果不是系统出问题，你可能点进了开发测试的点，这个会在后续版本改进"), Qt::AlignCenter);
            return;
        }
        neck.unlogin = true;
        sr.sendMsg(sock, neck, text.toStdString());
        qDebug()<<sock->bytesToWrite();
        displayAppend(ui->display, "you", ":/avtars/resources/user3.png", text, Qt::AlignRight);
    }
    ui->text->clear();
}

void PvP::on_text_returnPressed()
{
    on_send_pressed();
}

void PvP::on_sned_file_pressed()
{
    QString filePath = QFileDialog::getOpenFileName(
                nullptr,
                QObject::tr("请选择一个文件"), // 对话框标题
                QDir::homePath(),              // 默认从用户home目录开始
                QObject::tr("所有文件 (*.*);;文本文件 (*.txt);;图像文件 (*.png *.jpg *.bmp)") // 过滤器
                );
    if (filePath.isEmpty())
    {
        qDebug() << "用户取消了选择。";
        return;
    }
    QFileInfo fileInfo(filePath);

    qDebug() << "文件路径:" << fileInfo.filePath();
    qDebug() << "绝对路径:" << fileInfo.absoluteFilePath();
    qDebug() << "文件名（带后缀）:" << fileInfo.fileName();
    qDebug() << "基础文件名（不带后缀）:" << fileInfo.baseName();
    qDebug() << "后缀名:" << fileInfo.suffix();
    qDebug() << "文件大小（字节）:" << fileInfo.size();
    qDebug() << "创建时间:" << fileInfo.birthTime().toString(); // Qt 5.10+
    qDebug() << "最后修改时间:" << fileInfo.lastModified().toString();
    qDebug() << "最后访问时间:" << fileInfo.lastRead().toString();
    qDebug() << "是否是目录?" << fileInfo.isDir();
    qDebug() << "是否是文件?" << fileInfo.isFile();
    qDebug() << "是否存在?" << fileInfo.exists();
    qDebug() << "权限（可读）?" << fileInfo.isReadable();
    qDebug() << "权限（可写）?" << fileInfo.isWritable();
    qDebug() << "权限（可执行）?" << fileInfo.isExecutable();

    // 5. (可选) 使用 QMessageBox 弹窗显示信息
    QString infoMessage = QString("文件信息:\n"
                                  "名称: %1\n"
                                  "大小: %2 字节\n"
                                  "修改时间: %3")
            .arg(fileInfo.fileName())
            .arg(fileInfo.size())
            .arg(fileInfo.lastModified().toString());

    QMessageBox::information(nullptr, "文件详情", infoMessage);
}


void Worker::doWork(const QString parameter) {
    qDebug() << "Worker thread id:" << QThread::currentThreadId();
    for (int i = 0; i < 5; ++i) {
        QThread::sleep(1);
        QString result = parameter + QString::number(i);
        emit resultReady(result);
    }
    emit finished();
}
