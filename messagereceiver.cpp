/************************************************
����: MessageReceiver
����: ������������Ϸ���������Ϣ������Ϣ���й��˺��䱣��
************************************************/

#include "messagereceiver.h"

#include <QAbstractSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QHostInfo>
#include <QCryptographicHash>
#include <QDebug>
#include <QDomDocument>
#include <QMessageBox>
#include <QWidget>

#include "msgbox.h"

MessageReceiver* MessageReceiver::messageReceiver = NULL;

/************************************************
���ܣ�
    MessageReceiver������ʼ��������̬����
������
    ��
����ֵ:
    ����MessageReceiver����ָ��
************************************************/
MessageReceiver *MessageReceiver::instance()
{
    if (messageReceiver == NULL) {
        messageReceiver = new MessageReceiver;
    }
    return messageReceiver;
}

MessageReceiver::MessageReceiver(QObject *parent) :
    QObject(parent)
{
     udpSocketRev = new QUdpSocket(this);
     bool conn =udpSocketRev->bind(cons_MessageListenport);
     //����ʧ��
     if(!conn){
         qDebug() << "UDP���ճ�ʼ��ʧ��.";
     }else{
         //��udpSocketRev���źŹ�������
         connect(udpSocketRev,SIGNAL(readyRead()),this,SLOT(processPendingDatagram()));
         qDebug()<< "UDP���ճ�ʼ���ɹ�.";
//         qDebug() << "udpSocketRev address :" << udpSocketRev->localAddress().toString();
//         qDebug() << "udpSocketRev port :" <<udpSocketRev->localPort();
     }
}

/************************************************
���ܣ�
    ������յ�����Ϣ
    1 �����������������ڶ˿�������UDP�����������յ����ݵ��źźʹ������ݵĲ�����������
    2 ��֤���������˿��յ����ݺ󣬷����źţ������������ݵĲۺ��������ȶ����ݵĵ������Խ�����֤��ȷ�����ݴ������û�����ݶ�ʧ��
    3 Xml��������֤��ȷ�󣬶���Ϣ���ݰ�xml��ʽ���н�������ȡ������Ҫ�Ĳ������Ӷ�ȷ�����ݵ���ȷ�ԣ�
    4 ���ˣ��Խ��������Ϣ���绰��������ģ���е����ý��й��ˣ��Ӷ�����Ϣ���з��ࣻ
    5 ������ʾ������Ϣ���й��˷���󣬽�����Ӧ�ı������ʾ��
������
    ��
����ֵ:
    ��
************************************************/
void MessageReceiver::processPendingDatagram()
{
    if (g_phoneBook->isConflict()) {
        qDebug() << "Your phonenumber conflicts with another person. Abort data." << __FILE__ << __LINE__;
        return;
    }
    if (g_msgbox->isRepair()) {
        qDebug() << "MsgBox is repairing" << __FILE__ << __LINE__;
        return;
    }
  
    while(udpSocketRev->hasPendingDatagrams())
    {
        QHostAddress  remoteAddress ;
        quint16 remotePort;
        //�Ƿ��������
        QByteArray dataRevd;
        //udpSocket->pendingDatagramSize ��ȡ���ĳ���
        //data.resize �� data �������ó���
        dataRevd.resize(udpSocketRev->pendingDatagramSize());
        //��������
        udpSocketRev->readDatagram(dataRevd.data(),
                                   dataRevd.size(),
                                   &remoteAddress,
                                   &remotePort);
        qDebug() << " remoteAddress is :" << remoteAddress.toString()<< __FILE__ << __LINE__;
        qDebug() << " remotePort is :" << remotePort<< __FILE__ << __LINE__;
        //�����������,md5(32���ַ�)+����
//        QString dataString = dataRevd.data();
        QTextCodec *tc=QTextCodec::codecForName("UTF-8");
        QString dataString = tc->toUnicode(dataRevd);
        
        qDebug() << "receive data and md5 string is :" << dataString << __FILE__ << __LINE__;
        QString md5(QCryptographicHash::hash
                       (dataString.mid(32).toLatin1(),
                        QCryptographicHash::Md5).toHex());
//        qDebug() << "receive message data is :"<< dataString.mid(32);
        qDebug() << "receive computer md5 is :"<< md5;

        if(QString::compare(md5,dataString.mid(0,32))==0)
        {
            qDebug()<<"UDP receive Md5 right."<< __FILE__ << __LINE__;

           QString messageData = dataString.mid(32);
           qDebug() << "UDP receive string is :" << messageData << __FILE__<<__LINE__;
           //�����˿���Ϣ
           if(domParseXmlString(messageData))
           {
               QString returnString = "right";
               udpSocketRev->writeDatagram(returnString.toLatin1(),
                                           returnString.length(),
                                           remoteAddress,
                                           remotePort
                                          );
           }
        }
        else
        {
            qDebug()<<"UDP receive wrong, return wrong, need resend."<< __FILE__ << __LINE__;
            QString returnString = "wrong";
            udpSocketRev->writeDatagram(returnString.toLatin1(),
                                        returnString.length(),
                                        remoteAddress,
                                        remotePort
                                        );
        }
     }
}

/************************************************
���ܣ�
    ����Ϣ���ݰ�xml��ʽ���н�������ȡ������Ҫ�Ĳ��������б���
������
    messageData: ��Ϣ����
����ֵ:
    �����ɹ�����true�����򷵻�false
************************************************/
bool MessageReceiver::domParseXmlString( QString &messageData)
{
    //����
    QDomDocument domDocument;
    QString errorStr;
    QString phoneNumber;
    QString messageContent;

    int errorLine;
    int errorColumn;

    //QDomDocument��ʽ����XML�ļ��������������ݴ���QDomDocument��
    if (!domDocument.setContent(messageData, true, &errorStr, &errorLine,
                                &errorColumn)) {
        QMessageBox::information(NULL, tr("DOM Bookmarks"),
                                 tr("Parse error at line") 
                                 + QString::number(errorLine)
                                 + tr("column") + QString("%1:\n%2")
                                 .arg(errorColumn)
                                 .arg(errorStr));
        return false;
    }
    //��ĵ��ĸ��ڵ�
    QDomElement root = domDocument.documentElement();
    if(root.tagName()!= "Sms")
    {
        qDebug() << "UDP receive root wrong is:" <<root.tagName() ;
        return false;
    }
    else
        qDebug() << "UDP receive root right is:" <<root.tagName() ;

    QDomElement child = root.firstChildElement();
    if(child.tagName()!= "Number")
    {
        qDebug() << "UDP receive ChildElement Number wrong is:" << child.tagName() ;
        return false;
    }
    else
        qDebug() << "UDP receive ChildElement Number right,number is :" << child.text();

    phoneNumber = child.text();
    child = child.nextSiblingElement();
    if(child.tagName()!= "content")
    {
        qDebug() << "UDP receive ChildElement content wrong is:" << child.tagName() ;
        return false;
    }
    else
        qDebug() << "UDP receive ChildElement content right,content is :" << child.text();
    messageContent = child.text();

    emit signNewMsgArrived(phoneNumber, messageContent);
    return true;
}


