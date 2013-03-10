/************************************************
����: AbstractMsg
����: ������Ϣ�������ռ��䡢�����䡢�ݸ��䡢������
˵��: ����̳�CommonXmlProcess�࣬���ļ��������ڵ㡢��һ
     ����ǩ���ڶ�����ǩ����������г�ʼ������������ͨ�õĲ�
     �����廯���ṩ�����Ϣ��ɾ����Ϣ���������䡢������Ϣ��
     �����Խ��յ���Ϣ�����ܣ�ͬʱ�ṩ��ѯ���ܣ���������id��
     ѯ�����ݵ绰�����ѯ���������Ͳ�ѯ����ѯ������Ϣ������
     Ϊ��ʵ�ֿ��ٲ�ѯ��AbstractMsgά���˵绰����-id��ʱ��-id��
     ����-id����������ͬ�����¸�������Ϣ������
************************************************/

#include "abstractmsg.h"

const int cons_reserve_change_times = 100;

AbstractMsg::AbstractMsg(QObject *parent) :
    CommonXmlProcess(parent),
    m_boxesNum(4),
    m_unreadInboxNum(0),
    m_unreadDustbinNum(0),
    m_totalNum(0),
    m_changeTimes(0),
    m_isRepair(false),
    m_isAutoSave(false),
    m_isFileNormal(true)
{
}

/************************************************
���ܣ�
    ��ʼ��
������
    ��
����ֵ:
    ��ʼ���ɹ�����true, ���򷵻�false
************************************************/
bool AbstractMsg::initAbstractMsg(const QString &fileName, bool isAutoSave)
{
    m_msg.setWindowTitle(tr("msg"));
//    QString fileName(QApplication::applicationDirPath() + "/../Test/msgbox.xml");
    m_ltTags << "PhoneNum" << "DateTime" << "Content" << "MsgType" << "Box" << "State";    
    m_isInit = initCommon(fileName, "MsgBox", "Message", m_ltTags);
    if (!m_isInit) {
//        QMessageBox::information(NULL, tr("msg"), tr("AbstractMsg init failed. AbstractMsg will not work."));
        m_msg.setText(tr("AbstractMsg init failed. AbstractMsg will not work."));
        m_msg.show();
        return false;
    }
    if (isAutoSave) {
        m_isAutoSave = true;
        m_timerIdle.setInterval(10 * 1000);
        connect(&m_timerIdle, SIGNAL(timeout()), this, SLOT(save()));
        m_timerIdle.start();
        connect(this, SIGNAL(signSave()), this, SLOT(save()), Qt::QueuedConnection);    
    }
    createMap();
    return true;
}

