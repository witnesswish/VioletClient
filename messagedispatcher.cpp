#include "messagedispatcher.h"

MessageDispatcher* MessageDispatcher::instance()
{
    static MessageDispatcher inst;
    return &inst;
}

MessageDispatcher::MessageDispatcher(QObject *parent) : QObject(parent) {}

void MessageDispatcher::registerWindow(const QString &uid, QObject *receiver)
{
    if (!uid.isEmpty() && receiver) {
        m_receivers.insert(uid, receiver);
    }
}

void MessageDispatcher::unregisterWindow(const QString &uid, QObject *receiver)
{
    if (m_receivers.value(uid) == receiver) {
        m_receivers.remove(uid);
    }
}

void MessageDispatcher::dispatchMessage(const QString &uid, const QString &uid2, const QString &text)
{
    if (m_receivers.contains(uid)) {
        emit messageDispatched(m_receivers.value(uid), uid, uid2, text);
    }
}

void MessageDispatcher::enqueueMessage(const QString &uid, const QString &text) {}
void MessageDispatcher::closeAll() {
    QHashIterator<QString, QObject*> it(m_receivers);
    while (it.hasNext()) {
        it.next();
        QObject* obj = it.value();
        if (obj) {
            // 尝试调用close()方法（如果是QWidget派生类）
            QMetaObject::invokeMethod(obj, "close", Qt::DirectConnection);
            // 或者直接删除
            // obj->deleteLater();
        }
    }
    m_receivers.clear();
}
