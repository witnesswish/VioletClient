#include "clickabletextbrowser.h"
#include <QTextCursor>
#include <QTextBlock>
#include <QDebug>

#include "clickabletextbrowser.h"

ClickableTextBrowser::ClickableTextBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    // 启用鼠标跟踪（可选）
    setMouseTracking(true);
}
void ClickableTextBrowser::mousePressEvent(QMouseEvent *e)
{
    // 首先检查是否点击了超链接
    if (anchorAt(e->pos()).isEmpty()) {
        QString text = getTextUnderCursor(e->pos());
        if (!text.isEmpty()) {
            emit textClicked(text, e->pos());
        }
    }

    // 保持原有功能（文本选择等）
    QTextBrowser::mousePressEvent(e);
}
void ClickableTextBrowser::mouseDoubleClickEvent(QMouseEvent *e)
{
    QString text = getTextUnderCursor(e->pos());
    if (!text.isEmpty()) {
        emit textDoubleClicked(text);
    }
    QTextBrowser::mouseDoubleClickEvent(e);
}

QString ClickableTextBrowser::getTextUnderCursor(const QPoint &pos) const
{
    QTextCursor cursor = cursorForPosition(pos);
    cursor.select(QTextCursor::WordUnderCursor);
    return cursor.selectedText();
}
