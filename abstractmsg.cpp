/************************************************
类名: AbstractMsg
功能: 管理信息，包括收件箱、发件箱、草稿箱、垃圾箱
说明: 该类继承CommonXmlProcess类，将文件名、根节点、第一
     级标签、第二级标签传给基类进行初始化；并将基类通用的操
     作具体化，提供添加信息、删除信息、更改信箱、设置信息已
     读（对接收的信息）功能，同时提供查询功能，包括根据id查
     询、根据电话号码查询、根据类型查询、查询信箱信息数量，
     为了实现快速查询，AbstractMsg维护了电话号码-id、时间-id、
     信箱-id的索引，并同步更新各信箱信息数量。
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
功能：
    初始化
参数：
    无
返回值:
    初始化成功返回true, 否则返回false
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
功能：
    添加信息
    添加后更新索引和各信箱信息数量，并发送变更的信号
参数：
    phonenum: 电话号码
    datetime: 时间
    content: 内容
    msgtype: 信息类型
    box: 信箱
    state: 状态
返回值:
    添加成功返回新id, 否则返回-1
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
    //检查参数完整性
/*    if (phonenum.isEmpty()) { //电话号码有可能为空，保存未发送信息时
        qDebug() << "phonenum is not empty. add msg failed" << __FILE__ << __LINE__;
        return -1;        
    } else */if (datetime.isNull() || !datetime.isValid()) {
        qDebug() << "datetime is not valid. add msg failed" << __FILE__ << __LINE__;
        return -1;        
    } else if (!checkValidity(msgtype, box, state)) {
        qDebug() << "The msg type or box or state is not right. add msg failed" << __FILE__ << __LINE__;
        return -1;
    }
    
    //将信息各项内容打包成map传给基类作进一步处理
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
        //将消息插入到最前
        newId = insertElement(map, -1);
    } else {
        //从时间-id的索引中查询id，使得信息插入后仍然按时间排序，如果存有相同时间的信息，
        //则插入到相同信息的最后
        int id = 0;
        QMultiMap<QDateTime, int>::iterator iter = --m_dateTimeToIds.end();
        if (datetime >= iter.key()) {
            id = iter.value();
        } else {
            //取不大于时间datetime的最大id
            for (iter=m_dateTimeToIds.begin(); iter!=m_dateTimeToIds.end(); iter++) {
                if (iter.key() > datetime) {
                    break;
                }
                if (iter.value() > id) {
                    id = iter.value();
                }
            }
        }
        //将消息插入到指定位置
        newId = insertElement(map, id);
    }
    if (newId != -1) {
        //维护索引，更新信息数量
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
        //发送变更信号
        emit signMsgChanged(newId, ChangeEvent_Add, box);
    } else {
        qDebug() << "add msg failed" << newId << __FILE__ << __LINE__;
    }
    return newId;
}

