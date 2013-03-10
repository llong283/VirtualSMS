#include "MessageIdentification.h"
#include "QFile"
#include <QDebug>
#include <QTextStream>

MessageIdentification* MessageIdentification::MIInstance = NULL;

MessageIdentification::MessageIdentification(QObject *parent) :
    QObject(parent),fileNameforMI("MessageIdentification.txt")
{
    filterCount = 8;
    Init();
}
MessageIdentification *MessageIdentification::instance()
{
    if (MIInstance == NULL) {
        MIInstance = new MessageIdentification;
    }
    return MIInstance;
}
void MessageIdentification::Init()
{
    qDebug() <<"MessageIdentification::Init()";
    messageRecords.clear();
    if(!QFile::exists(fileNameforMI))
    {  qDebug()<< "MessageIdentification, fileNameforMI no exists.";
       return;
    }
    else
    {
        QFile file(fileNameforMI);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug()<< "MessageIdentification,open fileNameforMI fail.";
        }
        QTextStream out(&file);
        QString dataLine ;
        MIInformation record;
        //���ݸ�ʽ��ÿ��һ����¼���м��Կո���������ؼ��� + ���� + ���� +"\n"
        while((dataLine=out.readLine()) != NULL)
        {
            int firstPosition = dataLine.indexOf(" ");
            if(firstPosition>0)
            {
                 record.keyWord = dataLine.mid(0,firstPosition);
                 qDebug()<< "keyWord is:"+record.keyWord;
            }
            else
            {
                //��ʽ���ԣ�ɾ�����У���������
                qDebug()<<"firstPosition <=0";
                goto FormatError;
//               // continue;
            }
            int secondPosition = dataLine.indexOf(" ",firstPosition+1);
            if(secondPosition>0)
            {
               record.count = dataLine.mid(firstPosition+1,
                                                   secondPosition-(firstPosition+1)).toInt();
               qDebug()<< "count is:"+QString::number(record.count);
            }
            else
            {
                //��ʽ���ԣ�ɾ�����У���������
                qDebug()<<"secondPosition <=0";
                goto FormatError;
                //continue;
            }
            int thirdPosition = dataLine.indexOf(" ",secondPosition+1);
            if(thirdPosition>0)
            {
               record.state = (IdentificationState)dataLine.mid(secondPosition+1,
                                                                        thirdPosition-(secondPosition+1)).toInt();
               qDebug()<< "state is:"+QString::number(record.state);
            }
            else
            {
                //��ʽ���ԣ�ɾ�����У���������
                qDebug()<<"thirdPosition <=0";
                goto FormatError;
              //  continue;
            }
            messageRecords.insert(record.keyWord,record);
            if(record.state == Caution || record.state == Refuse)
                filterRecords.insert(record.keyWord,record);

            continue;
FormatError:
            qDebug()<<"MessageIdentification init ����ʽ����";

        }
        file.close();
    }
    //��ʾ���
    QMapIterator<QString,MIInformation> iterator(messageRecords);
    while(iterator.hasNext())
    {
        iterator.next();
        qDebug() <<"notedWords is:" +iterator.value().keyWord+
                   "; count is:" +QString::number(iterator.value().count) +
                   ";state is:"+QString::number(iterator.value().state);
    }

}

void MessageIdentification::setWordPropety(const QString data, IdentificationState command)
{
    if(messageRecords.contains(data))
    {
        if(command == Delete)
        {
            messageRecords[data].count =0;
            messageRecords[data].state = Noting;
        }
        else
        {
             messageRecords[data].state = command;
        }
    }
    else
    {
        MIInformation record;
        record.state = command;
        record.keyWord = data;
        record.count =0;
        messageRecords.insert(data,record);
    }
    if(messageRecords[data].state == Caution || messageRecords[data].state == Refuse)
    {
        if(!filterRecords.contains(data))
            filterRecords.insert(data,messageRecords[data]);
    }
    else
    {
          if(filterRecords.contains(data))
              filterRecords.remove(data);

    }
    saveKeywords();
}

