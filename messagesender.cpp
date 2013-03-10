/************************************************
类名: MessageSend
功能: 发送信息
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
    if(!udpSocketSend->bind(0)) //端口设为零,系统就会随机分配一个没有占用的端口的;
    {
        qDebug() << "MessageSend bind false.";
    }
    //! [0]
}

/************************************************
功能：
    将信息内容发送给指定号码 (单发)
参数：
    data: 信息内容
    phoneNumber: 电话号码
返回值:
    成功返回true，否则返回false
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
    //xml格式组织数据
    QString xmlDataString = domCreateXmlString(data);

    //添加校验，计算数据的MD5值（32个字符）,用于数据的完整性和正确性校验
    QString md5(QCryptographicHash::hash
                   (xmlDataString.toLatin1(),
                    QCryptographicHash::Md5).toHex());
    //! [1]

//    //! [2]
//    //根据phoneNumber查询对方的IP，这里定义一个确定的IP和端口，用于单机演示
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
功能：
    将信息内容发送给指定号码列表（群发）
参数：
    data: 信息内容
    phoneNumbers: 电话号码列表
返回值:
    返回一个电话列表，记录发送失败的电话
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
    //xml格式组织数据
    QString xmlDataString = domCreateXmlString(data);

    //添加校验，计算数据的MD5值（32个字符）,用于数据的完整性和正确性校验
    QString md5(QCryptographicHash::hash
                   (xmlDataString.toLatin1(),
                    QCryptographicHash::Md5).toHex());
    //! [1]
    //逐个发送短信
    foreach(QString phoneNumber,phoneNumbers)
    {
//        //! [2]
//        //根据phoneNumber查询对方的IP，这里定义一个确定的IP和端口，用于单机演示
////        QHostAddress RemoteIp = QHostAddress::LocalHost;
//        QHostAddress RemoteIp = g_phoneBook->getContactIpAddr(phoneNumber);
//        if (RemoteIp.isNull()) {
//            qDebug() << "Remote ip is null" << phoneNumber << __FILE__ << __LINE__;
//            continue;
//        }
        
//        //! [2]
        //! [3]
        if(!udpSendData((md5+xmlDataString),phoneNumber))//如果发送失败，则记录下来
        {
            failPhoneNumber << phoneNumber;
        }
        //! [3]

    }
    return failPhoneNumber;

}
/************************************************
功能：
    对数据进行Xml格式封装
参数：
    messageContent: 信息内容
返回值:
    返回封装后的数据
************************************************/
QString MessageSend::domCreateXmlString( QString messageContent)
{
    //xml格式组织数据
    QDomDocument domDocument;
    //创建根节点
    QDomElement root = domDocument.createElement("Sms");
    domDocument.appendChild(root) ;
    //创建元素节点
    //发件人号码
    QDomElement number = domDocument.createElement("Number");
    QDomText number_text = domDocument.createTextNode(g_phoneBook->getMyself().getCurrentPhoneNum());
    number.appendChild(number_text);
    root.appendChild(number);
//       //发送短信时间（采用发件发送时间，还是收到时间？）
//       QDomElement time = domDocument.createElement("time");
//       QDomText time_text = domDocument.createTextNode(
//                   QDate::currentDate().toString("yyyy/MM/dd") +
//                   " "+
//                   QTime::currentTime().toString("HH:ss"));
//       time.appendChild(time_text);
//       root.appendChild(time);
    //短信内容
    QDomElement content = domDocument.createElement("content");
    QDomText content_text = domDocument.createTextNode(messageContent);
    content.appendChild(content_text);
    root.appendChild(content);

    QString xmldatatostring = domDocument.toString();
    qDebug() << "MessageSend build xmldata is:" <<xmldatatostring <<__FILE__<<__LINE__;
    return xmldatatostring;
}

/************************************************
功能：
    通过UDP将信息内容发送给指定IP
参数：
    data: 信息内容
    remoteIp: 远程IP、
返回值:
    发送成功返回true，否则返回false
************************************************/
bool MessageSend::udpSendData(QString data, QString phoneNumber)
{

    //根据phoneNumber查询对方的IP，这里定义一个确定的IP和端口，用于单机演示
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

    /*发送完，定时等待回应，
    ①如果没有回应，认为发送失败，再次发送，
    ②3次发送失败，则保持到草稿箱（或者是未发送信箱）*/
    int count = 3;
    while(count--)
    {
        //休眠一下，等待回应
        QTime t;
        t.start();
        while(t.elapsed()<200);
        if(udpSocketSend->hasPendingDatagrams())
        {
            QByteArray dataReturn;
            //udpSocket->pendingDatagramSize 获取报文长度
            //data.resize 给 data 数组设置长度
            dataReturn.resize(udpSocketSend->pendingDatagramSize());
            //读入数据
            udpSocketSend->readDatagram(dataReturn.data(),
                                       dataReturn.size());
            //获得返回信息
            QString dataString = dataReturn.data();
            qDebug() << " receive return :" << dataString<< __FILE__ << __LINE__;
            if(QString::compare(dataString,"right") == 0)
            {
                 //保持数据到已发送
                 qDebug()<<"send Message success.";
                 return true;
            }
            else if(QString::compare(dataString,"wrong") == 0)
            {
                //重发
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
