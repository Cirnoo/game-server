#ifndef MYSQL_H
#define MYSQL_H

#include <QtSql>

class MySQL
{
public:
    MySQL();
    bool Login(QString name, QString password);
    bool ChangePassword(QString name, QString password);
    bool Register(QString name, QString password);
private:
    QSqlQuery  query;

};

#endif // MYSQL_H
