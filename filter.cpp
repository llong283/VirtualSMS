/************************************************
����: Filter
����: ���ݹ��˹�����յ�����Ϣ���й��ˣ����ع��˺������
************************************************/

#include "filter.h"

Filter::Filter(QObject *parent) :
    QObject(parent)
{
}

//���ݹ��˹�����յ�����Ϣ���й��ˣ����ع��˺������
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

//��Ϣ���ݷ��Ϲ���������(�����������)������ true�����򷵻�false
bool Filter::keyWordsFilter(const QString &messageContent)
{
    //�ַ������뷽ʽ
    QList<QString> keyWords = g_setting->getKeywords();
    foreach(QString key ,keyWords)
    {
        if(messageContent.contains(key,Qt::CaseInsensitive))
            return true;
    }
    return false;
}

//�����ں����������true�����򷵻�false
bool Filter::blackListFilter(const QString& senderPhoneNum)
{
    Contact contact = g_phoneBook->getContactOfPhoneNum(senderPhoneNum);
    if(contact.isValid() && contact.type==Contact::Type_BlackList) {
        return true;
    } else {
        return false;
    }
}

//�����ڰ����������true�����򷵻�false
bool Filter::whiteListFilter(const QString& senderPhoneNum)
{
    Contact contact = g_phoneBook->getContactOfPhoneNum(senderPhoneNum);
    if(contact.isValid() && contact.type==Contact::Type_WhiteList) {
        return true;
    } else {
        return false;
    }
}