/************************************************
���ܣ�
    �����Ϣ
    ��Ӻ���������͸�������Ϣ�����������ͱ�����ź�
������
    phonenum: �绰����
    datetime: ʱ��
    content: ����
    msgtype: ��Ϣ����
    box: ����
    state: ״̬
����ֵ:
    ��ӳɹ�������id, ���򷵻�-1
************************************************/
int AbstractMsg::addMessage(const QString &phonenum, 
                       const QDateTime &datetime, 
                       const QString &content, 
                       Message::MsgType msgtype,
                       Message::Box box,
                       Message::State state)
{
    QWriteLocker writeLocker(&m_lock);    
    if (!m_isInit) {
//        QMessageBox::information(NULL, tr("msg"), tr("AbstractMsg is not initialized"));
        m_msg.setText(tr("AbstractMsg is not initialized"));
        m_msg.show();
        return -1;
    }
    //������������
/*    if (phonenum.isEmpty()) { //�绰�����п���Ϊ�գ�����δ������Ϣʱ
        qDebug() << "phonenum is not empty. add msg failed" << __FILE__ << __LINE__;
        return -1;        
    } else */if (datetime.isNull() || !datetime.isValid()) {
        qDebug() << "datetime is not valid. add msg failed" << __FILE__ << __LINE__;
        return -1;        
    } else if (!checkValidity(msgtype, box, state)) {
        qDebug() << "The msg type or box or state is not right. add msg failed" << __FILE__ << __LINE__;
        return -1;
    }
    
    //����Ϣ�������ݴ����map������������һ������
    QMap<QString, QString> map;
    map.insert(m_ltTags.at(LTT_PhoneNum), phonenum);
    map.insert(m_ltTags.at(LTT_DateTime), datetime.toString("yyyy-MM-dd hh:mm:ss.zzz"));
    map.insert(m_ltTags.at(LTT_Content), content);
    map.insert(m_ltTags.at(LTT_MsgType), QString::number(msgtype));
    map.insert(m_ltTags.at(LTT_Box), QString::number(box));
    map.insert(m_ltTags.at(LTT_State), QString::number(state));
    
    int newId;
    if (m_dateTimeToIds.isEmpty() 
            || (!m_dateTimeToIds.isEmpty() && datetime<m_dateTimeToIds.begin().key())) {
        //����Ϣ���뵽��ǰ
        newId = insertElement(map, -1);
    } else {
        //��ʱ��-id�������в�ѯid��ʹ����Ϣ�������Ȼ��ʱ���������������ͬʱ�����Ϣ��
        //����뵽��ͬ��Ϣ�����
        int id = 0;
        QMultiMap<QDateTime, int>::iterator iter = --m_dateTimeToIds.end();
        if (datetime >= iter.key()) {
            id = iter.value();
        } else {
            //ȡ������ʱ��datetime�����id
            for (iter=m_dateTimeToIds.begin(); iter!=m_dateTimeToIds.end(); iter++) {
                if (iter.key() > datetime) {
                    break;
                }
                if (iter.value() > id) {
                    id = iter.value();
                }
            }
        }
        //����Ϣ���뵽ָ��λ��
        newId = insertElement(map, id);
    }
    if (newId != -1) {
        //ά��������������Ϣ����
        insertIdToMap<QString>(newId, m_phoneNumToIds, phonenum, datetime);        
        insertIdToMap<int>(newId, m_boxToIds, box, datetime);        
        m_dateTimeToIds.insertMulti(datetime, newId);        
        m_totalNum++;
        m_boxesNum[box]++;
        if (state == Message::State_Unread) {
            switch (box) {
            case Message::Box_Inbox:
                m_unreadInboxNum++;
                break;
            case Message::Box_Dustbin:
                m_unreadDustbinNum++;
                break;
            default:
                qDebug() << "this is a bug" << __FILE__ << __LINE__;
                break;
            }
        }
        if (m_isAutoSave) {
            m_changeTimes++;
            if (m_changeTimes > cons_reserve_change_times) {
                emit signSave();
            }
            m_timerIdle.start();
        }
        //���ͱ���ź�
        emit signMsgChanged(newId, ChangeEvent_Add, box);
    } else {
        qDebug() << "add msg failed" << newId << __FILE__ << __LINE__;
    }
    return newId;
}

/************************************************
���ܣ�
    �����Ϣ�����͡����䡢״̬�Ƿ���Ч
������
    msgtype: ��Ϣ����
    box: ����
    state: ״̬
����ֵ:
    ��Ч�򷵻�true, ���򷵻�false
************************************************/
bool AbstractMsg::checkValidity(Message::MsgType msgtype, 
                           Message::Box box, 
                           Message::State state)
{
    bool flag = false;
    if (msgtype == Message::MsgType_InMsg) {
        if ((box==Message::Box_Inbox || box==Message::Box_Dustbin) 
                && (state==Message::State_Unread || state==Message::State_Read)) {
            flag = true;
        }
    } else if (msgtype == Message::MsgType_OutMsg) {
        if ((box==Message::Box_Outbox || box==Message::Box_Draftbox || box==Message::Box_Dustbin)
                && (state==Message::State_Unsend || state==Message::State_Sent || state==Message::State_SendFail)) {
            flag = true;
        }
    }
    return flag;
}

