/************************************************
类名: PhoneBook
功能: 管理电话本
说明: 该类继承CommonXmlProcess类，将文件名、根节点、第一
     级标签、第二级标签传给基类进行初始化；并将基类通用的操
     作具体化，提供添加联系人、删除联系人、更改联系人类型功
     能，同时提供查询功能，包括根据id查询、根据电话号码查询、
     根据类型查询，为了实现快速查询，PhoneBook维护了id-电
     话号码、类型-id的索引。
************************************************/

#include "phonebook.h"
#include "udpbroadcast.h"

PhoneBook *PhoneBook::phoneBook = NULL;
const int cons_reserve_change_times = 50;

PhoneBook::PhoneBook(QObject *parent) :
    CommonXmlProcess(parent),
    m_isConflict(false),
    m_changeTimes(0)
{
}

/************************************************
功能：
    PhoneBook单例初始化，属静态函数
参数：
    无
返回值:
    返回PhoneBook单例指针
************************************************/
PhoneBook *PhoneBook::instance()
{
    if (phoneBook == NULL) {
        phoneBook = new PhoneBook;
    }
    return phoneBook;
}

bool PhoneBook::init()
{
    m_msg.setWindowTitle(tr("msg"));
    QString fileName(QApplication::applicationDirPath() + "/../Test/phonebook.xml");
    m_ltTags << "Name" << "PhoneNum" << "Type";    
    m_isInit = initCommon(fileName, "PhoneBook", "Contact", m_ltTags);
    if (!m_isInit) {
//        QMessageBox::information(NULL, tr("msg"), tr("PhoneBook init failed. PhoneBook will not work."));
        m_msg.setText(tr("PhoneBook init failed. PhoneBook will not work."));
        m_msg.show();
        return false;        
    }
    createMap();
    //删除上次保存的陌生人
    QList<int> ids = m_typeToIds.values(Contact::Type_Stranger);
    foreach (int id, ids) {
        removeContact(id);
    }
    m_timerUpdateState.setInterval(5 * 1000);
    connect(&m_timerUpdateState, SIGNAL(timeout()), this, SLOT(slotUpdateContactState()));
    m_timerUpdateState.start();
    m_timerIdle.setInterval(10 * 1000);
    connect(&m_timerIdle, SIGNAL(timeout()), this, SLOT(save()));
    m_timerIdle.start();
    connect(this, SIGNAL(signSave()), this, SLOT(save()), Qt::QueuedConnection);   
    return true;
}

/************************************************
功能：
    添加联系人
参数：
    name: 联系人姓名
    phonenum: 联系人电话号码
    type: 联系人类型
返回值:
    成功则返回新id，否则返回-1
************************************************/
int PhoneBook::addContact(const QString &name, const QString &phonenum, const Contact::Type &type)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
        m_msg.setText(tr("PhoneBook is not initialized"));
        m_msg.show();
        return -1;
    }
    if (!m_phoneNumToIds.values(phonenum).isEmpty()) {
        m_msg.setText(tr("Phonenum is existed in the phonebook. Please choose another."));
        m_msg.show();
        return -1;
    }
    QMap<QString, QString> map;
    map.insert(m_ltTags.at(LTT_Name), name);
    map.insert(m_ltTags.at(LTT_PhoneNum), phonenum);
    map.insert(m_ltTags.at(LTT_Type), QString::number(type));
    int id = appendElement(map);
    if (id != -1) {
        if (type == Contact::Type_Myself) {
            m_phoneNumToIds.insertMulti(phonenum + "#" + phonenum, id);
        } else {
            m_phoneNumToIds.insertMulti(phonenum, id);
        }
        m_typeToIds.insertMulti(type, id);
        m_changeTimes++;
        if (m_changeTimes > cons_reserve_change_times) {
            emit signSave();
        }
        m_timerIdle.start();
        emit signContactChanged(id, ChangeEvent_Add);
    }
    return id;
}