/************************************************
功能：
    检查信息的类型、信箱、状态是否有效
参数：
    msgtype: 信息类型
    box: 信箱
    state: 状态
返回值:
    有效则返回true, 否则返回false
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
功能：
    移除信息
    移除后更新索引和已读、未读数量，发送变更信号
参数：
    id: 信息的id
返回值:
    移除成功返回true, 否则false
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
    //先获取信息再删除，以维护索引
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
        //发送变更信号
        emit signMsgChanged(id, ChangeEvent_Remove, msg.box);
    } else {
        qDebug() << "remove msg failed" << __FILE__ << __LINE__;
    }
    return flag;
}

/************************************************
功能：
    改变信息类型（收件箱或垃圾箱）
    修改后更新索引和已读、未读数量，发送变更信号
参数：
    id: 信息id
    dstBox: 目标类型
返回值:
    修改成功返回true, 否则false
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
        //更新box-id索引
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
        //发送变更信号
        emit signMsgChanged(msg.id, ChangeEvent_ChangeBox, srcBox);
    } else {
        qDebug() << "changeBox failed" << __FILE__ << __LINE__;
    }
    return flag;
}

/************************************************
功能：
    修改信息状态
    暂不使用
参数：
    id: 信息id
返回值:
    设置成功返回true, 否则false
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
        //发送变更信号
        emit signMsgChanged(msg.id, ChangeEvent_ChangeState, srcState);
    } else {
        qDebug() << "changeState failed" << __FILE__ << __LINE__;
    }
    return flag;    
}

/************************************************
功能：
    设置信息已读
参数：
    id: 信息id
返回值:
    设置成功返回true, 否则false
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
        //发送变更信号
        emit signMsgChanged(msg.id, ChangeEvent_ChangeState, msg.state);
    } else {
        qDebug() << "setRead failed" << __FILE__ << __LINE__;
    }
    return true;
}

/************************************************
功能：
    保存
参数：
    无
返回值:
    保存成功返回true, 否则false
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
功能：
    创建索引，更新已读、未读数量信息
参数：
    无
返回值:
    无
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
功能：
    修复
参数：
    无
返回值:
    修复成功返回true, 否则返回false
************************************************/
bool AbstractMsg::repair()
{
    m_lock.lockForWrite();
    m_isRepair = true;
    m_lock.unlock();
    
    QList<int> iIndex = idIndex();    
    int count = iIndex.count();
    //修正类型状态、时间不对的信息
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

    //重新获取索引
    m_boxesNum.fill(0);
    m_unreadInboxNum = 0;
    m_unreadDustbinNum = 0;
    m_totalNum = 0;
    m_changeTimes = 0;
    m_phoneNumToIds.clear();
    m_dateTimeToIds.clear();
    m_boxToIds.clear();
    createMap();
    
    //修正位置不对的信息    
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
功能：
    查询指定id的信息，用于外部调用
参数：
    id: 信息id
返回值:
    返回查询结果
************************************************/
Message AbstractMsg::getMessageOfId(int id)
{
    QReadLocker readLocker(&m_lock);
    return getMessageOfIdFromInner(id);
}

/************************************************
功能：
    查询指定id的信息，用于内部调用
参数：
    id: 信息id
返回值:
    返回查询结果
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
功能：
    查询所有信息
参数：
    无
返回值:
    返回查询结果
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
功能：
    查询最近信息
    注：该函数将被弃用
参数：
    n: 查询数目
返回值:
    返回查询结果
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
功能：
    查询某个类型的最近的信息
参数：
    n: 查询数目
    box: 信箱
返回值:
    返回查询结果
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
功能：
    查询某个信箱从position开始的n条信息，如果position超过
    了信息的范围，则返回空；如果n大于信息的数目或者n等于-1
    则返回position开始的所有box信息
参数：
    box: 信箱
    position: 起始位置
    n: 查询数目
返回值:
    返回查询结果
************************************************/
QList<Message> AbstractMsg::getMessagesOfBox(Message::Box box, int position, int n)
{
    QReadLocker readLocker(&m_lock); 
    return getMidMessages<int>(m_boxToIds, box, position, n);
}

/************************************************
功能：
    查询某个电话号码从position开始的n条信息，如果position
    超过了信息的范围，则返回空；如果n大于信息的数目或者n等于-1
    则返回position开始的所有box信息
参数：
    phoneNum: 电话号码
    position: 起始位置
    n: 查询数目
返回值:
    返回查询结果
************************************************/
QList<Message> AbstractMsg::getMessagesOfPhoneNum(const QString &phoneNum, int position, int n)
{
    QReadLocker readLocker(&m_lock); 
    return getMidMessages<QString>(m_phoneNumToIds, phoneNum, position, n);
}

/************************************************
功能：
    查询某个时间的从position开始的n条信息，如果position
    超过了信息的范围，则返回空；如果n大于信息的数目或者n等于-1
    则返回position开始的所有box信息
参数：
    dateTime: 时间
    position: 起始位置
    n: 查询数目
返回值:
    返回查询结果
************************************************/
QList<Message> AbstractMsg::getMessagesOfDateTime(const QDateTime &dateTime)
{
    QReadLocker readLocker(&m_lock);
    return getMidMessages<QDateTime>(m_dateTimeToIds, dateTime, 0, -1);
}

/************************************************
功能：
    将id插入索引的合适位置，使其按时间顺序排列，以便查询，
    维护的索引是: m_phoneNumToIds, m_boxToIds
参数：
    id: 信息id
    map: 索引
    key: 索引关键字
    datetime: id对应的时间
返回值:
    无
************************************************/
template<typename T> void AbstractMsg::insertIdToMap(int id, 
                                                QMultiMap<T, int> &map,                                                
                                                const T &key, 
                                                const QDateTime &datetime)
{
    QList<int> ids = map.values(key);
    if (ids.isEmpty() || datetime>=getMessageOfIdFromInner(ids.at(0)).datetime) {
        map.insert(key, id);
    } else { //考虑导入时的情况，有可能时间比目前的最大时间小
        QList<int>::iterator iter=ids.begin();
        for (; iter!=ids.end(); iter++) {
            if (datetime >= getMessageOfIdFromInner(*iter).datetime) {
                ids.insert(iter, id);
                break;
            }
        }
        //有可能比所有时间都小
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
功能：
    查询某个信箱或某个电话号码从position开始的n条信息，如
    果position超过了范围，则返回空；如果n大于信息数目或者
    n等于-1则返回position开始的所有信息
参数：
    map: 查询的索引，包括信箱-id索引或号码-id索引
    key: 索引关键字
    box: 信箱
    position: 起始位置
    n: 查询数目
返回值:
    返回查询结果
************************************************/
template<typename T> QList<Message> AbstractMsg::getMidMessages(const QMultiMap<T, int> &map,
                                                           const T &key,
                                                           int position,
                                                           int n=-1)
{
    //values获得的id是逆序的，所以后面的index是从后往前遍历
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