/************************************************
���ܣ�
    �Ƴ���Ϣ
    �Ƴ�������������Ѷ���δ�����������ͱ���ź�
������
    id: ��Ϣ��id
����ֵ:
    �Ƴ��ɹ�����true, ����false
************************************************/
bool AbstractMsg::removeMessage(int id)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
//        QMessageBox::information(NULL, tr("msg"), tr("AbstractMsg is not initialized"));
        m_msg.setText(tr("AbstractMsg is not initialized"));
        m_msg.show();
        return false;
    }
    //�Ȼ�ȡ��Ϣ��ɾ������ά������
    Message msg = getMessageOfIdFromInner(id);
    bool flag = removeElement(id);
    if (flag) {
        m_phoneNumToIds.remove(msg.phonenum, id);
        m_boxToIds.remove(msg.box, id);
        m_dateTimeToIds.remove(msg.datetime, id);
        m_totalNum--;
        m_boxesNum[msg.box]--;
        if (msg.state == Message::State_Unread) {
            switch (msg.box) {
            case Message::Box_Inbox:
                m_unreadInboxNum--;
                break;
            case Message::Box_Dustbin:
                m_unreadDustbinNum--;
                break;
            default:
                qDebug() << "this is a bug" << __FILE__ << __LINE__;
                break;
            }
        }
        if (m_isAutoSave) {
            m_changeTimes++;
            if (m_changeTimes > cons_reserve_change_times) {
                emit signSave();
            }
            m_timerIdle.start();
        }
        //���ͱ���ź�
        emit signMsgChanged(id, ChangeEvent_Remove, msg.box);
    } else {
        qDebug() << "remove msg failed" << __FILE__ << __LINE__;
    }
    return flag;
}

/************************************************
���ܣ�
    �ı���Ϣ���ͣ��ռ���������䣩
    �޸ĺ�����������Ѷ���δ�����������ͱ���ź�
������
    id: ��Ϣid
    dstBox: Ŀ������
����ֵ:
    �޸ĳɹ�����true, ����false
************************************************/
bool AbstractMsg::changeBox(int id, Message::Box dstBox)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
//        QMessageBox::information(NULL, tr("msg"), tr("AbstractMsg is not initialized"));
        m_msg.setText(tr("AbstractMsg is not initialized"));
        m_msg.show();
        return false;
    }
    Message msg = getMessageOfIdFromInner(id);
    int srcBox = msg.box;
    if (srcBox == dstBox) {
        qDebug() << "dstBox is same with srcBox" << __FILE__ << __LINE__;
        return false;
    }
    QMap<QString, QString> map;
    map.insert(m_ltTags.at(LTT_Box), QString::number(dstBox));
    bool flag = modifyElement(msg.id, map);
    if (flag) {       
        //����box-id����
        m_boxToIds.remove(srcBox, msg.id);
        insertIdToMap<int>(msg.id, m_boxToIds, dstBox, msg.datetime);
        m_boxesNum[srcBox]--;
        m_boxesNum[dstBox]++;
        if (msg.state == Message::State_Unread) {
            switch (dstBox) {
            case Message::Box_Inbox:
                m_unreadInboxNum++;
                m_unreadDustbinNum--;
                break;
            case Message::Box_Dustbin:
                m_unreadInboxNum--;
                m_unreadDustbinNum++;
                break;
            default:
                qDebug() << "this is a bug" << __FILE__ << __LINE__;
                break;
            }
        }
        if (m_isAutoSave) {
            m_changeTimes++;
            if (m_changeTimes > cons_reserve_change_times) {
                emit signSave();
            }
            m_timerIdle.start();
        }
        //���ͱ���ź�
        emit signMsgChanged(msg.id, ChangeEvent_ChangeBox, srcBox);
    } else {
        qDebug() << "changeBox failed" << __FILE__ << __LINE__;
    }
    return flag;
}

/************************************************
���ܣ�
    �޸���Ϣ״̬
    �ݲ�ʹ��
������
    id: ��Ϣid
����ֵ:
    ���óɹ�����true, ����false
************************************************/
bool AbstractMsg::changeState(int id, Message::State dstState)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
//        QMessageBox::information(NULL, tr("msg"), tr("AbstractMsg is not initialized"));
        m_msg.setText(tr("AbstractMsg is not initialized"));
        m_msg.show();
        return false;
    }
    Message msg = getMessageOfIdFromInner(id);
    if (msg.msgtype == Message::MsgType_InMsg) {
        qDebug() << "In message don't need to change state. Please use setRead instead." << __FILE__ << __LINE__;
        return false;
    }
    if (dstState != Message::State_Sent) {
        qDebug() << "dstState should be Sent" << __FILE__ << __LINE__;
        return false;
    }
    int srcState = msg.state;
    if (srcState == dstState) {
        qDebug() << "srcState is same with dstState" << __FILE__ << __LINE__;
        return true;
    }
    QMap<QString, QString> map;
    map.insert(m_ltTags.at(LTT_State), QString::number(dstState));
    bool flag = modifyElement(msg.id, map);
    if (flag) {
        if (m_isAutoSave) {
            m_changeTimes++;
            if (m_changeTimes > cons_reserve_change_times) {
                emit signSave();
            }
            m_timerIdle.start();        
        }
        //���ͱ���ź�
        emit signMsgChanged(msg.id, ChangeEvent_ChangeState, srcState);
    } else {
        qDebug() << "changeState failed" << __FILE__ << __LINE__;
    }
    return flag;    
}