/************************************************
功能：
    删除联系人
参数：
    id: 要删除的联系人id
返回值:
    初始化成功返回true, 否则false
************************************************/
bool PhoneBook::removeContact(const int &id)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
        m_msg.setText(tr("PhoneBook is not initialized"));
        m_msg.show();
        return false;
    }
    Contact contact = getContactOfIdFromInner(id);
    bool flag = removeElement(id);
    if (flag) {
        m_phoneNumToIds.remove(contact.getCurrentPhoneNum(), id);
        m_typeToIds.remove(contact.type, id);
        m_changeTimes++;
        if (m_changeTimes > cons_reserve_change_times) {
            emit signSave();
        }
        m_timerIdle.start();
        emit signContactChanged(id, ChangeEvent_Remove);
    }
    return flag;
}

/************************************************
功能：
    修改联系人信息   
参数：
    id: 要修改的联系人id
    name: 联系人姓名
    phonenum: 联系人电话号码
    type: 联系人类型
返回值:
    修改成功返回true, 否则false
************************************************/
bool PhoneBook::modifyContact(const int &id, 
                              const QString &name, 
                              const QString &phonenum, 
                              const Contact::Type &type)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
        m_msg.setText(tr("PhoneBook is not initialized"));
        m_msg.show();
        return false;
    }
    QString number = phonenum;
    if (type == Contact::Type_Myself) {
        number = phonenum.split("#").at(Contact::MyPhoneNum_Current);
    }
    QList<int> ids = m_phoneNumToIds.values(number);
    ids.removeOne(id);
    if (ids.count() > 0) {
        m_msg.setText(tr("Phonenum is existed in the phonebook. Please choose another."));
        m_msg.show();
        if (ids.count() > 1) {
            qDebug() << "this is a bug" << __FILE__ << __LINE__;
        }
        return false;        
    }
    Contact contact = getContactOfIdFromInner(id);
    if (contact.name==name && contact.phonenum==phonenum && contact.type==type) {
        qDebug() << "Nothing to change." << __FILE__ << __LINE__;
        return true;
    }
    
    QMap<QString, QString> map;
    map.insert(m_ltTags.at(LTT_Name), name);
    map.insert(m_ltTags.at(LTT_PhoneNum), phonenum);
    map.insert(m_ltTags.at(LTT_Type), QString::number(type));
    bool flag = modifyElement(id, map);    
    if (flag) {
        if (contact.name != name) {
            emit signContactChanged(id, ChangeEvent_ChangeName);
        }
        //此处有待商榷
        if (contact.phonenum != phonenum) {
            m_phoneNumToIds.remove(contact.getCurrentPhoneNum(), id);
            if (contact.type == Contact::Type_Myself) {
                m_phoneNumToIds.insertMulti(phonenum.split("#").at(Contact::MyPhoneNum_Current), id);
            } else {
                m_phoneNumToIds.insertMulti(phonenum, id);
            }
            emit signContactChanged(id, ChangeEvent_ChangePhoneNum);
        }
        if (contact.type != type) {
            m_typeToIds.remove(contact.type, id);
            m_typeToIds.insertMulti(type, id);
            emit signContactChanged(id, ChangeEvent_ChangeType);
        }
        m_changeTimes++;
        if (m_changeTimes > cons_reserve_change_times) {
            emit signSave();
        }        
        m_timerIdle.start();
    }
    return flag;
}

/************************************************
功能：
    修改联系人类型
参数：
    id: 要修改的联系人id
    dstType: 目标类型
返回值:
    修改成功返回true, 否则false
************************************************/
bool PhoneBook::changeType(const int &id, const Contact::Type &dstType)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
        m_msg.setText(tr("PhoneBook is not initialized"));
        m_msg.show();
        return false;
    }
    Contact contact = getContactOfIdFromInner(id);
    int srcType = contact.type;
    if (srcType == dstType) {
        qDebug() << "dstType is same with srcType" << __FILE__ << __LINE__;
        return false;
    }

    QMap<QString, QString> map;
    map.insert(m_ltTags.at(LTT_Type), QString::number(dstType));
    bool flag = modifyElement(id, map);    
    if (flag) {
        //此处有待商榷
        m_typeToIds.remove(contact.type, id);
        m_typeToIds.insertMulti(dstType, id);
        m_changeTimes++;
        if (m_changeTimes > cons_reserve_change_times) {
            emit signSave();
        }
        m_timerIdle.start();
        emit signContactChanged(id, ChangeEvent_ChangeType);
    }
    return flag;    
}

