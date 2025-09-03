#include "registerviolet.h"
#include "ui_registerviolet.h"

RegisterViolet::RegisterViolet(QSslSocket *socket, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterViolet)
{
    setWindowIcon(QIcon(":/resources/violet.ico"));
    setAttribute(Qt::WA_Mapped);
    ui->setupUi(this);
    sock = socket;
    connect(sock, &QSslSocket::readyRead, this, &RegisterViolet::read_cb);
    setAttribute(Qt::WA_DeleteOnClose);
}

RegisterViolet::~RegisterViolet()
{
    delete ui;
}
void RegisterViolet::on_vreg_pressed()
{
    QString username = ui->username->text().trimmed();
    QString password1 = ui->password1->text().trimmed();
    QString password2 = ui->password2->text().trimmed();
    QString nickname = ui->nickname->text().trimmed();
    QString email = ui->email->text().trimmed();
    if(username.isEmpty() || password1.isEmpty() || password2.isEmpty())
    {
        QMessageBox::warning(this, "error", "用户名和密码需要填写");
        return;
    }
    if( username.length() > 15 || password1.length() > 60 || password2.length() > 60)
    {
        QMessageBox::warning(this, "error", "账号/密码太长了");
        return;
    }
    if(password1 != password2)
    {
        QMessageBox::warning(this, "error", "两次输入的密码不一样");
        return;
    }
    VioletProtNeck neck = {};
    strcpy(neck.command, "vreg");
    memcpy(neck.name, username.toStdString().c_str(), sizeof(neck.name));
    memcpy(neck.pass, password1.toStdString().c_str(), sizeof(neck.pass));
    memcpy(neck.email, email.toStdString().c_str(), sizeof(neck.email));
    QString tmp("violet");
    sr.sendMsg(sock, neck, tmp.toStdString());
}

void RegisterViolet::read_cb() {
    auto ret = sr.recvMsg(sock, -1);
    QString command(ret->neck.command);
    QString content = QString::fromStdString(std::string(ret->content.begin(), ret->content.end()));
    if(command == "vregsucc")
    {
        ui->result->setAlignment(Qt::AlignCenter);
        ui->result->setStyleSheet("QLabel { color: green; }");
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [=](){
            remainingSeconds--;
            ui->result->setText(QString("注册成功，请关闭这个页面，进行登录,窗口将在 %1 秒后自动关闭").arg(remainingSeconds));
            ui->vreg->hide();
            if (remainingSeconds <= 0) {
                timer->stop();      // 停止计时器
                this->close();       // 关闭窗口（等效点击X按钮）
            }
        });
        remainingSeconds = 180;  // 初始倒计时10秒
        timer->start(1000);     // 启动计时器
        return;
    }
    ui->result->setAlignment(Qt::AlignCenter);
    ui->result->setStyleSheet("QLabel { color: red; }");
    ui->result->setText(QString("注册失败 ") + content);
}

void RegisterViolet::closeEvent(QCloseEvent *ev)
{
    if(timer)
    {
        timer->stop();
    }
    disconnect(sock, &QSslSocket::readyRead, this, &RegisterViolet::read_cb);
    emit sclose();
    ev->accept();
}