/************************************************
���ܣ�
    ������Ϣ�Ѷ�
������
    id: ��Ϣid
����ֵ:
    ���óɹ�����true, ����false
************************************************/
bool AbstractMsg::setRead(int id, bool isRead)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
//        QMessageBox::information(NULL, tr("msg"), tr("AbstractMsg is not initialized"));
        m_msg.setText(tr("AbstractMsg is not initialized"));
        m_msg.show();
        return false;
    }
    QMap<QString, QString> map;
    Message msg = getMessageOfIdFromInner(id);
    if (msg.msgtype == Message::MsgType_OutMsg) {
        qDebug() << "Can't set out messsage read" << msg.id << __FILE__ << __LINE__;
        return false;
    }
    if (msg.state == (isRead ? Message::State_Read : Message::State_Unread)) {
        qDebug() << "Read state doesn't change" << __FILE__ << __LINE__;
        return true;
    }
    map.insert(m_ltTags.at(LTT_State), QString::number(isRead ? Message::State_Read : Message::State_Unread));
    bool flag = modifyElement(msg.id, map);
    if (flag) {
        if (isRead && msg.state == Message::State_Unread) {
            if (msg.box == Message::Box_Inbox) {
                m_unreadInboxNum--;                
            } else {
                m_unreadDustbinNum--;                
            }
        } else if (!isRead && msg.state == Message::State_Read) {
            if (msg.box == Message::Box_Inbox) {
                m_unreadInboxNum++;              
            } else {
                m_unreadDustbinNum++;                
            }
        }       
        if (m_isAutoSave) {
            m_changeTimes++;
            if (m_changeTimes > cons_reserve_change_times) {
                emit signSave();
            }
            m_timerIdle.start();
        }
        //���ͱ���ź�
        emit signMsgChanged(msg.id, ChangeEvent_ChangeState, msg.state);
    } else {
        qDebug() << "setRead failed" << __FILE__ << __LINE__;
    }
    return true;
}

/************************************************
���ܣ�
    ����
������
    ��
����ֵ:
    ����ɹ�����true, ����false
************************************************/
bool AbstractMsg::save()
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
//        QMessageBox::information(NULL, tr("msg"), tr("AbstractMsg is not initialized"));
        m_msg.setText(tr("AbstractMsg is not initialized"));
        m_msg.show();
        return false;
    }
    if (m_isAutoSave) {
        if (m_changeTimes == 0) {
            //        qDebug() << "no change" << __FILE__ << __LINE__;
            return true;
        }
        m_timerIdle.start();
        m_changeTimes = 0;
    }
    qDebug() << "save" << __FILE__ << __LINE__;
    return CommonXmlProcess::save();    
}

