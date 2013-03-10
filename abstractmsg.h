#ifndef ABSTRACTMSG_H
#define ABSTRACTMSG_H

#include <QObject>
#include "commonxmlprocess.h"

class AbstractMsg : public CommonXmlProcess
{
    Q_OBJECT
public:
    AbstractMsg(QObject *parent = 0);
    //����¼�
    enum ChangeEvent {
        ChangeEvent_Add, //�����Ϣ
        ChangeEvent_Remove, //ɾ����Ϣ
        ChangeEvent_ChangeBox, //�޸���Ϣ����
        ChangeEvent_ChangeState //�޸���Ϣ״̬
    };
    
    int addMessage(const QString &phonenum, 
                   const QDateTime &datetime, 
                   const QString &content, 
                   Message::MsgType msgtype,
                   Message::Box box,
                   Message::State state);
    bool removeMessage(int id);
    bool changeBox(int id, Message::Box dstBox);
    bool changeState(int id, Message::State dstState);
    bool setRead(int id, bool isRead);
    bool repair();
    Message getMessageOfId(int id);   
    QList<Message> getAllMessages();
    QList<Message> getLastMessages(int n);
    QList<Message> getLastMessages(int n, Message::Box box);
    QList<Message> getMessagesOfBox(Message::Box box, int position=0, int n=-1);
    QList<Message> getMessagesOfPhoneNum(const QString &phoneNum, int position=0, int n=-1);
    QList<Message> getMessagesOfDateTime(const QDateTime &dateTime);
    inline int getTotalNum() {
        QReadLocker readLocker(&m_lock);
        return m_totalNum;        
    }
    inline int getBoxNum(Message::Box box) {
        QReadLocker readLocker(&m_lock);
        return m_boxesNum[box];
    }
    inline int getUnreadInboxNum() {
        QReadLocker readLocker(&m_lock);
        return m_unreadInboxNum;        
    }
    inline int getUnreadDustbinNum() {
        QReadLocker readLocker(&m_lock);
        return m_unreadDustbinNum;        
    }
    inline int getMessageNumOfPhoneNum(const QString &phoneNum) {
        QReadLocker readLocker(&m_lock);
        return m_phoneNumToIds.values(phoneNum).count();
    }
    inline bool isRepair() {
        return m_isRepair;
    }
    inline bool isFileNormal() {
        return m_isFileNormal;
    }
    
protected:
    bool initAbstractMsg(const QString &fileName, bool isAutoSave);
    
public slots:
    bool save();
    
private:    
    bool m_isInit;
    bool m_isRepair;
    bool m_isAutoSave;
    bool m_isFileNormal;
    
    enum _LevelTwoTag{
        LTT_PhoneNum,
        LTT_DateTime,
        LTT_Content,
        LTT_MsgType,
        LTT_Box,
        LTT_State
    };
    QVector<QString> m_ltTags;
    QMultiMap<QString, int> m_phoneNumToIds; //�绰����-id����
    QMultiMap<QDateTime, int> m_dateTimeToIds; //ʱ��-id���������ڽ�id��ʱ������
    QMultiMap<int, int> m_boxToIds; //����-id����
    int m_totalNum;
    int m_unreadInboxNum;
    int m_unreadDustbinNum;
    QVector<int> m_boxesNum;
    QReadWriteLock m_lock;
    QTimer m_timerIdle;
    int m_changeTimes;
    QMessageBox m_msg;
    
    void createMap();
    Message getMessageOfIdFromInner(int id);
    bool checkValidity(Message::MsgType msgtype,
                       Message::Box box,
                       Message::State state);
    template<typename T> void insertIdToMap(int id,
                                            QMultiMap<T, int> &map,
                                            const T &key,
                                            const QDateTime &datetime);
    template<typename T> QList<Message> getMidMessages(const QMultiMap<T, int> &map,
                                                       const T &key,
                                                       int position,
                                                       int n);
    
signals:
    void signMsgChanged(int, int, int);
    void signSave();
};

#endif // ABSTRACTMSG_H
