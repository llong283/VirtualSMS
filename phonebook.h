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
    //����¼�
    enum ChangeEvent {
        ChangeEvent_Add, //�����ϵ��
        ChangeEvent_Remove, //ɾ����ϵ��
//        ChangeEvent_Modify, //�޸���ϵ����Ϣ
        ChangeEvent_ChangeName,
        ChangeEvent_ChangeType, //�޸���ϵ������
        ChangeEvent_ChangePhoneNum, //�޸���ϵ�˵绰����
        ChangeEvent_ChangeState //״̬���
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
        int state; //״̬
        QHostAddress ipaddr; //ip��ַ
        QDateTime lastActiveTime; //�ϴλ�Ծʱ��
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