/************************************************
���ܣ�
    ���������������Ѷ���δ��������Ϣ
������
    ��
����ֵ:
    ��
************************************************/
void AbstractMsg::createMap()
{
    QList<int> iIndex = idIndex();
    QList<Message> invalidMsgs;
    int count = iIndex.count();
    for (int i=0; i<count; i++) {
        int id = iIndex.at(i);
        Message msg = getMessageOfIdFromInner(id);    
        if (!msg.isValid()) {
            invalidMsgs << msg;
            m_isFileNormal = false;
        }
        m_phoneNumToIds.insertMulti(msg.phonenum, id);
        m_boxToIds.insertMulti(msg.box, id);
        m_dateTimeToIds.insertMulti(msg.datetime, id);
        m_totalNum++;
        m_boxesNum[msg.box]++;
        if (msg.state == Message::State_Unread) {
            switch (msg.box) {
            case Message::Box_Inbox:
                m_unreadInboxNum++;
                break;
            case Message::Box_Dustbin:
                m_unreadDustbinNum++;
                break;
            default:
                qDebug() << "this is a bug" << __FILE__ << __LINE__;
                break;
            }
        }   
    } 
    
    if (!invalidMsgs.isEmpty()) {
        QString strErrorDetail;
        foreach (Message msg, invalidMsgs) {
            QString strMsgtype;
            if (msg.msgtype == Message::MsgType_InMsg) {
                strMsgtype = tr("In message");
            } else if (msg.msgtype == Message::MsgType_OutMsg) {
                strMsgtype = tr("Out message");
            } else {
                strMsgtype = tr("Invalid message type");
            }
            strErrorDetail += tr("PhoneNum: %1\nDateTime: %2\nContent: %3\nMessageType: %4 %5\nBox: %6 %7\nState: %8 %9\n\n")
                    .arg(msg.phonenum)
                    .arg(msg.datetime.toString("yyyy-MM-dd hh:mm:ss.zzz"))
                    .arg(msg.content)
                    .arg(msg.msgtype)
                    .arg(strMsgtype)
                    .arg(msg.box)
                    .arg(Message::msgBoxToTr((Message::Box)msg.box))
                    .arg(msg.state)
                    .arg(Message::msgStateToTr((Message::State)msg.state));
        }
        QFile file(QApplication::applicationDirPath() + "/MsgError.log");
        if (file.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
            QTextStream ts(&file);
            QFileInfo fileInfo(file.fileName());
            if (fileInfo.size() == 0) {
                ts << tr("MsgError\n\n");
            }
            ts << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n";
            ts << strErrorDetail + "\n";
            file.close();
        }

        QString strError = tr("There are total %1 error messages. See \"MsgError.log\" for more detail.")
                .arg(invalidMsgs.count());
        m_msg.setText(strError);
        m_msg.show();
    }
}

/************************************************
���ܣ�
    �޸�
������
    ��
����ֵ:
    �޸��ɹ�����true, ���򷵻�false
************************************************/
bool AbstractMsg::repair()
{
    m_lock.lockForWrite();
    m_isRepair = true;
    m_lock.unlock();
    
    QList<int> iIndex = idIndex();    
    int count = iIndex.count();
    //��������״̬��ʱ�䲻�Ե���Ϣ
    for (int i=0; i<count; i++) {
        int id = iIndex.at(i);
        Message msg = getMessageOfIdFromInner(id);
        if (!msg.isValid()) {
            if (!msg.isMsgTypeValid()) {
                qDebug() << "MsgType invalid" << msg.content << __FILE__ << __LINE__;
                msg.msgtype = Message::MsgType_InMsg;
            }
            if (!msg.isStateValid()) {
                qDebug() << "State invalid" << msg.content << __FILE__ << __LINE__;
                if (msg.msgtype == Message::MsgType_InMsg) {
                    msg.state = Message::State_Unread;
                } else {
                    msg.state = Message::State_SendFail;
                }
            }
            if (msg.datetime.isNull() || !msg.datetime.isValid()) {
                msg.datetime = QDateTime::currentDateTime();
            }
            qDebug() << msg.content << "invalid" << __FILE__ << __LINE__;
            if (!removeMessage(msg.id)) {
                qDebug() << "remove msg fail" << msg.id << msg.content << __FILE__ << __LINE__;
            }
            if (-1 == addMessage(msg.phonenum, 
                                 msg.datetime, 
                                 msg.content, 
                                 (Message::MsgType)msg.msgtype, 
                                 Message::Box_Dustbin, 
                                 (Message::State)msg.state)) {
                qDebug() << "add msg fail" << msg.id << msg.content << __FILE__ << __LINE__;
            }
        }
    }

    //���»�ȡ����
    m_boxesNum.fill(0);
    m_unreadInboxNum = 0;
    m_unreadDustbinNum = 0;
    m_totalNum = 0;
    m_changeTimes = 0;
    m_phoneNumToIds.clear();
    m_dateTimeToIds.clear();
    m_boxToIds.clear();
    createMap();
    
    //����λ�ò��Ե���Ϣ    
    Message prevMsg;
    QList<Message> invalidMsgs;
    iIndex = idIndex();
    count = iIndex.count();
    qDebug() << count << iIndex << __FILE__ << __LINE__;
    for (int i=0; i<count; i++) {
        int id = iIndex.at(i);
        Message msg = getMessageOfIdFromInner(id);
        if (!msg.isValid()) {
            qDebug() << "this is a bug" << __FILE__ << __LINE__;
            return false;
        }
        if (!prevMsg.isEmpty()) {
            if (prevMsg.datetime > msg.datetime) {
                invalidMsgs << msg;
                removeMessage(msg.id);
            } else {
                prevMsg = msg;
            }
        } else {
            prevMsg = msg;
        }
    }
    foreach (Message msg, invalidMsgs) {
        addMessage(msg.phonenum, 
                   msg.datetime, 
                   msg.content, 
                   (Message::MsgType)msg.msgtype, 
                   (Message::Box)msg.box, 
                   (Message::State)msg.state);
    }
    m_changeTimes = 1;
    save();

    return true;
}

