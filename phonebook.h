#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include "config.h"
#include "commonxmlprocess.h"

#define g_phoneBook PhoneBook::instance()

class PhoneBook : public CommonXmlProcess
{
    Q_OBJECT
public:
    static PhoneBook *instance();
    //变更事件
    enum ChangeEvent {
        ChangeEvent_Add, //添加联系人
        ChangeEvent_Remove, //删除联系人
//        ChangeEvent_Modify, //修改联系人信息
        ChangeEvent_ChangeName,
        ChangeEvent_ChangeType, //修改联系人类型
        ChangeEvent_ChangePhoneNum, //修改联系人电话号码
        ChangeEvent_ChangeState //状态变更
    };
    
    bool init();
    int addContact(const QString &name, 
                   const QString &phonenum, 
                   const Contact::Type &type=Contact::Type_Other);
    bool removeContact(const int &id);
    bool modifyContact(const int &id, const QString &name, const QString &phonenum, const Contact::Type &type);
    bool changeType(const int &id, const Contact::Type &dstType);
    bool changePhoneNum(const int &id, const QString &dstPhoneNum);
    bool changeState(int id, Contact::State dstState, const QHostAddress &ipAddr, const QDateTime &activeTime);
//    bool save();
    void setConflict(bool isConflict);
    bool isConflict();
    Contact getContactOfId(const int &id);
    QList<Contact> getAllContacts();
    Contact getContactOfPhoneNum(const QString &phonenum);
    QList<Contact> getContactsOfType(const Contact::Type &type);
    Contact getMyself();
    QHostAddress getContactIpAddr(int id);
    
public slots:
    bool save();
        
private:
    PhoneBook(QObject *parent = 0);
    static PhoneBook *phoneBook;
    bool m_isInit;
    bool m_isConflict;
    QReadWriteLock m_lock;
    QTimer m_timerUpdateState;
    QTimer m_timerIdle;
    QMessageBox m_msg;
    int m_changeTimes;
    
    enum _LevelTwoTag {
        LTT_Name,
        LTT_PhoneNum,
        LTT_Type
    };    
    struct ContactState {
        int state; //状态
        QHostAddress ipaddr; //ip地址
        QDateTime lastActiveTime; //上次活跃时间
    };
    
    QVector<QString> m_ltTags;
   
    QMultiMap<QString, int> m_phoneNumToIds;
    QMultiMap<int, int> m_typeToIds;
    QMap<int, ContactState> m_idToContactState;
    
    void createMap();
    Contact getContactOfIdFromInner(const int &id);
    
signals:
    void signContactChanged(int, int);
    void signSave();
    
private slots:
    void slotUpdateContactState();
};

#endif // PHONEBOOK_H
