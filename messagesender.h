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
    QUdpSocket *udpSocketSend; //����UDPЭ�飬���ڶ��ŵķ���
    QString domCreateXmlString(QString messageContent );
    bool udpSendData(QString data, QString phoneNumber);


//    int socketRemotePort ; //�Է������˿�
//    QHostAddress *RemoteIp;

public slots:
//    bool sendMessage(QString &data, QHostAddress RemoteIp, int RemotePort);
    bool sendMessage(QString &data, QString phoneNumber); //���ڵ���
    QList<QString> sendMessage(QString &data, QList<QString> phoneNumbers); //����Ⱥ��

};

#endif // MESSAGESEND_H
