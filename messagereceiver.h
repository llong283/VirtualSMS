#ifndef MESSAGERECEIVEANDSEND_H
#define MESSAGERECEIVEANDSEND_H

#include <QAbstractSocket>
#include <QUdpSocket>
#include "config.h"
#include "setting.h"
#include "Filter.h"

#define g_receiver MessageReceiver::instance()

class MessageReceiver : public QObject
{
    Q_OBJECT

public :
   //explicit MessageReceiver(QObject *parent = 0,int port);
    static MessageReceiver *instance();
    
signals:
    void signNewMsgArrived(const QString&, const QString&);

private:
    explicit MessageReceiver(QObject *parent = 0);
    static MessageReceiver *messageReceiver;
    
    QUdpSocket  *udpSocketRev;  //采用UDP协议，用于短信的接收
//    int socketListenPort ;      //通信端口
    bool domParseXmlString( QString &data);   //DOM方式解析XMl格式的字符串
    void messageFilter(int senderPhoneNum, QString & messageContent); //信息过滤

public slots:
   void processPendingDatagram();  //读取收到的udp通信数据

};

#endif // MESSAGERECEIVEANDSEND_H
