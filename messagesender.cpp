/************************************************
����: MessageSend
����: ������Ϣ
************************************************/

#include "messagesender.h"
#include <QUdpSocket>
#include <QCryptographicHash>
#include <QThread>
#include <QMessageBox>
#include <QTime>
#include <QCoreApplication>
#include <QDebug>
#include "msgbox.h"

MessageSend::MessageSend(QObject *parent) :
    QObject(parent)
{
    //! [0]
    udpSocketSend = new QUdpSocket(this);
    if(!udpSocketSend->bind(0)) //�˿���Ϊ��,ϵͳ�ͻ��������һ��û��ռ�õĶ˿ڵ�;
    {
        qDebug() << "MessageSend bind false.";
    }
    //! [0]
}

/************************************************
���ܣ�
    ����Ϣ���ݷ��͸�ָ������ (����)
������
    data: ��Ϣ����
    phoneNumber: �绰����
����ֵ:
    �ɹ�����true�����򷵻�false
************************************************/
bool MessageSend::sendMessage(QString &data, QString phoneNumber)
{
    if (g_phoneBook->isConflict()) {
        qDebug() << "Your phonenumber conflicts with another person" << __FILE__ << __LINE__;
        return false;
    }
    if (!g_phoneBook->getMyself().isValid()) {
        qDebug() << "Myself is not valid" << __FILE__ << __LINE__;
        return false;
    }
    if (g_msgbox->isRepair()) {
        qDebug() << "MsgBox is repairing" << __FILE__ << __LINE__;
        return false;
    }
    //! [1]
    //xml��ʽ��֯����
    QString xmlDataString = domCreateXmlString(data);

    //���У�飬�������ݵ�MD5ֵ��32���ַ���,�������ݵ������Ժ���ȷ��У��
    QString md5(QCryptographicHash::hash
                   (xmlDataString.toLatin1(),
                    QCryptographicHash::Md5).toHex());
    //! [1]

//    //! [2]
//    //����phoneNumber��ѯ�Է���IP�����ﶨ��һ��ȷ����IP�Ͷ˿ڣ����ڵ�����ʾ
////    QHostAddress RemoteIp = QHostAddress::LocalHost;
//    QHostAddress RemoteIp = g_phoneBook->getContactIpAddr(phoneNumber);
//    if (RemoteIp.isNull()) {
//        qDebug() << "Remote ip is null" << phoneNumber << __FILE__ << __LINE__;
//        continue;
//    }
//    //! [2]
    //! [3]
    return udpSendData((md5+xmlDataString),phoneNumber);
    //! [3]
}

/************************************************
���ܣ�
    ����Ϣ���ݷ��͸�ָ�������б�Ⱥ����
������
    data: ��Ϣ����
    phoneNumbers: �绰�����б�
����ֵ:
    ����һ���绰�б���¼����ʧ�ܵĵ绰
************************************************/
QList<QString> MessageSend::sendMessage(QString &data, QList<QString> phoneNumbers)
{
    if (g_phoneBook->isConflict()) {
        qDebug() << "Your phonenumber conflicts with another person" << __FILE__ << __LINE__;
        return phoneNumbers;
    }
    if (!g_phoneBook->getMyself().isValid()) {
        qDebug() << "Myself is not valid" << __FILE__ << __LINE__;
        return phoneNumbers;
    }
    if (g_msgbox->isRepair()) {
        qDebug() << "MsgBox is repairing" << __FILE__ << __LINE__;
        return phoneNumbers;
    }
    QList<QString> failPhoneNumber ;
    failPhoneNumber.clear();

    //! [1]
    //xml��ʽ��֯����
    QString xmlDataString = domCreateXmlString(data);

    //���У�飬�������ݵ�MD5ֵ��32���ַ���,�������ݵ������Ժ���ȷ��У��
    QString md5(QCryptographicHash::hash
                   (xmlDataString.toLatin1(),
                    QCryptographicHash::Md5).toHex());
    //! [1]
    //������Ͷ���
    foreach(QString phoneNumber,phoneNumbers)
    {
//        //! [2]
//        //����phoneNumber��ѯ�Է���IP�����ﶨ��һ��ȷ����IP�Ͷ˿ڣ����ڵ�����ʾ
////        QHostAddress RemoteIp = QHostAddress::LocalHost;
//        QHostAddress RemoteIp = g_phoneBook->getContactIpAddr(phoneNumber);
//        if (RemoteIp.isNull()) {
//            qDebug() << "Remote ip is null" << phoneNumber << __FILE__ << __LINE__;
//            continue;
//        }
        
//        //! [2]
        //! [3]
        if(!udpSendData((md5+xmlDataString),phoneNumber))//�������ʧ�ܣ����¼����
        {
            failPhoneNumber << phoneNumber;
        }
        //! [3]

    }
    return failPhoneNumber;

}
/************************************************
���ܣ�
    �����ݽ���Xml��ʽ��װ
������
    messageContent: ��Ϣ����
����ֵ:
    ���ط�װ�������
************************************************/
QString MessageSend::domCreateXmlString( QString messageContent)
{
    //xml��ʽ��֯����
    QDomDocument domDocument;
    //�������ڵ�
    QDomElement root = domDocument.createElement("Sms");
    domDocument.appendChild(root) ;
    //����Ԫ�ؽڵ�
    //�����˺���
    QDomElement number = domDocument.createElement("Number");
    QDomText number_text = domDocument.createTextNode(g_phoneBook->getMyself().getCurrentPhoneNum());
    number.appendChild(number_text);
    root.appendChild(number);
//       //���Ͷ���ʱ�䣨���÷�������ʱ�䣬�����յ�ʱ�䣿��
//       QDomElement time = domDocument.createElement("time");
//       QDomText time_text = domDocument.createTextNode(
//                   QDate::currentDate().toString("yyyy/MM/dd") +
//                   " "+
//                   QTime::currentTime().toString("HH:ss"));
//       time.appendChild(time_text);
//       root.appendChild(time);
    //��������
    QDomElement content = domDocument.createElement("content");
    QDomText content_text = domDocument.createTextNode(messageContent);
    content.appendChild(content_text);
    root.appendChild(content);

    QString xmldatatostring = domDocument.toString();
    qDebug() << "MessageSend build xmldata is:" <<xmldatatostring <<__FILE__<<__LINE__;
    return xmldatatostring;
}

