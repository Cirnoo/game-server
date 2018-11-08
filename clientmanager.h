#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QObject>
class QTcpSocket;
class Data;
class ClientManager : public QObject
{
    Q_OBJECT
public:
    explicit ClientManager(QTcpSocket *parent, Data & _data);
private:
    QTcpSocket * const socket;
    Data & data;
public slots:
    void readMessage();
};
#endif // CLIENTMANAGER_H
