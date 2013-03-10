#ifndef MESSAGESEND_H
#define MESSAGESEND_H

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>
#include "config.h"
#include "setting.h"

class MessageSend : public QObject
{
    Q_OBJECT
public:
    explicit MessageSend(QObject *parent = 0);

    
signals:

private:
    QUdpSocket *udpSocketSend; //采用UDP协议，用于短信的发送
    QString domCreateXmlString(QString messageContent );
    bool udpSendData(QString data, QString phoneNumber);


//    int socketRemotePort ; //对方监听端口
//    QHostAddress *RemoteIp;

public slots:
//    bool sendMessage(QString &data, QHostAddress RemoteIp, int RemotePort);
    bool sendMessage(QString &data, QString phoneNumber); //用于单发
    QList<QString> sendMessage(QString &data, QList<QString> phoneNumbers); //用于群发

};

#endif // MESSAGESEND_H