void MessageIdentification::MessageIdentificationProcessAndSave(QString &data)
{
    QMap<QString, int> words =distinguishPhrase.DistinguishPhraseProcessAndFilter(data);
    QMapIterator<QString,int> iterator(words);
    while(iterator.hasNext())
    {
        iterator.next();
        if(messageRecords.contains(iterator.key()))
        {
            messageRecords[iterator.key()].count += iterator.value();
            if(messageRecords[iterator.key()].state == Noting &&
                    messageRecords[iterator.key()].count>=filterCount)
            {
                messageRecords[iterator.key()].state = Caution;
                filterRecords.insert(iterator.key(),messageRecords[iterator.key()]);
                emit NewUserMessagehabit(messageRecords[iterator.key()]);
            }
        }
        else
        {
            MIInformation record;
            record.state = Noting;
            record.keyWord = iterator.key();
            record.count =iterator.value();
            messageRecords.insert(record.keyWord,record);
        }

    }
    saveKeywords()  ;
}

QList<MIInformation> MessageIdentification::getShowList(int count)
{
    filterCount = count;
    qDebug()<<"MessageIdentification,filterCount is:" +QString::number(filterCount);
    QList<MIInformation> showlist;

    QMapIterator<QString,MIInformation> iterator(messageRecords);
    IdentificationState state;
    bool changing = false;
    while(iterator.hasNext())
    {
        iterator.next();
        state = iterator.value().state;
        //����¼�����ı߽�ֵ�����ı�ʱ���Ѿ����ΪCaution�����Ƿ���Ҫ���¼���
        if(state == Allow ||  state == Refuse /*||state == Caution*/ )
            showlist.append(iterator.value());
        else if( state == Noting)
        {
            if(iterator.value().count>=count)
            {
                messageRecords[iterator.key()].state= Caution;//״̬�ı�
                filterRecords.insert(iterator.key(),messageRecords[iterator.key()]);
                changing =true;
                showlist.append(messageRecords[iterator.key()]);
            }
        }
        else if(state ==Caution)
        {
            if(iterator.value().count<count)
            {
                messageRecords[iterator.key()].state= Noting;//״̬�ı�
                if(filterRecords.contains(iterator.key()))
                    filterRecords.remove(iterator.key());
                changing =true;
            }
            else
                showlist.append(iterator.value());
        }

    }
    if(changing)
        saveKeywords();

    return showlist;

}

//���÷ִ����еĽӿڣ�������ʱ��浽���ôʱ���
bool MessageIdentification::addCommonWord(QString word)
{
    return distinguishPhrase.addCommonWord(word);
}

//�ж��Ƿ���Ҫ���ˣ����ؽ��
MIInformation MessageIdentification::messageIdentification(QString data)
{
    QMapIterator<QString,MIInformation> iterator(filterRecords);
    while(iterator.hasNext())
    {
        iterator.next();
        if(data.contains(iterator.key()))
            return filterRecords[iterator.key()];
    }
    MIInformation Information;
    Information.state = Nonoting;
    return Information;
}
//��������
void MessageIdentification::saveKeywords()
{
    //������д���򲿷��޸�
    QFile file(fileNameforMI);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //         QMessageBox::warning(this,fileNameforMI,"can't open",QMessageBox::Yes);
        qDebug()<< "open fileNameforMI fail.";
    }
    else
        qDebug()<< "open fileNameforMI success.";

    QTextStream in(&file);
    QMapIterator<QString,MIInformation> iteratorN(messageRecords);
    while(iteratorN.hasNext())
    {
        iteratorN.next();
        //���һ�У���ʽ:�ؼ���+�ո�+���ִ���+�Ƿ��¼��1��ʾ��¼��0��ʾ�ǹ���+�ո�+"\n"
        in <<(iteratorN.value().keyWord+" "+
              QString::number(iteratorN.value().count) + " "
              + QString::number(iteratorN.value().state)+" "
              "\n");
//        qDebug() <<"keyword is:" +iterator.key()+"; weight is:" +QString::number(iterator.value());

    }
    file.close();

}