/************************************************
���ܣ�
    ͨ��UDP����Ϣ���ݷ��͸�ָ��IP
������
    data: ��Ϣ����
    remoteIp: Զ��IP��
����ֵ:
    ���ͳɹ�����true�����򷵻�false
************************************************/
bool MessageSend::udpSendData(QString data, QString phoneNumber)
{

    //����phoneNumber��ѯ�Է���IP�����ﶨ��һ��ȷ����IP�Ͷ˿ڣ����ڵ�����ʾ
    //        QHostAddress RemoteIp = QHostAddress::LocalHost;
    QHostAddress remoteIp = g_phoneBook->getContactIpAddr(g_phoneBook->getContactOfPhoneNum(phoneNumber).id);
    if (remoteIp.isNull()) {
        qDebug() << "Remote ip is null, phonenum:" << phoneNumber << __FILE__ << __LINE__;
        return false;
    }

    int length = udpSocketSend->writeDatagram(data.toUtf8().data(),
                                   remoteIp,
                                   cons_MessageListenport);
    if(length <0)
    {
          qDebug()<< "udpSocketSend send fail." 
                  << "RemoteIp" << remoteIp
                  << "Error"
                  << udpSocketSend->error()
                  << udpSocketSend->errorString() << __FILE__ << __LINE__;
          return false;
    }

    /*�����꣬��ʱ�ȴ���Ӧ��
    �����û�л�Ӧ����Ϊ����ʧ�ܣ��ٴη��ͣ�
    ��3�η���ʧ�ܣ��򱣳ֵ��ݸ��䣨������δ�������䣩*/
    int count = 3;
    while(count--)
    {
        //����һ�£��ȴ���Ӧ
        QTime t;
        t.start();
        while(t.elapsed()<200);
        if(udpSocketSend->hasPendingDatagrams())
        {
            QByteArray dataReturn;
            //udpSocket->pendingDatagramSize ��ȡ���ĳ���
            //data.resize �� data �������ó���
            dataReturn.resize(udpSocketSend->pendingDatagramSize());
            //��������
            udpSocketSend->readDatagram(dataReturn.data(),
                                       dataReturn.size());
            //��÷�����Ϣ
            QString dataString = dataReturn.data();
            qDebug() << " receive return :" << dataString<< __FILE__ << __LINE__;
            if(QString::compare(dataString,"right") == 0)
            {
                 //�������ݵ��ѷ���
                 qDebug()<<"send Message success.";
                 return true;
            }
            else if(QString::compare(dataString,"wrong") == 0)
            {
                //�ط�
//                count++;
                udpSocketSend->writeDatagram(data.toLatin1(),
                                             remoteIp,
                                             cons_MessageListenport);
                qDebug()<<"send Message wrong ,resend";
            }
        } else {
            qDebug() << "no pending data" << __FILE__ << __LINE__;
        }

    }
    qDebug()<<"send Message fail.";
    return false;

}