/************************************************
功能：
    修改联系人电话号码
参数：
    id: 要修改的联系人id
    dstPhoneNum: 目标号码，如果联系人为自己目标号码格式为:
        currentPhoneNum#previousPhoneNum
返回值:
    修改成功返回true, 否则false
************************************************/
bool PhoneBook::changePhoneNum(const int &id, const QString &dstPhoneNum)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
        m_msg.setText(tr("PhoneBook is not initialized"));
        m_msg.show();
        return false;
    }
    Contact contact = getContactOfIdFromInner(id);    
    //检查原号码与目标号码是否一致
    QString srcPhoneNum = contact.phonenum;
    if (srcPhoneNum == dstPhoneNum) {
        qDebug() << "dstPhoneNum is same with srcPhoneNum" << __FILE__ << __LINE__;
        return false;
    }
    //检查号码是否唯一
    QString number = dstPhoneNum;
    if (contact.type == Contact::Type_Myself) {
        number = dstPhoneNum.split("#").at(Contact::MyPhoneNum_Current);
    }
    QList<int> ids = m_phoneNumToIds.values(number);
    ids.removeOne(id);
    if (ids.count() > 0) {
        m_msg.setText(tr("Phonenum is existed in the phonebook. Please choose another."));
        m_msg.show();
        if (ids.count() > 1) {
            qDebug() << "this is a bug" << __FILE__ << __LINE__;
        }
        return false;        
    }
    
    qDebug() << "changePhoneNum" << id << dstPhoneNum << __FILE__ << __LINE__;
    
    QMap<QString, QString> map;
    map.insert(m_ltTags.at(LTT_PhoneNum), dstPhoneNum);
    bool flag = modifyElement(id, map);    
    if (flag) {
        //此处有待商榷
        m_phoneNumToIds.remove(srcPhoneNum, id);
        if (contact.type==Contact::Type_Myself) {
            m_phoneNumToIds.insertMulti(dstPhoneNum.split("#").at(Contact::MyPhoneNum_Current), id);
        } else {
            m_phoneNumToIds.insertMulti(dstPhoneNum, id);
        }
        m_changeTimes++;
        if (m_changeTimes > cons_reserve_change_times) {
            emit signSave();
        }        
        m_timerIdle.start();
        emit signContactChanged(id, ChangeEvent_ChangePhoneNum);
    }
    return flag;    
}

/************************************************
功能：
    修改联系人状态
参数：
    id: 要修改的联系人id
    dstState: 目标状态
返回值:
    修改成功返回true, 否则false
************************************************/
bool PhoneBook::changeState(int id, 
                            Contact::State dstState, 
                            const QHostAddress &ipAddr, 
                            const QDateTime &activeTime)
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
        m_msg.setText(tr("PhoneBook is not initialized"));
        m_msg.show();
        return false;
    }
    ContactState contactState = m_idToContactState.value(id);
    if (contactState.ipaddr != ipAddr) {
        contactState.ipaddr = ipAddr;
    }
    contactState.lastActiveTime = activeTime;
    if (contactState.state != dstState) {
        contactState.state = dstState;
        emit signContactChanged(id, ChangeEvent_ChangeState);
    }
    m_idToContactState.insert(id, contactState);    
    return true;
}

