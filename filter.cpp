/************************************************
类名: Filter
功能: 根据过滤规则对收到的信息进行过滤，返回过滤后的类型
************************************************/

#include "filter.h"

Filter::Filter(QObject *parent) :
    QObject(parent)
{
}

//根据过滤规则对收到的信息进行过滤，返回过滤后的类型
Message::Box Filter::messageFilter(const QString& senderPhoneNum, const QString &messageContent)
{
    Message::Box messageBox(Message::Box_Inbox);

    int filterMode = g_setting->getFilterMode();
    switch (filterMode) {
    case FM_Unused:
        if (keyWordsFilter(messageContent))
            messageBox = Message::Box_Dustbin;
        else
            messageBox = Message::Box_Inbox;
        break;
    case FM_Blacklist:
        if (!blackListFilter(senderPhoneNum) &&
                !keyWordsFilter(messageContent))
            messageBox = Message::Box_Inbox;
        else
            messageBox = Message::Box_Dustbin;
        break;
    case FM_Whitelist:
        if (whiteListFilter(senderPhoneNum) &&
                !keyWordsFilter(messageContent))
            messageBox = Message::Box_Inbox;
        else
            messageBox = Message::Box_Dustbin;
        break;
    default:
        break;
    }

    return messageBox;
}

//信息内容符合过滤条件的(需放入垃圾箱)，返回 true；否则返回false
bool Filter::keyWordsFilter(const QString &messageContent)
{
    //字符串编码方式
    QList<QString> keyWords = g_setting->getKeywords();
    foreach(QString key ,keyWords)
    {
        if(messageContent.contains(key,Qt::CaseInsensitive))
            return true;
    }
    return false;
}

//号码在黑名单里，返回true，否则返回false
bool Filter::blackListFilter(const QString& senderPhoneNum)
{
    Contact contact = g_phoneBook->getContactOfPhoneNum(senderPhoneNum);
    if(contact.isValid() && contact.type==Contact::Type_BlackList) {
        return true;
    } else {
        return false;
    }
}

//号码在白名单里，返回true，否则返回false
bool Filter::whiteListFilter(const QString& senderPhoneNum)
{
    Contact contact = g_phoneBook->getContactOfPhoneNum(senderPhoneNum);
    if(contact.isValid() && contact.type==Contact::Type_WhiteList) {
        return true;
    } else {
        return false;
    }
}
