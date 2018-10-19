#include <QCoreApplication>
#include "data.h"
#include "tcpnet.h"
#include "Packdef.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QSqlQuery       query;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("sweep");
    db.setUserName("root");
    db.setPassword("1234");
    PRINT("连接数据库...")
    bool ok = db.open();
    if(ok)  qDebug()<<"OK";
    else qDebug()<<"False";
    query = (QSqlQuery)db;
    Data data(query);

    return a.exec();
}