/************************************************
功能：
    保存联系人
参数：
    无
返回值:
    保存成功返回true, 否则false
************************************************/
bool PhoneBook::save()
{
    QWriteLocker writeLocker(&m_lock);
    if (!m_isInit) {
        m_msg.setText(tr("PhoneBook is not initialized"));
        m_msg.show();
        return false;
    }
    if (m_changeTimes == 0) {
        return true;
    }
    qDebug() << "save" << __FILE__ << __LINE__;
    m_timerIdle.start();
    m_changeTimes = 0;
    return CommonXmlProcess::save();    
}

/************************************************
功能：
    解析电话号码，建立索引
参数:
    无
返回值:
    无
************************************************/
void PhoneBook::createMap()
{
    QList<int> iIndex = idIndex();
    QList<Contact> invalidContacts;
    QList<Contact> emptyNumContacts;
    foreach (int id, iIndex) {
        Contact contact = getContactOfIdFromInner(id);        
        if (contact.phonenum.isEmpty()) {
            qDebug() << "empty phonenum contact" << contact.name << __FILE__ << __LINE__;
            emptyNumContacts << contact;
        }
        if (!contact.isValid()) {
            qDebug() << "invalid contact" << contact.phonenum << __FILE__ << __LINE__;
            invalidContacts << contact;
        }

        ContactState contactState;        
        contactState.ipaddr = QHostAddress();
        contactState.lastActiveTime = QDateTime::currentDateTime();
        if (contact.type == Contact::Type_Myself) {
            contactState.state = Contact::State_Online;
        } else {
            contactState.state = Contact::State_Offline;
        }
        if (m_phoneNumToIds.contains(contact.getCurrentPhoneNum())) {
            qDebug() << "Repeated phonenumber. Remove it." << contact.name << contact.getCurrentPhoneNum() << __FILE__ <<  __LINE__;
            removeContact(contact.id);
            continue;
        }
        m_phoneNumToIds.insertMulti(contact.getCurrentPhoneNum(), id);
        m_typeToIds.insertMulti(contact.type, id);
        m_idToContactState.insert(contact.id, contactState);
    }
    
    foreach (Contact contact, emptyNumContacts) {
        if (invalidContacts.indexOf(contact) != -1) {
            invalidContacts.removeOne(contact);
        }
        removeContact(contact.id);
    }

    foreach (Contact contact, invalidContacts) {
        changeType(contact.id, Contact::Type_Other);
    }
    Contact myself = getMyself();
    if (myself.isValid()) {
        QStringList numList = myself.phonenum.split("#");
        QString curNum = numList.at(0);
        QString prevNum;
        if (numList.count() >= 2) {
            prevNum = numList.at(1);
        }
        if (numList.count() != 2 || curNum.isEmpty() || prevNum.isEmpty()) {
            if (curNum.isEmpty()) {
                if (prevNum.isEmpty()) {
                    removeContact(myself.id);
                    return;
                } else {
                    curNum = prevNum;
                }                
            } else {
                if (prevNum.isEmpty()) {
                    prevNum = curNum;
                }
            }
            changePhoneNum(myself.id, curNum + "#" + prevNum);
        }
    }
}

void PhoneBook::setConflict(bool isConflict)
{
    QWriteLocker writeLocker(&m_lock);
    m_isConflict = isConflict;
}

bool PhoneBook::isConflict()
{
    QReadLocker readLocker(&m_lock);
    return m_isConflict;
}

/************************************************
功能：
    查询指定id的联系人信息，用于外部调用
参数：
    无
返回值:
    返回查询结果
************************************************/
Contact PhoneBook::getContactOfId(const int &id)
{
    QReadLocker readLocker(&m_lock);
    return getContactOfIdFromInner(id);
}

/************************************************
功能：
    查询指定id的联系人信息，用于内部调用
参数：
    无
返回值:
    返回查询结果
************************************************/
Contact PhoneBook::getContactOfIdFromInner(const int &id)
{
    Contact contact;
    contact.id = id;
    QMap<QString, QString> values = getValues(id);
    contact.name = values.value(m_ltTags.at(LTT_Name));
    contact.phonenum = values.value(m_ltTags.at(LTT_PhoneNum));
    contact.type = values.value(m_ltTags.at(LTT_Type)).toInt();
    if (contact.type == Contact::Type_Myself) {
        contact.state = Contact::State_Online;
    } else {
        contact.state = m_idToContactState.value(id).state;
    }
    return contact;
}

