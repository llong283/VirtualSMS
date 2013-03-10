#ifndef CONFIG_H
#define CONFIG_H

#include <QtCore>
#include <QtGui>
#include <QtXml>
#include <QDomDocument>
#include <QHostAddress>

const int cons_head_size = 32; //��ϵ��ͷ���С
const int cons_btn_icon_size = 25; //��ťͼ���С
const int cons_msg_item_height = 40; //��Ϣ�б���߶�
const int cons_msg_image_size = 10; //��Ϣ�б�ͼ���С
const int cons_contact_item_height = 30; //��ϵ����߶�
const int cons_other_item_height = 20; //�����б���߶�
const int cons_tab_size = 40; //��ǩҳ�߶�
const int cons_phonebook_btn_size = 15; //�绰����ťͼ���С
const int cons_tree_item_height = 20; //����ĸ߶�

const int cons_MessageListenport = 1573; //��Ϣ���յ�ͨ�Ŷ˿ڣ�
//const int cons_Remoteport = 1572;
const int cons_broadcast_port = 45645;

//�㲥����ͷ
const QString cons_info              = "Command_Info";
const QString cons_quit              = "Command_Quit";
//����ͻͷ
const QString cons_check_conflict    = "Command_CheckConflict";
const QString cons_reply_conflict    = "Command_ReplyConflict";

//����
enum Language {
    Language_Chinese, //����
    Language_English //Ӣ��
};

//�ʶ�����
enum SpeakLanguage {
    Speak_Chinese,
    Speak_English
};

//����ģʽ
enum RunMode {
    RunMode_Chat, //����ģʽ
    RunMode_Message //����ģʽ
};

//����ģʽ
enum FilterMode {
    FM_Unused, //δ����
    FM_Blacklist, //���ú�����
    FM_Whitelist //���ð�����
};

//�㲥��ʽ
enum Broadcast {
    Broadcast_Command,
    Broadcast_Name,
    Broadcast_CurPhoneNum,
    Broadcast_PrevPhoneNum,
    Broadcast_Size
};

enum Command {
    Command_Invalid,
    Command_Info,
    Command_Quit,
    Command_CheckConflict,
    Command_ReplyConflict
};

//����ʶ��״̬
enum IdentificationState
{
    Noting = 0,   //ͳ���û���Ϊ�γ�ϰ�ߺ�֪ͨ���棬״̬Ϊ��Caution��
    Caution =1,  //���ѣ�����ͳ����Ϊ������ҹ���ʱ��Ҫ�����û�
    Refuse = 2,   //��ֹ������ͳ����Ϊ������ҹ���ʱ����Ҫ����
    Allow = 3,   //��������ͳ�Ʋ���Ϊ������
    Delete = 4 ,  //���ɾ����ʾ�ڽ������ʾ���ԭ����¼�ϼ���ͳ�ƣ������¼��״̬תΪ��Noting��
//    Empty = 5     //���ɾ����ʾ�ڽ������ʾ���ɾ����ʷͳ�ƣ����¿�ʼͳ�ƣ������¼��״̬תΪ��Noting��
    Nonoting = 6   //��û��¼
};
struct MIInformation //MessageIdentification����Ϣ��
{
    QString keyWord;           //�ؼ���
    int count;                 //���ִ���
    IdentificationState state; //����

};
struct PNIInformation  //PhoneNumberIdentification����Ϣ��
{
    QString number;  //����
    int count;       //��¼����
    IdentificationState state; //״̬
};
struct NumberSegmentInformation
{
   qulonglong startNumber;
   qulonglong endNumber;
   IdentificationState state ;
   qulonglong count ;
   QList<qulonglong> Numbers;
};

//��ϵ��
struct Contact {
    int id; //����id
    QString name; //����
    QString phonenum; //����
    int type; //����
    int state; //״̬
    
    //����
    enum Type {
        Type_Other, //����
        Type_BlackList, //������
        Type_WhiteList, //������
        Type_Stranger, //İ����
        Type_Myself //�Լ�
    };    
    
    //״̬
    enum State {
        State_Offline, //����
        State_Online //����
    };
    
    //�ҵĺ���ĸ�ʽcurrentPhoneNum#previousPhoneNum
    enum MyPhoneNum {
        MyPhoneNum_Current,
        MyPhoneNum_Previous
    };

    Contact() : id(-1) {
    }
    inline bool isValid() {
        return id!=-1 && isTypeValid();
    }
    inline bool isTypeValid() {
        return type>=Type_Other && type<=Type_Myself;
    }

