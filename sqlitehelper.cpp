#include "sqlitehelper.h"

sqliteHelper::sqliteHelper(QObject *parent) : QObject(parent) {}

sqliteHelper::~sqliteHelper() {
    closeDatabase();
}

bool sqliteHelper::openDatabase(const QString &dbName, const QString &connectionName)
{
    if (m_db.isOpen()) {
        return true;
    }
    if(connectionName.trimmed().isEmpty())
    {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
    }
    else
    {
        m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }
    m_db.setDatabaseName(dbName);
    m_dbName = dbName;

    if (!m_db.open()) {
        qDebug() << "Error: Failed to open database:" << m_db.lastError().text();
        return false;
    }
    return true;
}

void sqliteHelper::closeDatabase()
{
    if (m_db.isOpen()) {
        // QSqlQuery query(m_db);
        // query.finish(); // 结束未完成的查询
        m_db.close();
        m_db = QSqlDatabase();
        qDebug() << "close database 1 times";
    }
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        qDebug() << "close database 1 times";
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
}

bool sqliteHelper::isOpen() const
{
    return m_db.isOpen();
}

bool sqliteHelper::createTable(const QString &tableName, const QString &fields)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database is not open";
        return false;
    }

    QString queryStr = QString("CREATE TABLE IF NOT EXISTS %1 (%2)").arg(tableName, fields);
    QSqlQuery query(m_db);

    if (!query.exec(queryStr)) {
        qDebug() << "Create table error:" << query.lastError().text();
        query.finish();
        return false;
    }
    query.finish();
    return true;
}

bool sqliteHelper::insertRecord(const QString &tableName, const QVariantMap &data)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database is not open";
        return false;
    }

    if (data.isEmpty()) {
        qDebug() << "No data to insert";
        return false;
    }

    QString queryStr = buildInsertQuery(tableName, data);
    QSqlQuery query(m_db);

    query.prepare(queryStr);

    // 绑定值
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    if (!query.exec()) {
        qDebug() << "Insert error:" << query.lastError().text();
        query.finish();
        return false;
    }
    query.finish();
    return true;
}

bool sqliteHelper::updateRecord(const QString &tableName, const QVariantMap &data, const QString &where)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database is not open";
        return false;
    }

    if (data.isEmpty()) {
        qDebug() << "No data to update";
        return false;
    }

    QString queryStr = buildUpdateQuery(tableName, data, where);
    QSqlQuery query(m_db);

    query.prepare(queryStr);

    // 绑定值
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    if (!query.exec()) {
        qDebug() << "Update error:" << query.lastError().text();
        query.finish();
        return false;
    }
    query.finish();
    return query.numRowsAffected() > 0;
}

bool sqliteHelper::deleteRecord(const QString &tableName, const QString &where)
{
    if (!m_db.isOpen()) {
        qDebug() << "Database is not open";
        return false;
    }

    QString queryStr = QString("DELETE FROM %1").arg(tableName);
    if (!where.isEmpty()) {
        queryStr += " WHERE " + where;
    }

    QSqlQuery query(m_db);
    if (!query.exec(queryStr)) {
        qDebug() << "Delete error:" << query.lastError().text();
        query.finish();
        return false;
    }
    query.finish();
    return query.numRowsAffected() > 0;
}

QList<QVariantMap> sqliteHelper::selectRecords(const QString &tableName, const QString &where, const QString &orderBy)
{
    QList<QVariantMap> result;

    if (!m_db.isOpen()) {
        qDebug() << "Database is not open";
        return result;
    }

    QString queryStr = QString("SELECT * FROM %1").arg(tableName);
    if (!where.isEmpty()) {
        queryStr += " WHERE " + where;
    }
    if (!orderBy.isEmpty()) {
        queryStr += " ORDER BY " + orderBy;
    }

    QSqlQuery query(queryStr, m_db);

    while (query.next()) {
        QVariantMap record;
        QSqlRecord sqlRecord = query.record();

        for (int i = 0; i < sqlRecord.count(); ++i) {
            record[sqlRecord.fieldName(i)] = sqlRecord.value(i);
        }

        result.append(record);
    }
    query.finish();
    return result;
}

bool sqliteHelper::beginTransaction()
{
    if (!m_db.isOpen()) {
        qDebug() << "Database is not open";
        return false;
    }
    return m_db.transaction();
}

bool sqliteHelper::commitTransaction()
{
    if (!m_db.isOpen()) {
        qDebug() << "Database is not open";
        return false;
    }
    return m_db.commit();
}

bool sqliteHelper::rollbackTransaction()
{
    if (!m_db.isOpen()) {
        qDebug() << "Database is not open";
        return false;
    }
    return m_db.rollback();
}

QString sqliteHelper::buildInsertQuery(const QString &tableName, const QVariantMap &data)
{
    QString columns;
    QString values;

    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        if (!columns.isEmpty()) {
            columns += ", ";
            values += ", ";
        }
        columns += it.key();
        values += ":" + it.key();
    }

    return QString("INSERT INTO %1 (%2) VALUES (%3)").arg(tableName, columns, values);
}

QString sqliteHelper::buildUpdateQuery(const QString &tableName, const QVariantMap &data, const QString &where)
{
    QString setClause;

    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        if (!setClause.isEmpty()) {
            setClause += ", ";
        }
        setClause += it.key() + " = :" + it.key();
    }

    QString queryStr = QString("UPDATE %1 SET %2").arg(tableName, setClause);
    if (!where.isEmpty()) {
        queryStr += " WHERE " + where;
    }

    return queryStr;
}