/************************************************
���ܣ�
    ��ѯָ��id����Ϣ�������ⲿ����
������
    id: ��Ϣid
����ֵ:
    ���ز�ѯ���
************************************************/
Message AbstractMsg::getMessageOfId(int id)
{
    QReadLocker readLocker(&m_lock);
    return getMessageOfIdFromInner(id);
}

/************************************************
���ܣ�
    ��ѯָ��id����Ϣ�������ڲ�����
������
    id: ��Ϣid
����ֵ:
    ���ز�ѯ���
************************************************/
Message AbstractMsg::getMessageOfIdFromInner(int id)
{
    Message msg;
    QMap<QString, QString> values = getValues(id);
    if (values.isEmpty()) {
        qDebug() << "values is empty" << id << __FILE__ << __LINE__;
    } else {
        msg.id = id;
        msg.phonenum = values.value(m_ltTags.at(LTT_PhoneNum));
        msg.datetime = QDateTime::fromString(
                    values.value(m_ltTags.at(LTT_DateTime)), "yyyy-MM-dd hh:mm:ss.zzz");
        msg.content = values.value(m_ltTags.at(LTT_Content));
        msg.msgtype = values.value(m_ltTags.at(LTT_MsgType)).toInt();
        msg.box = values.value(m_ltTags.at(LTT_Box)).toInt();
        msg.state = values.value(m_ltTags.at(LTT_State)).toInt();
    }
    return msg;
}

/************************************************
���ܣ�
    ��ѯ������Ϣ
������
    ��
����ֵ:
    ���ز�ѯ���
************************************************/
QList<Message> AbstractMsg::getAllMessages()
{
    QReadLocker readLocker(&m_lock);
    QList<int> iIndex = idIndex();
    QList<Message> msgs;
    foreach (int id, iIndex) {
        Message msg = getMessageOfIdFromInner(id);
        if (msg.isEmpty()) {
            qDebug() << "msg is empty" << msg.id << msg.content << __FILE__ << __LINE__;
            continue;
        }
        msgs.append(msg);
    }
    return msgs;
}

/************************************************
���ܣ�
    ��ѯ�����Ϣ
    ע���ú�����������
������
    n: ��ѯ��Ŀ
����ֵ:
    ���ز�ѯ���
************************************************/
QList<Message> AbstractMsg::getLastMessages(int n)
{
    QReadLocker readLocker(&m_lock);  
    QList<Message> msgs;
    QList<int> iIndex = idIndex();
    QList<int>::iterator iter=iIndex.begin();
    int count = iIndex.count();
    int i = (n<0 || n>count) ? 0 : count-n;
    while (i-- > 0) {
        iter++;
    }
    for (; iter!=iIndex.end(); iter++) {
        Message msg = getMessageOfIdFromInner(*iter);
        if (msg.isEmpty()) {
            qDebug() << "msg is empty" << msg.id << msg.content << __FILE__ << __LINE__;
            continue;
        }
        msgs.append(msg);
    }
    return msgs;        

}

/************************************************
���ܣ�
    ��ѯĳ�����͵��������Ϣ
������
    n: ��ѯ��Ŀ
    box: ����
����ֵ:
    ���ز�ѯ���
************************************************/
QList<Message> AbstractMsg::getLastMessages(int n, Message::Box box)
{    
    QReadLocker readLocker(&m_lock); 
    QList<int> ids = m_boxToIds.values(box);
    int count = ids.count();
    int pos;
    if (n<0 || n>count) {
        pos = 0;
        n = count;
    } else {
        pos = count - n;
    }
    return getMidMessages<int>(m_boxToIds, box, pos, count);
}

