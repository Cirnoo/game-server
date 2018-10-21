#include "mysql.h"
#include "Packdef.h"
#include <QString>
MySQL::MySQL()
{
    PRINT("连接数据库...")
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("sweep");
    db.setUserName("root");
    db.setPassword("1234");
    query = (QSqlQuery)db;
    bool ok = db.open();
    if(ok)  qDebug()<<"OK";
    else qDebug()<<"False";

}

bool MySQL::Login(QString name, QString password)
{
    query.prepare("select * from users where (id=:Name and pw=:PW);");
    query.bindValue(":Name",name);
    query.bindValue(":PW",password);
    query.exec();
    bool flag=query.next();
    if(flag)
    {
        qDebug()<<"Login successful.";
    }
    else
    {
        qDebug()<<"User name or password error.";
    }
//    DATA_PACKAGE pack;
//    pack.ms_type=flag?MS_TYPE::LOGIN_RE_T:MS_TYPE::LOGIN_RE_F;
//    tcp.SendData(m_socket,pack);
    return flag;
}

bool MySQL::ChangePassword(QString name, QString password)
{
    query.prepare("update users set pw=:PW where id=:Name;");
    query.bindValue(":Name",name);
    query.bindValue(":PW",password);
    bool flag=query.next();
    if(flag)
    {
        qDebug()<<"Change password success.";
    }
    else
    {
        qDebug()<<"Change password failure.";
    }
    return flag;
}

bool MySQL::Register(QString name, QString password)
{
    query.prepare("insert into users(id,pw)values(:Name,:PW)");
    query.bindValue(":Name",name);
    query.bindValue(":PW",password);
    bool flag=query.exec();
    qDebug()<<"add user"<<(flag?"success":"fail");
//    DATA_PACKAGE pack;
//    pack.ms_type=flag?MS_TYPE::REGISTER_RE_T:MS_TYPE::REGISTER_RE_F;
//    tcp.SendData(m_socket,pack);
    return flag;
}
