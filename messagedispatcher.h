#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include <QObject>
#include <QHash>
#include <QQueue>
#include <QMutex>

struct JM
{
    QString uid;
    QString text;
};

class MessageDispatcher : public QObject
{
    Q_OBJECT
public:
    static MessageDispatcher* instance();
    // 注册/注销窗口
    void registerWindow(const QString &uid, QObject *receiver);
    void unregisterWindow(const QString &uid, QObject *receiver);
    // 消息分发接口
    void dispatchMessage(const QString &uid, const QString &uid2, const QString &text);
    void enqueueMessage(const QString &uid, const QString &text);
    void confirmAndDispatch();
    void closeAll();
    //下面两个方法没有实现，想法有些问题，应该用list保存已经打开的窗口，不在这里管理
    bool isWindowsRegistered(const QString &uid, QObject *receiver);
    QString getWidnows();
signals:
    // 实际分发信号（参数：接收者指针，用户id(/group), user，消息内容）
    void messageDispatched(QObject *receiver, const QString &uid, const QString &uid2, const QString &text);
private:
    explicit MessageDispatcher(QObject *parent = nullptr);
    QHash<QString, QObject*> m_receivers; // uid -> 接收窗口
    QQueue<JM> m_messageQueue;
    QMutex m_mutex;
};

#endif // MESSAGEDISPATCHER_H
