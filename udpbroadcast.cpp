#include "udpbroadcast.h"

udpbroadcast* udpbroadcast::_instance = NULL;

const int cons_command_port = 1574;

/************************************************
���ܣ�
    udpbroadcast������ʼ��������̬����
������
    ��
����ֵ:
    ����udpbroadcast����ָ��
************************************************/
udpbroadcast* udpbroadcast::instance()
{
    if(_instance == NULL)
    {
        _instance = new udpbroadcast;
    }
    return _instance;
}

udpbroadcast::udpbroadcast(QObject *parent) :
    QObject(parent),broadcastPort(cons_broadcast_port),protocolHead("phonenumber")
{
    udpSocketBroadcastSender = new QUdpSocket(this);
    if(!udpSocketBroadcastSender->bind(0)) { //�˿���Ϊ��,ϵͳ�ͻ��������һ��û��ռ�õĶ˿ڵ�;
        qDebug() << "udpSocketBroadcastSender bind false.";
    }

    udpSocketBroadcastRecvd  = new QUdpSocket(this);
    if (!udpSocketBroadcastRecvd->bind(broadcastPort, QUdpSocket::ReuseAddressHint)) {
        qDebug() << "broadcast bind failed" 
                 << "Error"
                 << udpSocketBroadcastRecvd->error() 
                 << udpSocketBroadcastRecvd->errorString()
                 << __FILE__ << __LINE__;
    }
    connect(udpSocketBroadcastRecvd, SIGNAL(readyRead()),
            this, SLOT(slotProcessIncomingInfo()));

    m_udpCheckConflict  = new QUdpSocket(this);
    if (!m_udpCheckConflict->bind(0)) {
        qDebug() << "m_udpCheckConflict bind failed" 
                 << "Error"
                 << m_udpCheckConflict->error() 
                 << m_udpCheckConflict->errorString()
                 << __FILE__ << __LINE__;
    }

    //��ʱ1s�㲥������Ϣ
    m_timer.setInterval(1 * 1000);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotBroadcastInformation()));
    m_timer.start();
}

void udpbroadcast::slotBroadcastInformation()
{
    broadcastInformation(Command_Info);
}

/************************************************
���ܣ�
    ������յ��Ĺ㲥��Ϣ�Ĳۺ���
    1 �������յ����ݵ�����źţ���ȡ���ݣ�
    2 ����������Э������ݸ�ʽ(protocolHead#CurrentPhoneNum#PreviousPhoneNum), ��֤���������ݣ�
    3 �����źţ���֤��ȷ�󣬷����źţ�֪ͨ���ӵĲۺ����������û���Ϣ������+remoteIP����
������
    ��
����ֵ:
    ��
************************************************/
void udpbroadcast::slotProcessIncomingInfo()
{
    QHostAddress  remoteAddress ;
    quint16 remotePort;
    while (udpSocketBroadcastRecvd->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocketBroadcastRecvd->pendingDatagramSize());
        udpSocketBroadcastRecvd->readDatagram(datagram.data(),
                                datagram.size(),
                                &remoteAddress,
                                &remotePort);
        processInfo(datagram, remoteAddress, remotePort);
    }
}

//�����յ��Ĺ㲥��Ϣ
Command udpbroadcast::processInfo(const QByteArray &byteArray, const QHostAddress &ipAddr, quint16 port)
{
    QList<QHostAddress> localAddrList = QNetworkInterface::allAddresses();
    if (localAddrList.contains(ipAddr)) {            
        return Command_Invalid;
    }
    QTextCodec *tc = QTextCodec::codecForName("utf-8");
    QString strInfo = tc->toUnicode(byteArray);
    QStringList broadList = strInfo.split("#");
    Command command;
    if (broadList.count()==Broadcast_Size) {
        Contact contact = g_phoneBook->getContactOfPhoneNum(
                    broadList.at(Broadcast_CurPhoneNum));
        QDateTime currentTime = QDateTime::currentDateTime();
        command = parseCommand(broadList.at(Broadcast_Command));
        switch (command) {
        case Command_Info:
            if (contact.isValid()) {
                g_phoneBook->changeState(contact.id, Contact::State_Online, ipAddr, currentTime);
                //��ϵ�˵�½�����ڸı�״̬���ͣ��������ܻ�ȡip
                if (contact.state == Contact::State_Offline) {
                    qDebug() << "login" << __FILE__ << __LINE__;
                    emit signContactLogin(broadList.at(Broadcast_CurPhoneNum));
                }
            } else {
                contact = g_phoneBook->getContactOfPhoneNum(broadList.at(Broadcast_PrevPhoneNum));
                if (contact.isValid()) { //��ϵ���޸��˺���
                    qDebug() << "Contact phone number changed. Update it." << __FILE__ << __LINE__;
                    g_phoneBook->changePhoneNum(contact.id, broadList.at(Broadcast_CurPhoneNum));
                    g_phoneBook->changeState(contact.id, Contact::State_Online, ipAddr, currentTime);
                } else {
                    //İ���˵�½
                    g_phoneBook->addContact(broadList.at(Broadcast_Name),
                                            broadList.at(Broadcast_CurPhoneNum),
                                            Contact::Type_Stranger);
                    emit signContactLogin(broadList.at(Broadcast_CurPhoneNum));
                }
            }
            break;
        case Command_Quit:
            if (contact.type == Contact::Type_Stranger) {
                g_phoneBook->removeContact(contact.id);
            } else {
                g_phoneBook->changeState(contact.id, Contact::State_Offline, ipAddr, currentTime);
            }
            break;
        case Command_CheckConflict:
            if (contact.type == Contact::Type_Myself) {
                Contact myself = g_phoneBook->getMyself();
                QString strConflictInfo;
                if (myself.isValid()) {
                    strConflictInfo = QString("%1#%2#%3")
                            .arg(generateCommand(Command_ReplyConflict))
                            .arg(myself.name)
                            .arg(myself.phonenum);
                    m_udpCheckConflict->writeDatagram(strConflictInfo.toUtf8().data(),
                                                      ipAddr,
                                                      port);
                } else {
                    qDebug() << "Myself is not valid" << __FILE__ << __LINE__;
                }
            }
            break;
        case Command_ReplyConflict:
            emit signPhoneNumConflict(broadList.at(Broadcast_Name), ipAddr);
            break;
        default:
            qDebug() << "invalid broadcast information" << strInfo << ipAddr << __FILE__ << __LINE__;
            break;
        }            
    } else {
//        qDebug() << "invalid broadcast information" << strInfo << ipAddr << __FILE__ << __LINE__;
    }
    return command;
}

