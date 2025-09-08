#ifndef COMMON_H
#define COMMON_H

#include <QMainWindow>
#include <QStandardPaths>
#include <QDir>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QJniEnvironment>
#include <QtCore/private/qandroidextras_p.h>
#endif

class Common
{
public:
    Common();
};

/**
 * @brief The ANDROIDPATH_TYPE enum
 * PRISTORAGE 内部存储空间，不需要申请权限，例如: /data/user/0/packagename/files...
 * EXTPRIVATE 外部私有空间，不需要申请权限（9以上），例如：/storage/emulater/0/Android/data/packagename/files...
 * EXTPUBLIC  外部公有空间，需要申请权限，例如：/storage/emulater/0/any...
 */
enum ANDROIDPATH_TYPE
{
    PRISTORAGE,
    EXTPRIVATE,
    EXTPUBLIC
};

inline bool checkAndroidPerssion()
{
#ifdef Q_OS_ANDROID
    QFuture<QtAndroidPrivate::PermissionResult> future = QtAndroidPrivate::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");
    QtAndroidPrivate::PermissionResult ret = future.result();
    if (ret == QtAndroidPrivate::PermissionResult::Denied)
    {
        QtAndroidPrivate::requestPermission("android.permission.READ_EXTERNAL_STORAGE");
        ret = QtAndroidPrivate::checkPermission("android.permission.READ_EXTERNAL_STORAGE").result();
        if(ret == QtAndroidPrivate::PermissionResult::Denied)
        {
            qWarning() << "Storage permission denied";
            return false;
        }
    }
    return true;
#else
    return false;
#endif
}

inline QString getAndroidPath(ANDROIDPATH_TYPE type=ANDROIDPATH_TYPE::EXTPRIVATE)
{
#ifdef Q_OS_ANDROID
    switch (type) {
    case ANDROIDPATH_TYPE::EXTPRIVATE:
    {
        QJniObject context = QtAndroidPrivate::context();
        if (!context.isValid())
        {
            qWarning() << "Failed to get Android context";
            return QString();
        }
        QJniObject fileObj = context.callObjectMethod("getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;", nullptr);
        if (!fileObj.isValid())
        {
            qWarning() << "Failed to call getExternalFilesDir";
            return QString();
        }
        QJniObject pathString = fileObj.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
        if (pathString.isValid())
        {
            QString absolutePath = pathString.toString();
            qDebug() << "Android External Files Dir: " << absolutePath;
            return absolutePath;
        }
        else
        {
            qWarning() << "Failed to get absolute path";
            return QString();
        }
        break;
    }
    case ANDROIDPATH_TYPE::EXTPUBLIC:
    {
        QString externalStoragePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        return externalStoragePath;
        break;
    }
    case ANDROIDPATH_TYPE::PRISTORAGE:
    {
        QString externalStoragePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        return externalStoragePath;
        break;
    }
    default:
    {
        return QString();
        break;
    }
    }
#else
    return QString();
#endif
}

#endif // COMMON_H
