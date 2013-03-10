#include "phonenumberidentification.h"


PhoneNumberIdentification* PhoneNumberIdentification::phoneNumberIdentification = NULL;

PhoneNumberIdentification::PhoneNumberIdentification(QObject *parent) :
    QObject(parent),fileNameforPNI("phoneNumberIdentification.txt")
{
    filterCount =4;
    init();
}

//初始化
void PhoneNumberIdentification::init()
{
    qDebug() <<"PhoneNumberIdentification Init.";
    numberRecords.clear();
    if(!QFile::exists(fileNameforPNI))
    {  qDebug()<< "PhoneNumberIdentification, fileNameforPNI no exists.";
       return;
    }
    else
    {
        QFile file(fileNameforPNI);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug()<< "PhoneNumberIdentification,open fileNameforPNI fail.";
        }
        QTextStream out(&file);
        QString dataLine ;
        PNIInformation numInformation;
        //数据格式（每行一条记录，中间以空格隔开）：号码 + 次数 + 属性 +"\n"
        while((dataLine=out.readLine()) != NULL)
        {
            int firstPosition = dataLine.indexOf(" ");
            if(firstPosition>0)
            {
                 numInformation.number = dataLine.mid(0,firstPosition);
                 qDebug()<< "number is:"+numInformation.number;
            }
            else
            {
                //格式不对，删除该行，继续解析
                qDebug()<<"firstPosition <=0";
                goto FormatError;
//               // continue;
            }
            int secondPosition = dataLine.indexOf(" ",firstPosition+1);
            if(secondPosition>0)
            {
               numInformation.count = dataLine.mid(firstPosition+1,
                                                   secondPosition-(firstPosition+1)).toInt();
               qDebug()<< "count is:"+QString::number(numInformation.count);
            }
            else
            {
                //格式不对，删除该行，继续解析
                qDebug()<<"secondPosition <=0";
                goto FormatError;
                //continue;
            }
            int thirdPosition = dataLine.indexOf(" ",secondPosition+1);
            if(thirdPosition>0)
            {
               numInformation.state = (IdentificationState)dataLine.mid(secondPosition+1,
                                                                        thirdPosition-(secondPosition+1)).toInt();
               qDebug()<< "state is:"+QString::number(numInformation.state);
            }
            else
            {
                //格式不对，删除该行，继续解析
                qDebug()<<"thirdPosition <=0";
                goto FormatError;
              //  continue;
            }
            numberRecords.insert(numInformation.number,numInformation);

            continue;
FormatError:
            qDebug()<<"PhoneNumberIdentification init ，格式不对";
        }
        file.close();
    }
    //显示结果
    QMapIterator<QString,PNIInformation> iterator(numberRecords);
    while(iterator.hasNext())
    {
        iterator.next();
        qDebug() <<"numberRecords number is:"+iterator.value().number +
                   " count is :" +QString::number(iterator.value().count)+
                   " state is:" +QString::number(iterator.value().state);

    }

//    QMapIterator<QString,int> iterator(notedNumbers);
//    while(iterator.hasNext())
//    {
//        iterator.next();
//        qDebug() <<"notedNumbers is:" +iterator.key()+"; weight is:" +QString::number(iterator.value());

//    }
//    QMapIterator<QString,int> iteratorN(notedNumbers);
//    while(iteratorN.hasNext())
//    {
//        iteratorN.next();
//        qDebug() <<"notedNumbers is:" +iteratorN.key()+"; weight is:" +QString::number(iteratorN.value());

//    }



}

//单例调用
PhoneNumberIdentification* PhoneNumberIdentification::instance()
{
    if(phoneNumberIdentification == NULL)
        phoneNumberIdentification = new PhoneNumberIdentification;
    return phoneNumberIdentification;
}

//返回显示列表，用于界面显示及用户习惯的判断
QList<PNIInformation> PhoneNumberIdentification::getShowList(int count)
{
    filterCount = count;
    qDebug()<<"PhoneNumberIdentification,filterCount is:" +QString::number(filterCount);
    QList<PNIInformation> showlist;

    QMapIterator<QString,PNIInformation> iterator(numberRecords);
    IdentificationState state;
    bool changing = false;
    while(iterator.hasNext())
    {
        iterator.next();
        state = iterator.value().state;
        //当记录次数的边界值发生改变时，已经标记为Caution的项是否需要重新加载
        if(state == Allow ||  state == Refuse /*||state == Caution*/ )
            showlist.append(iterator.value());
        else if( state == Noting)
        {
            if(iterator.value().count>=count)
            {
                numberRecords[iterator.key()].state= Caution;//状态改变
                changing =true;
                showlist.append(numberRecords[iterator.key()]);
            }
        }
        else if(state ==Caution)
        {
            if(iterator.value().count<count)
            {
                numberRecords[iterator.key()].state= Noting;//状态改变
                changing =true;
            }
            else
                showlist.append(iterator.value());
        }

    }
    if(changing)
        saveKeyNumbers();

    return showlist;
}

//保存用户设置
void PhoneNumberIdentification::setNumberPropety(const QString Number, IdentificationState command)
{
    qDebug()<<"setNumberPropety";
    if(numberRecords.contains(Number))
    {
        if(command == Delete)
        {
            numberRecords[Number].count =0;
            numberRecords[Number].state = Noting;
        }
        else
        {
             numberRecords[Number].state = command;
        }
    }
    else
    {
        PNIInformation record;
        record.state = command;
        record.number = Number;
        record.count =0;
        numberRecords.insert(Number,record);
    }
    saveKeyNumbers();
}

//记录用户操作
void PhoneNumberIdentification::NumberIdentificationProcessAndSave(QString Number)
{
    if(numberRecords.contains(Number))
    {
        numberRecords[Number].count++;
        if(numberRecords[Number].state == Noting &&
                numberRecords[Number].count>=filterCount)
        {
            numberRecords[Number].state = Caution;
            emit NewUserNumberhabit(numberRecords[Number]);
        }
    }
    else
    {
        PNIInformation record;
        record.state = Noting;
        record.number = Number;
        record.count =1;
        numberRecords.insert(Number,record);
    }

    saveKeyNumbers();

}

//判断号码是否符合用户使用习惯，返回结果
PNIInformation PhoneNumberIdentification::numberIdentification(QString Number)
{
    if(numberRecords.contains(Number))
    {
        return numberRecords[Number];
    }
    else
    {
        PNIInformation recordTmp;
        recordTmp.state = Allow;
        recordTmp.number = Number;
        recordTmp.count =0;
        return recordTmp;

    }
}

void PhoneNumberIdentification::saveKeyNumbers()
{
    //所有重写，或部分修改
    QFile file(fileNameforPNI);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //         QMessageBox::warning(this,fileNameforMI,"can't open",QMessageBox::Yes);
        qDebug()<< "open fileNameforPNI fail.";
    }
    else
        qDebug()<< "open fileNameforPNI success.";

    QTextStream in(&file);
    QMapIterator<QString,PNIInformation> iterator(numberRecords);
    while(iterator.hasNext())
    {
        iterator.next();
//        qDebug() <<"numberRecords number is:"+iterator.value().number +
//                   " count is :" +QString::number(iterator.value().count)+
//                   " state is:" +QString::number(iterator.value().state);
        in <<(iterator.value().number+" "+
              QString::number(iterator.value().count) + " " +
              QString::number(iterator.value().state)+" "+
              "\n");

    }


    file.close();

}