    bool operator==(const Contact &contact) {
        return id==contact.id;
    }    
    inline QString getCurrentPhoneNum() {        
        if (type == Type_Myself) {
            return phonenum.split("#").at(MyPhoneNum_Current);
        } else {
            return phonenum;
        }
    }
    static QString contactTypeToString(Contact::Type type)
    {
        QString strType;
        switch (type) {
        case Contact::Type_Myself:
            strType = QObject::tr("Myself");
            break;
        case Contact::Type_Other:
            strType = QObject::tr("Other");
            break;
        case Contact::Type_WhiteList:
            strType = QObject::tr("WhiteList");
            break;
        case Contact::Type_BlackList:
            strType = QObject::tr("BlackList");
            break;
        case Contact::Type_Stranger:
            strType = QObject::tr("Stranger");
            break;
        default:
            break;
        }
        return strType;
    }
};
Q_DECLARE_METATYPE(Contact);

//��Ϣ
struct Message {
    int id; //����id
    QString phonenum; //����
    QDateTime datetime; //ʱ��
    QString content; //��Ϣ����
    int msgtype; //��Ϣ����
    int box; //����
    int state; //״̬
    
    //��Ϣ����
    enum MsgType {
        MsgType_InMsg, //������Ϣ
        MsgType_OutMsg //������Ϣ
    };
    
    //����
    enum Box {
        Box_Inbox, //�ռ���
        Box_Outbox, //������
        Box_Draftbox, //�ݸ���
        Box_Dustbin //������
    };
    
    //״̬
    enum State {
        State_Unread, //δ��
        State_Read, //�Ѷ�
        State_Unsend, //δ����
        State_Sent, //�ѷ���
        State_SendFail //����ʧ��
    };
    
    Message() : id(-1) {        
    }
    bool operator==(const Message &msg) {
        return id==msg.id;
    }
    inline bool isEmpty() {
        return id==-1;
    }
    inline bool isValid() {
        return id!=-1 && isMsgTypeValid() && isBoxValid() && isStateValid();
    }
    inline bool isMsgTypeValid() {
        return msgtype==MsgType_InMsg || msgtype==MsgType_OutMsg;
    }
    inline bool isBoxValid() {
        if (msgtype == MsgType_InMsg) {
            return box==Box_Inbox || box==Box_Dustbin;
        } else {
            return box==Box_Outbox || box==Box_Draftbox || box==Box_Dustbin;
        }
    }
    inline bool isStateValid() {
        if (msgtype == MsgType_InMsg) {
            return state==State_Unread || state==State_Read;
        } else {
            return state==State_Unsend || state==State_Sent || state==State_SendFail;
        }
    }
    static QString msgBoxToTr(Message::Box box)
    {
        QString strBox;
        switch (box) {
        case Message::Box_Inbox:
            strBox = QObject::tr("Inbox");
            break;
        case Message::Box_Outbox:
            strBox = QObject::tr("Outbox");
            break;
        case Message::Box_Draftbox:
            strBox = QObject::tr("Draftbox");
            break;
        case Message::Box_Dustbin:
            strBox = QObject::tr("Dustbin");
            break;
        default:
            strBox = QObject::tr("Invalid box");
            break;
        }
        return strBox;
    }
    static QString msgStateToTr(Message::State state) {
        QString strState;
        switch (state) {
        case Message::State_Unread:
            strState = QObject::tr("Unread");
            break;
        case Message::State_Read:
            strState = QObject::tr("Read");
            break;
        case Message::State_Unsend:
            strState = QObject::tr("Unsend");
            break;
        case Message::State_Sent:
            strState = QObject::tr("Sent");
            break;
        case Message::State_SendFail:
            strState = QObject::tr("SendFail");
            break;
        default:
            strState = QObject::tr("Invalid state");
            break;
        }
        return strState;
    }
    static QString msgStateToString(Message::State state) {
        QString strState;
        switch (state) {
        case Message::State_Unread:
            strState = "Unread";
            break;
        case Message::State_Read:
            strState = "Read";
            break;
        case Message::State_Unsend:
            strState = "Unsend";
            break;
        case Message::State_Sent:
            strState = "Sent";
            break;
        case Message::State_SendFail:
            strState = "SendFail";
            break;
        default:
            break;
        }
        return strState;
    }
};
Q_DECLARE_METATYPE(Message);

#endif // CONFIG_H
