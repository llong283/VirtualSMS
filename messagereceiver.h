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
    
    QUdpSocket  *udpSocketRev;  //����UDPЭ�飬���ڶ��ŵĽ���
//    int socketListenPort ;      //ͨ�Ŷ˿�
    bool domParseXmlString( QString &data);   //DOM��ʽ����XMl��ʽ���ַ���
    void messageFilter(int senderPhoneNum, QString & messageContent); //��Ϣ����

public slots:
   void processPendingDatagram();  //��ȡ�յ���udpͨ������

};

#endif // MESSAGERECEIVEANDSEND_H
