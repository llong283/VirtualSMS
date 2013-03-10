/************************************************
类名: MessageReceiver
功能: 负责接收网络上发过来的信息，对信息进行过滤后将其保存
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
功能：
    MessageReceiver单例初始化，属静态函数
参数：
    无
返回值:
    返回MessageReceiver单例指针
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
     //链接失败
     if(!conn){
         qDebug() << "UDP接收初始化失败.";
     }else{
         //把udpSocketRev的信号关联到槽
         connect(udpSocketRev,SIGNAL(readyRead()),this,SLOT(processPendingDatagram()));
         qDebug()<< "UDP接收初始化成功.";
//         qDebug() << "udpSocketRev address :" << udpSocketRev->localAddress().toString();
//         qDebug() << "udpSocketRev port :" <<udpSocketRev->localPort();
     }
}

/************************************************
功能：
    处理接收到的信息
    1 监听：程序启动后，在端口上申请UDP监听，并将收到数据的信号和处理数据的槽连接起来；
    2 验证：当监听端口收到数据后，发送信号，启动处理数据的槽函数，首先对数据的的完整性进行验证，确保数据传输过程没有数据丢失；
    3 Xml解析：验证正确后，对信息内容按xml格式进行解析，提取出所需要的参数，从而确保数据的正确性；
    4 过滤：对解析后的信息按电话本和设置模块中的配置进行过滤，从而对信息进行分类；
    5 保存显示：对信息进行过滤分类后，进行相应的保存和显示。
参数：
    无
返回值:
    无
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
        //是否读到数据
        QByteArray dataRevd;
        //udpSocket->pendingDatagramSize 获取报文长度
        //data.resize 给 data 数组设置长度
        dataRevd.resize(udpSocketRev->pendingDatagramSize());
        //读入数据
        udpSocketRev->readDatagram(dataRevd.data(),
                                   dataRevd.size(),
                                   &remoteAddress,
                                   &remotePort);
        qDebug() << " remoteAddress is :" << remoteAddress.toString()<< __FILE__ << __LINE__;
        qDebug() << " remotePort is :" << remotePort<< __FILE__ << __LINE__;
        //获得数据内容,md5(32个字符)+数据
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
           //分析端口信息
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
功能：
    对信息内容按xml格式进行解析，提取出所需要的参数并进行保存
参数：
    messageData: 信息内容
返回值:
    解析成功返回true，否则返回false
************************************************/
bool MessageReceiver::domParseXmlString( QString &messageData)
{
    //解析
    QDomDocument domDocument;
    QString errorStr;
    QString phoneNumber;
    QString messageContent;

    int errorLine;
    int errorColumn;

    //QDomDocument方式解析XML文件，并将解析内容存入QDomDocument中
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
    //活动文档的根节点
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


