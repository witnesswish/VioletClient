#ifndef REGISTERVIOLET_H
#define REGISTERVIOLET_H

#include <QDialog>
#include <QMessageBox>
#include <QSslSocket>
#include <QTimer>
#include <QCloseEvent>

#include "protocol.h"

namespace Ui {
class RegisterViolet;
}

class RegisterViolet : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterViolet(QSslSocket *socket, QWidget *parent = nullptr);
    ~RegisterViolet();
signals:
    void sclose();
private slots:
    void on_vreg_pressed();

private:
    Ui::RegisterViolet *ui;
    QSslSocket *sock;
    SRHelper sr;
     QTimer *timer;
     int remainingSeconds;

private:
    void read_cb();
protected:
    void closeEvent(QCloseEvent *ev) override;
};

#endif // REGISTERVIOLET_H
