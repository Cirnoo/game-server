#include "data.h"
#include <QString>
#define MSMAP(x,y) AddMS(MS_TYPE::x,[this,x](USER_INFO)->bool{ this->y(x)});
Data::Data(QSqlQuery _query):tcp(this)
{
    query=_query;


}



bool Data::DealMS(QTcpSocket * _socket,DATA_PACKAGE pack)
{
    auto name=QString::fromStdWString(pack.user.name.GetStr());
    auto password=QString::fromStdWString(pack.user.password.GetStr());
    socket=_socket;
    switch (pack.ms_type)
    {
    case MS_TYPE::REGISTER_RQ:
    {
        return Register(name,password);
    }
    case MS_TYPE::REGISTER_RE_T:
    {
        return true;
    }
    case MS_TYPE::LOGIN_RQ:
    {
        return Login(name,password);
    }
    case MS_TYPE::LOGIN_RE_F:
    {
        return true;
    }
    default:
    {
        DATA_PACKAGE pack;
        tcp.SendMessage(socket,pack);
        return true;
    }
    }
    return true;
}

void Data::SendMS(QTcpSocket *socket, DATA_PACKAGE pack)
{
    socket->write((char *)&pack,sizeof (pack));
}




bool Data::Register(QString name, QString password)
{
    query.prepare("insert into users(id,pw)values(:Name,:PW)");
    query.bindValue(":Name",name);
    query.bindValue(":PW",password);
    bool flag=query.exec();
    qDebug()<<"add user"<<(flag?"success":"fail");
    DATA_PACKAGE pack;
    pack.ms_type=flag?MS_TYPE::REGISTER_RE_T:MS_TYPE::REGISTER_RE_F;
    tcp.SendMessage(socket,pack);
    return flag;
}



bool Data::Login(QString name, QString password)
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
    DATA_PACKAGE pack;
    pack.ms_type=flag?MS_TYPE::LOGIN_RE_T:MS_TYPE::LOGIN_RE_F;
    tcp.SendMessage(socket,pack);
    return flag;
}


bool Data::ChangePassword(QString name, QString password)
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
