#ifndef CONFIG_H
#define CONFIG_H

#include <QtCore>
#include <QtGui>
#include <QtXml>
#include <QDomDocument>
#include <QHostAddress>

const int cons_head_size = 32; //联系人头像大小
const int cons_btn_icon_size = 25; //按钮图标大小
const int cons_msg_item_height = 40; //信息列表项高度
const int cons_msg_image_size = 10; //信息列表图标大小
const int cons_contact_item_height = 30; //联系人项高度
const int cons_other_item_height = 20; //其他列表项高度
const int cons_tab_size = 40; //标签页高度
const int cons_phonebook_btn_size = 15; //电话本按钮图标大小
const int cons_tree_item_height = 20; //树项的高度

const int cons_MessageListenport = 1573; //信息接收的通信端口，
//const int cons_Remoteport = 1572;
const int cons_broadcast_port = 45645;

//广播命令头
const QString cons_info              = "Command_Info";
const QString cons_quit              = "Command_Quit";
//检查冲突头
const QString cons_check_conflict    = "Command_CheckConflict";
const QString cons_reply_conflict    = "Command_ReplyConflict";

//语言
enum Language {
    Language_Chinese, //中文
    Language_English //英文
};

//朗读语言
enum SpeakLanguage {
    Speak_Chinese,
    Speak_English
};

//运行模式
enum RunMode {
    RunMode_Chat, //聊天模式
    RunMode_Message //短信模式
};

//过滤模式
enum FilterMode {
    FM_Unused, //未启用
    FM_Blacklist, //启用黑名单
    FM_Whitelist //启用白名单
};

//广播格式
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

//智能识别状态
enum IdentificationState
{
    Noting = 0,   //统计用户行为形成习惯后，通知界面，状态为”Caution“
    Caution =1,  //提醒，该项统计作为过滤项，且过滤时需要提醒用户
    Refuse = 2,   //禁止，该项统计作为过滤项，且过滤时不需要提醒
    Allow = 3,   //允许，该项统计不作为过滤项
    Delete = 4 ,  //命令，删除显示在界面的显示项，在原来记录上继续统计，该项记录的状态转为”Noting“
//    Empty = 5     //命令，删除显示在界面的显示项，并删除历史统计，重新开始统计，该项记录的状态转为”Noting“
    Nonoting = 6   //还没记录
};
struct MIInformation //MessageIdentification的信息项
{
    QString keyWord;           //关键词
    int count;                 //出现次数
    IdentificationState state; //属性

};
struct PNIInformation  //PhoneNumberIdentification的信息项
{
    QString number;  //号码
    int count;       //记录次数
    IdentificationState state; //状态
};
struct NumberSegmentInformation
{
   qulonglong startNumber;
   qulonglong endNumber;
   IdentificationState state ;
   qulonglong count ;
   QList<qulonglong> Numbers;
};

//联系人
struct Contact {
    int id; //虚拟id
    QString name; //姓名
    QString phonenum; //号码
    int type; //类型
    int state; //状态
    
    //类型
    enum Type {
        Type_Other, //其他
        Type_BlackList, //白名单
        Type_WhiteList, //黑名单
        Type_Stranger, //陌生人
        Type_Myself //自己
    };    
    
    //状态
    enum State {
        State_Offline, //离线
        State_Online //在线
    };
    
    //我的号码的格式currentPhoneNum#previousPhoneNum
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

//信息
struct Message {
    int id; //虚拟id
    QString phonenum; //号码
    QDateTime datetime; //时间
    QString content; //信息内容
    int msgtype; //信息类型
    int box; //信箱
    int state; //状态
    
    //信息类型
    enum MsgType {
        MsgType_InMsg, //接收信息
        MsgType_OutMsg //发送信息
    };
    
    //信箱
    enum Box {
        Box_Inbox, //收件箱
        Box_Outbox, //发件箱
        Box_Draftbox, //草稿箱
        Box_Dustbin //垃圾箱
    };
    
    //状态
    enum State {
        State_Unread, //未读
        State_Read, //已读
        State_Unsend, //未发送
        State_Sent, //已发送
        State_SendFail //发送失败
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