/************************************************
���ܣ�
    ��ѯĳ�������position��ʼ��n����Ϣ�����position����
    ����Ϣ�ķ�Χ���򷵻ؿգ����n������Ϣ����Ŀ����n����-1
    �򷵻�position��ʼ������box��Ϣ
������
    box: ����
    position: ��ʼλ��
    n: ��ѯ��Ŀ
����ֵ:
    ���ز�ѯ���
************************************************/
QList<Message> AbstractMsg::getMessagesOfBox(Message::Box box, int position, int n)
{
    QReadLocker readLocker(&m_lock); 
    return getMidMessages<int>(m_boxToIds, box, position, n);
}

/************************************************
���ܣ�
    ��ѯĳ���绰�����position��ʼ��n����Ϣ�����position
    ��������Ϣ�ķ�Χ���򷵻ؿգ����n������Ϣ����Ŀ����n����-1
    �򷵻�position��ʼ������box��Ϣ
������
    phoneNum: �绰����
    position: ��ʼλ��
    n: ��ѯ��Ŀ
����ֵ:
    ���ز�ѯ���
************************************************/
QList<Message> AbstractMsg::getMessagesOfPhoneNum(const QString &phoneNum, int position, int n)
{
    QReadLocker readLocker(&m_lock); 
    return getMidMessages<QString>(m_phoneNumToIds, phoneNum, position, n);
}

/************************************************
���ܣ�
    ��ѯĳ��ʱ��Ĵ�position��ʼ��n����Ϣ�����position
    ��������Ϣ�ķ�Χ���򷵻ؿգ����n������Ϣ����Ŀ����n����-1
    �򷵻�position��ʼ������box��Ϣ
������
    dateTime: ʱ��
    position: ��ʼλ��
    n: ��ѯ��Ŀ
����ֵ:
    ���ز�ѯ���
************************************************/
QList<Message> AbstractMsg::getMessagesOfDateTime(const QDateTime &dateTime)
{
    QReadLocker readLocker(&m_lock);
    return getMidMessages<QDateTime>(m_dateTimeToIds, dateTime, 0, -1);
}

/************************************************
���ܣ�
    ��id���������ĺ���λ�ã�ʹ�䰴ʱ��˳�����У��Ա��ѯ��
    ά����������: m_phoneNumToIds, m_boxToIds
������
    id: ��Ϣid
    map: ����
    key: �����ؼ���
    datetime: id��Ӧ��ʱ��
����ֵ:
    ��
************************************************/
template<typename T> void AbstractMsg::insertIdToMap(int id, 
                                                QMultiMap<T, int> &map,                                                
                                                const T &key, 
                                                const QDateTime &datetime)
{
    QList<int> ids = map.values(key);
    if (ids.isEmpty() || datetime>=getMessageOfIdFromInner(ids.at(0)).datetime) {
        map.insert(key, id);
    } else { //���ǵ���ʱ��������п���ʱ���Ŀǰ�����ʱ��С
        QList<int>::iterator iter=ids.begin();
        for (; iter!=ids.end(); iter++) {
            if (datetime >= getMessageOfIdFromInner(*iter).datetime) {
                ids.insert(iter, id);
                break;
            }
        }
        //�п��ܱ�����ʱ�䶼С
        if (iter==ids.end()) {
            ids.insert(iter, id);
        }
        map.remove(key);
        for (QList<int>::iterator iter=(--ids.end()); iter>=ids.begin(); iter--) {
            map.insertMulti(key, *iter);            
        }
    }
}

/************************************************
���ܣ�
    ��ѯĳ�������ĳ���绰�����position��ʼ��n����Ϣ����
    ��position�����˷�Χ���򷵻ؿգ����n������Ϣ��Ŀ����
    n����-1�򷵻�position��ʼ��������Ϣ
������
    map: ��ѯ����������������-id���������-id����
    key: �����ؼ���
    box: ����
    position: ��ʼλ��
    n: ��ѯ��Ŀ
����ֵ:
    ���ز�ѯ���
************************************************/
template<typename T> QList<Message> AbstractMsg::getMidMessages(const QMultiMap<T, int> &map,
                                                           const T &key,
                                                           int position,
                                                           int n=-1)
{
    //values��õ�id������ģ����Ժ����index�ǴӺ���ǰ����
    QList<int> ids = map.values(key);
    int count = ids.count();
    QList<Message> msgs;
    if (position>=0 && position<count) {
        int index = count - 1 - position;
        if (n == -1) {
            n = count;
        }
        for (int i=0; i<n && index>=0; i++, index--) {
            msgs.append(getMessageOfIdFromInner(ids.at(index)));
        }
    }
    return msgs;    
}