Command udpbroadcast::parseCommand(const QString &strCommand)
{
    Command command;
    if (strCommand == cons_info) {
        command = Command_Info;
    } else if (strCommand == cons_quit) {
        command = Command_Quit;
    } else if (strCommand == cons_check_conflict) {
        command = Command_CheckConflict;
    } else if (strCommand == cons_reply_conflict) {
        command = Command_ReplyConflict;
    } else {
        command = Command_Invalid;
    }
    return command;
}

QString udpbroadcast::generateCommand(Command command)
{
    QString strCommand;
    switch (command) {
    case Command_Info:
        strCommand = cons_info;
        break;
    case Command_Quit:
        strCommand = cons_quit;
        break;
    case Command_CheckConflict:
        strCommand = cons_check_conflict;
        break;
    case Command_ReplyConflict:
        strCommand = cons_reply_conflict;
        break;
    default:
        qDebug() << "this is a bug" << __FILE__ << __LINE__;
        break;
    }
    return strCommand;    
}

/************************************************
���ܣ�
    �㲥�Լ�����Ϣ
    ����Э������ݸ�ʽ(protocolHead#name#currentPhoneNum#previousPhoneNum), �㲥���ݣ�
������
    ��
����ֵ:
    ��
************************************************/
void udpbroadcast::broadcastInformation(Command command)
{
    Contact myself = g_phoneBook->getMyself();
    if (myself.isValid()) {
        QString data = QString("%1#%2#%3")
                .arg(generateCommand(command))
                .arg(myself.name)
                .arg(myself.phonenum);
        udpSocketBroadcastSender->writeDatagram(data.toUtf8().data(),
                                                QHostAddress::Broadcast,
                                                broadcastPort);
//        qDebug() << data << broadcastPort << __FILE__ << __LINE__;
    }
}

bool udpbroadcast::checkConflict(const QString &phoneNum)
{
    qDebug() << "checkConflict" << __FILE__ << __LINE__;
    QString strInfo;
    if (phoneNum.isEmpty()) {
        qDebug() << "checkConflict" << __FILE__ << __LINE__;
        //���ܺͺ��������һ����Ϊphonebook�ж�д����
        Contact myself = g_phoneBook->getMyself();
        qDebug() << "checkConflict" << __FILE__ << __LINE__;
        if (myself.isValid()) {
            strInfo = QString("%1#%2#%3")
                    .arg(generateCommand(Command_CheckConflict))
                    .arg(myself.name)
                    .arg(myself.phonenum);
        } else {
            return false;          
        }
    } else {
        strInfo = generateCommand(Command_CheckConflict) + "##" + phoneNum + "#";
    }
    qDebug() << "checkConflict" << __FILE__ << __LINE__;
    m_udpCheckConflict->writeDatagram(strInfo.toUtf8().data(),
                                      QHostAddress::Broadcast,
                                      broadcastPort);
    qDebug() << strInfo << __FILE__ << __LINE__;
    
    int count = 3;
    bool flag = false;
    while(count--) {
        //����һ�£��ȴ���Ӧ
        QTime t;
        t.start();
        while(t.elapsed()<100)
            QCoreApplication::processEvents();
        while (m_udpCheckConflict->hasPendingDatagrams()) {
            QByteArray dataReturn;
            QHostAddress  remoteAddress ;
            quint16 remotePort;
            dataReturn.resize(m_udpCheckConflict->pendingDatagramSize());
            //��������
            m_udpCheckConflict->readDatagram(dataReturn.data(),
                                             dataReturn.size(),
                                             &remoteAddress,
                                             &remotePort);
            if (!flag && processInfo(dataReturn, remoteAddress, remotePort) == Command_ReplyConflict) {
                flag = true;
            }
        }
        if (flag) {
            break;
        }
    }
    if (g_phoneBook->isConflict() != flag) {
        g_phoneBook->setConflict(flag);
    }
    qDebug() << "checkConflict" << __FILE__ << __LINE__;
    return flag;
}
