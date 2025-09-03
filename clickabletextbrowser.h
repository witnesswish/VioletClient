#ifndef CLICKABLETEXTBROWSER_H
#define CLICKABLETEXTBROWSER_H

#include <QTextBrowser>
#include <QMouseEvent>

class ClickableTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit ClickableTextBrowser(QWidget *parent = nullptr);

signals:
    // 普通文本点击信号
    void textClicked(const QString &text, const QPoint &pos);
    // 双击信号
    void textDoubleClicked(const QString &text);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

private:
    QString getTextUnderCursor(const QPoint &pos) const;
};

#endif // CLICKABLETEXTBROWSER_H