/************************************************
功能：
    查询所有联系人信息
参数：
    无
返回值:
    返回查询结果
************************************************/
QList<Contact> PhoneBook::getAllContacts()
{
    QReadLocker readLocker(&m_lock);
    QList<int> iIndex = idIndex();
    QList<Contact> contacts;
    foreach (int id, iIndex) {
        contacts.append(getContactOfIdFromInner(id));
    }
    return contacts;
}

/************************************************
功能：
    查询某个电话号码的联系人信息
参数：
    phonenum: 电话号码
返回值:
    返回查询结果
************************************************/
Contact PhoneBook::getContactOfPhoneNum(const QString &phonenum)
{
    QReadLocker readLocker(&m_lock);
    Contact contact;
    QList<int> ids = m_phoneNumToIds.values(phonenum);
    if (!ids.isEmpty()) {
        int id = ids.at(0);
        if (ids.count() > 1) {
            qDebug() << "this is a bug. Phonenumber is not unique." << phonenum << __FILE__ << __LINE__;
        }
        contact = getContactOfId(id);
    }
    return contact;
}

/************************************************
功能：
    查询某个类型的联系人信息
参数：
    type: 联系人类型
返回值:
    返回查询结果
************************************************/
QList<Contact> PhoneBook::getContactsOfType(const Contact::Type &type)
{
    QReadLocker readLocker(&m_lock);
    QList<int> ids = m_typeToIds.values(type);
    QList<Contact> contacts;
    foreach (int id, ids) {
        contacts.append(getContactOfIdFromInner(id));
    }

    return contacts;
}

/************************************************
功能：
    查询自己信息
参数：
    无
返回值:
    返回自己
************************************************/
Contact PhoneBook::getMyself()
{
    QReadLocker readLocker(&m_lock);
    Contact contact;    
    if (!idIndex().isEmpty()) {
        QList<int> ids = m_typeToIds.values(Contact::Type_Myself);
        if (ids.isEmpty()) {
            qDebug() << "No myself information in phonebook" << __FILE__ << __LINE__;
        } else {
            if (ids.count() > 1) {
                //暂只提示
                qDebug() << "Myself is not unique" << __FILE__ << __LINE__;
            }
            contact = getContactOfIdFromInner(ids.at(0));
        }
    }
    return contact;
}

/************************************************
功能：
    定时更新联系人状态，由m_timer触发
参数：
    无
返回值:
    无
************************************************/
void PhoneBook::slotUpdateContactState()
{
    QWriteLocker writeLocker(&m_lock);
    QDateTime curTime = QDateTime::currentDateTime();
    for (QMap<int, ContactState>::iterator iter=m_idToContactState.begin();
         iter!=m_idToContactState.end(); iter++) {
        ContactState contactState = iter.value();
        if (contactState.lastActiveTime.secsTo(curTime)>=3 
                && contactState.state==Contact::State_Online) {
            contactState.state = Contact::State_Offline;
            m_idToContactState.insert(iter.key(), contactState);
            emit signContactChanged(iter.key(), ChangeEvent_ChangeState);
        }
    }
}

/************************************************
功能：
    获取联系人的ip地址，如果联系人不存在，返回空的ip地址
参数：
    id: 联系人id
返回值:
    返回联系人的ip地址
************************************************/
QHostAddress PhoneBook::getContactIpAddr(int id)
{
    QReadLocker readLocker(&m_lock);
    Contact contact = getContactOfIdFromInner(id);
    QHostAddress ipAddr;
    if (contact.isValid()) {
        ipAddr = m_idToContactState.value(contact.id).ipaddr;
    }
    return ipAddr;
}
