#ifndef SQLITEHELPER_H
#define SQLITEHELPER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QList>

class sqliteHelper : public QObject
{
public:
    explicit sqliteHelper(QObject *parent = nullptr);
    ~sqliteHelper();
    bool openDatabase(const QString &dbName, const QString &connectionName="qt_sql_default_connection");
    void closeDatabase();
    bool isOpen() const;
    bool createTable(const QString &tableName, const QString &fields);
    bool insertRecord(const QString &tableName, const QVariantMap &data);
    bool updateRecord(const QString &tableName, const QVariantMap &data, const QString &where);
    bool deleteRecord(const QString &tableName, const QString &where);
    QList<QVariantMap> selectRecords(const QString &tableName, const QString &where = QString(), const QString &orderBy = QString());
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
private:
    QSqlDatabase m_db;
    QString m_dbName;
    QString buildInsertQuery(const QString &tableName, const QVariantMap &data);
    QString buildUpdateQuery(const QString &tableName, const QVariantMap &data, const QString &where);
};

#endif // SQLITEHELPER_H
