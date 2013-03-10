#ifndef UDPBROADCAST_H
#define UDPBROADCAST_H

#include <QObject>
#include <QtNetwork>
#include "phonebook.h"


#define g_udpbroadcast udpbroadcast::instance()

class udpbroadcast : public QObject
{
    Q_OBJECT
public:
    explicit udpbroadcast(QObject *parent = 0);
    static udpbroadcast* instance();
    void broadcastInformation(Command command);    
    bool checkConflict(const QString &phoneNum=QString());
    
signals:
//    void newContactInformation(const QString&, const QString&, const QString&, const QHostAddress&);
    void signPhoneNumConflict(const QString&, const QHostAddress&);
    void signContactLogin(const QString&);
    
public slots:

private:
    static udpbroadcast* _instance;
    QUdpSocket *udpSocketBroadcastRecvd;   //���ڼ����㲥��Ϣ
    QUdpSocket *udpSocketBroadcastSender;  //���ڷ��͹㲥��Ϣ
    QUdpSocket *m_udpCheckConflict;
    const int broadcastPort;      //�㲥�˿�
    const QString protocolHead;   //�㲥ͨ��Э��ͷ
    QTimer m_timer;

    void sendInfo(Command command, const QHostAddress &ipAddr);
    Command parseCommand(const QString &strCommand);
    QString generateCommand(Command command);
    Command processInfo(const QByteArray &byteArray, const QHostAddress &ipAddr, quint16 port);
    
private slots:
    void slotProcessIncomingInfo();
    void slotBroadcastInformation();
};

#endif // UDPBROADCAST_H
