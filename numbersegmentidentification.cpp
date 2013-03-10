#include "numbersegmentidentification.h"
#include <QFile>
#include <QDebug>
#include <QtGlobal>


const quint64 MinPhoneNumber = 1;
const quint64 MaxPhoneNumber = Q_UINT64_C(1000*1000*1000*100); //小于11位十进制数

NumberSegmentIdentification* NumberSegmentIdentification::NSIInstance = NULL;

NumberSegmentIdentification::NumberSegmentIdentification(QObject *parent) :
    QObject(parent),fileNameforNSI("NumberSegmentIdentification.txt")
{
//  quint64 MaxPhoneNumber = Q_UINT64_C(1000*1000*1000*1000-1); //小于11位十进制数
//    qDebug()<<"the MaxPhoneNumber is"+QString::number(MaxPhoneNumber);
    filterCount = 6;
    init();

}
NumberSegmentIdentification * NumberSegmentIdentification::instance()
{
    if(NSIInstance == NULL)
        NSIInstance =  new NumberSegmentIdentification;
    return NSIInstance;
}

void NumberSegmentIdentification::init()
{
    qDebug() <<"NumberSegmentIdentification Init.";
    allNotingNumbers.clear();
    records.clear();
    if( QFile::exists(fileNameforNSI))
    {
        QFile file(fileNameforNSI);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug()<< "PhoneNumberIdentification,open fileNameforPNI fail.";
        }
        QTextStream out(&file);
        QString dataLine ;
        QString startNumber;
        struct NumberSegmentInformation numberSegmentInformation;
        //两条相邻的记录的两个起始号码之间构成一个号码段
        numberSegmentInformation.startNumber = 0; //初始为0，为了判断是否已经收集完第1条记录，此时不为0
        struct NumberSegmentInformation numberSegmentInformationTmp;

        while((dataLine=out.readLine()) != NULL)
        {
            //起始号码
            int firstPosition = dataLine.indexOf(" ");
            if(firstPosition>0)
            {
                 startNumber = dataLine.mid(0,firstPosition);
                 qDebug()<< "startNumber is:"+startNumber;
                 numberSegmentInformationTmp.startNumber = startNumber.toULongLong();
            }
            else
            {
//                //格式不对，删除该行，继续解析
                qDebug()<<"firstPosition <=0";
//                continue;
                  goto FormatError;
            }
            //状态
            int secondPosition = dataLine.indexOf(" ",firstPosition+1);
            if(secondPosition>0)
            {
                numberSegmentInformationTmp.state = (IdentificationState)dataLine.mid(firstPosition+1,secondPosition-(firstPosition+1)).toInt();
                qDebug()<< "state is:"+QString::number(numberSegmentInformationTmp.state);
            }
            else
            {
//                //格式不对，删除该行，继续解析
                qDebug()<<"secondPosition <=0";
//                continue;
                  goto FormatError;
            }
            //记录的号码个数
            int thirdPosition = dataLine.indexOf(" ",secondPosition+1);
            if(thirdPosition>0)
            {
                numberSegmentInformationTmp.count = dataLine.mid(secondPosition+1,thirdPosition-(secondPosition+1)).toInt();
                qDebug()<< "count is:"+QString::number(numberSegmentInformationTmp.count);
            }
            else
            {
                //格式不对，删除该行，继续解析
                qDebug()<<"thirdPosition <=0";
                goto FormatError;
            }

            //提取记录的号码
            int position1 = thirdPosition ;
            int position2 ;
            int count =numberSegmentInformationTmp.count;
            numberSegmentInformationTmp.Numbers.clear();
            while(count)
            {
                position2 = dataLine.indexOf(" ",position1+1);
                if(position2>0)
                {
                    numberSegmentInformationTmp.Numbers.append(dataLine.mid(position1+1,position2-(position1+1)).toULongLong());
                    allNotingNumbers.append(dataLine.mid(position1+1,position2-(position1+1)).toULongLong());
                    position1 = position2;
                }
                else
                {
                    //格式不对，删除该行，继续解析
                    qDebug()<<"position2 <=0";

                    goto FormatError;
                }
                count--;

            }

            //号码段的结束号码
            if(numberSegmentInformation.startNumber >0)
            {
                 numberSegmentInformation.endNumber = numberSegmentInformationTmp.startNumber-1;
                 records.insert( numberSegmentInformation.startNumber, numberSegmentInformation );

                 numberSegmentInformation = numberSegmentInformationTmp;
            }
            else
             {
                qDebug()<<"read first record";
                numberSegmentInformation = numberSegmentInformationTmp;
//                qDebug() << "records start number:" + QString::number(numberSegmentInformation.startNumber)+
//                            ",end number:" + QString::number(numberSegmentInformation.endNumber)+
//                             ",state:" + QString::number(numberSegmentInformation.state)+
//                             ",count:" + QString::number(numberSegmentInformation.count);


            }

            continue;

FormatError:
            qDebug()<<"NumberSegmentIdentification init ，格式不对";

        }

        file.close();

    }
    else
    {
         //文件不存在时的处理方法
        qDebug()<< "PhoneNumberIdentification, fileNameforPNI no exists.";
        createNewNumberSegment();
    }


    //显示结果
    qDebug()<<"NumberSegmentIdentification init result.";
//    showresult();
    saveKeyNumberSegments();
//    for(int i=0;i<allNotingNumbers.count();i++)
//    {
//        qDebug()<<"allNotingNumbers " +QString::number(i) + " is :" +
//                  QString::number(allNotingNumbers[i]);
//    }

//    QMapIterator<qulonglong,NumberSegmentInformation> iterator(records);
//    while(iterator.hasNext())
//    {
//        iterator.next();
//        qDebug() << "records start number:" + QString::number(iterator.value().startNumber)+
//                    ",end number:" + QString::number(iterator.value().endNumber)+
//                     ",state:" + QString::number(iterator.value().state)+
//                     ",count:" + QString::number(iterator.value().count);
//    }

}
//初始化号段，以号码位数相同的位一个号段


void NumberSegmentIdentification::createNewNumberSegment()
{
     QFile file(fileNameforNSI);
     if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
     {
         qDebug()<< "create fileNameforNSI fail.";
     }
     else
         qDebug()<< "create fileNameforNSI success.";
//     int numberEnd = numberStart*10 -1;
     QTextStream in(&file);
     qulonglong numberStart = 1;
     struct NumberSegmentInformation numberSegmentInformation;
     while(numberStart<MaxPhoneNumber)
     {

         numberSegmentInformation.startNumber = numberStart;

         numberSegmentInformation.endNumber =numberStart*10 - 1;
         numberSegmentInformation.count = 0;
         numberSegmentInformation.state =Noting;
         numberSegmentInformation.Numbers.clear();

         records.insert(numberStart,numberSegmentInformation);
         //格式：起始号码 + 属性（该号段是否需要统计）+号段中记录的号码个数n + 号码1+号码2+···号码n +"\n"
         in << QString::number(numberStart)+" "+QString::number(Noting)+ " " + QString::number(0)+" " +  "\n";
         numberStart *= 10;
     }
     in << QString::number(numberStart)+" "+QString::number(Noting)+ " " + QString::number(0)+" " + "\n";

     file.close();
    

}

//根据记录次数（默认为5次），获得需要显示在界面的数据列表
QList<NumberSegmentInformation> NumberSegmentIdentification::getShowList(int count)
{
    qDebug()<<" NumberSegmentIdentification::getShowList";
    if(count !=0)
        filterCount = count;
    QList<NumberSegmentInformation> showlist;
    bool changing = false;
    QMapIterator<qulonglong,NumberSegmentInformation> iteratorNS(records);
    IdentificationState state;
    while(iteratorNS.hasNext())
    {
        iteratorNS.next();
        state =  iteratorNS.value().state;
//        qDebug() << "getShowList,state is:" + QString::number(state) ;
        //当记录次数的边界值发生改变时，已经标记为Caution的项是否需要重新加载
        if(state == Allow ||  state == Refuse /*||state == Caution*/ )
            showlist.append(iteratorNS.value());
        else if( state == Noting)
        {
            if(iteratorNS.value().count>=count)
            {
                records[iteratorNS.key()].state= Caution;//状态改变

                changing =true;
                showlist.append(records[iteratorNS.key()]);
            }
        }
        else if(state ==Caution)
        {
            if(iteratorNS.value().count<count)
            {
                records[iteratorNS.key()].state= Noting;//状态改变
                changing =true;
            }
            else
                showlist.append(iteratorNS.value());
        }

    }
    if(changing)
    {
        qDebug()<<"getShowList,record change,resave";
        saveKeyNumberSegments();
    }
    qDebug()<<" NumberSegmentIdentification::getShowList success";
    return showlist;


}

//根据用户的设置，重新调整分段即相应信息
bool NumberSegmentIdentification::setNumberSegmentPropety(qulonglong startNumber,
                                                          qulonglong endNumber,
                                                          IdentificationState command)
{
    qDebug()<<"setNumberSegmentPropety";
    if( startNumber>=endNumber || startNumber<MinPhoneNumber || endNumber>=MaxPhoneNumber)
    {
        qDebug()<<" setNumberSegmentPropety NumberSegment format wrong。";
        return false;
    }
//    QString startNumber = QString::number(startNumber);
//    QString endNumberString = QString::number(endNumber);

    //![0]节点排序
    QList<qulonglong> allStartNumber = records.keys();
    if(!allStartNumber.contains(startNumber)) //某一号段新的起点或结点
        allStartNumber.append(startNumber);
    if(!allStartNumber.contains(endNumber+1)) //某一号段新的起点或结点
        allStartNumber.append(endNumber+1);
    if(!allStartNumber.contains(MaxPhoneNumber)) //增加边界
        allStartNumber.append(MaxPhoneNumber);

    std::list<qulonglong> allStartNumber_stl = allStartNumber.toStdList();
    allStartNumber_stl.sort(); //排序
    allStartNumber = QList<qulonglong>::fromStdList(allStartNumber_stl);

    //两个节点点所在的位置（已经排好序）
    int startNumberIndex = allStartNumber.indexOf(startNumber);
    int endNumberIndex = allStartNumber.indexOf(endNumber+1);

    //![1]首先确定用户设置的号段（startNumber与endNumber之间）
    if(!records.contains(startNumber))
    {
        //新的中间段，startNumber与endNumber为号段
        struct NumberSegmentInformation numberSegmentInformation;
        numberSegmentInformation.startNumber = startNumber;
        numberSegmentInformation.endNumber = endNumber;
        numberSegmentInformation.count = 0;
        numberSegmentInformation.state = command;

        //插入到记录表中
        records.insert(startNumber,numberSegmentInformation);
    }
    else
    {
        records[startNumber].state = command;
        records[startNumber].endNumber = endNumber;
    }
    //重新统计该号段的号码个数
    records[startNumber].Numbers.clear();
    qDebug()<<"startNumber,endNumber:" +QString::number(startNumber) +" ~ " +QString::number(endNumber);

    for(int i =0;i <allNotingNumbers.count();i++)
    {
        if(allNotingNumbers.at(i) >=records[startNumber].startNumber &&
                allNotingNumbers.at(i)<=records[startNumber].endNumber)
        {
            records[startNumber].Numbers.append(allNotingNumbers.at(i));
            qDebug()<<"startNumber~endNumber,add:" +QString::number(allNotingNumbers.at(i));
        }

    }
    records[startNumber].count = records[startNumber].Numbers.count();

     qDebug()<<"after startNumber,endNumber:";
     showresult();
    //![2]确定endNumber+1与后面的号段合并,以endNumber+1为起点，以下一个号段的结点为结点
    if(!records.contains(endNumber+1))
    {
        struct NumberSegmentInformation numberSegmentInformationEnd;
        numberSegmentInformationEnd.startNumber = endNumber+1;
        numberSegmentInformationEnd.endNumber = allStartNumber[endNumberIndex+1]-1; //下一个结点的起始位置-1
        numberSegmentInformationEnd.count = 0;
        if(records.contains(allStartNumber[endNumberIndex+1]))
        {
            numberSegmentInformationEnd.state =records[allStartNumber[endNumberIndex+1]].state;
            //清除重复的记录
            records.remove(allStartNumber[endNumberIndex+1]);
        }
        else
            numberSegmentInformationEnd.state = Noting;

        records.insert(numberSegmentInformationEnd.startNumber,numberSegmentInformationEnd);
        //重新统计该号段的号码个数
        records[endNumber+1].Numbers.clear();
        qDebug()<<"endNumber+1~next:" +
                  QString::number(records[endNumber+1].startNumber) +" ~ " +
                  QString::number(records[endNumber+1].endNumber);
        for(int i =0;i <allNotingNumbers.count();i++)
        {
            if(allNotingNumbers.at(i) >=records[endNumber+1].startNumber &&
                    allNotingNumbers.at(i)<=records[endNumber+1].endNumber)
            {
                records[endNumber+1].Numbers.append(allNotingNumbers.at(i));
                qDebug()<<"endNumber+1~next,add:" +QString::number(allNotingNumbers.at(i));
            }
        }
        records[endNumber+1].count = records[endNumber+1].Numbers.count();
    }

    //![3]确定startNumber与前面的号段合并，以前一个号段的起点为起点，以startNumber-1为结点
    if(startNumberIndex>0)
    {
        qulonglong prestartNumber = allStartNumber[startNumberIndex-1];
        records[prestartNumber].endNumber = startNumber -1;
        //重新统计该号段的号码个数
        records[prestartNumber].Numbers.clear();
        qDebug()<<"pre~startNumber -1:" +
                  QString::number(records[prestartNumber].startNumber) +" ~ " +
                  QString::number(records[prestartNumber].endNumber);
        for(int i =0;i <allNotingNumbers.count();i++)
        {
            if(allNotingNumbers.at(i) >=records[prestartNumber].startNumber &&
                    allNotingNumbers.at(i)<=records[prestartNumber].endNumber)
            {
                records[prestartNumber].Numbers.append(allNotingNumbers.at(i));
                qDebug()<<"pre~startNumber -1,add:" +QString::number(allNotingNumbers.at(i));
            }
        }
        records[prestartNumber].count = records[prestartNumber].Numbers.count();

    }
    //![4]去除startNumber和endNumber号段见多余的记录表
    for(int i = startNumberIndex+1;i<endNumberIndex;i++)
    {
        records.remove(allStartNumber[i]);
    }
    //![5]保存数据
    saveKeyNumberSegments();
    return true;
}

void NumberSegmentIdentification::showresult()
{
    //显示结果
    qDebug()<<"showresult.";
    for(int i=0;i<allNotingNumbers.count();i++)
    {
        qDebug()<<"allNotingNumbers " +QString::number(i) + " is :" +
                  QString::number(allNotingNumbers[i]);
    }
    QMapIterator<qulonglong,NumberSegmentInformation> iterator(records);
    while(iterator.hasNext())
    {
        iterator.next();
        qDebug() << "records start number:" + QString::number(iterator.value().startNumber)+
                    ",end number:" + QString::number(iterator.value().endNumber)+
                     ",state:" + QString::number(iterator.value().state)+
                     ",count:" + QString::number(iterator.value().count);
        for(int i =0;i<iterator.value().Numbers.count();i++)
        {
            qDebug() <<"number "+QString::number(i) + " is  :"+
                       QString::number(iterator.value().Numbers.at(i));


        }
    }
}
//记录用户行为
void NumberSegmentIdentification::NSIProcessAndSave(qulonglong Number)
{
    qDebug()<<"NumberSegmentInformationProcessAndSave.";
    if(allNotingNumbers.contains(Number)) //已经记录
        return;
    else
    {
        allNotingNumbers.append(Number);

        QMapIterator<qulonglong,NumberSegmentInformation> iteratorNS(records);
        while(iteratorNS.hasNext())
        {
            iteratorNS.next();
            if(iteratorNS.value().startNumber <=Number && iteratorNS.value().endNumber >=Number)
            {
                records[iteratorNS.key()].count++;
                records[iteratorNS.key()].Numbers.append(Number);

                if(records[iteratorNS.key()].count >=filterCount)
                {
                    qDebug()<<"filterCount is:" + QString::number(filterCount);
                    if( records[iteratorNS.key()].state == Noting)
                    {
                        records[iteratorNS.key()].state = Caution;
                        emit NewUserhabit(records[iteratorNS.key()]);
                        qDebug()<<"NumberSegmentInformationProcessandSave state change emit.";

                    }
                }

                saveKeyNumberSegments();
                qDebug()<<"NumberSegmentInformationProcessandSave success.";

                return;
            }
        }


    }

}

//判断号码是否符号用户的某种习惯
NumberSegmentInformation NumberSegmentIdentification::numberSegmentIdentification(qulonglong Number)
{
    QMapIterator<qulonglong,NumberSegmentInformation> iterator(records);
    while(iterator.hasNext())
    {
        iterator.next();
        if(iterator.value().startNumber <= Number && Number <= iterator.value().endNumber)
            return records[iterator.key()];
    }
    NumberSegmentInformation Information;
    Information.state = Nonoting;
    return Information;
}
//将数据保存到文件
void NumberSegmentIdentification::saveKeyNumberSegments()
{

    //所有重写，或部分修改
    QFile file(fileNameforNSI);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //         QMessageBox::warning(this,fileNameforMI,"can't open",QMessageBox::Yes);
        qDebug()<< "open fileNameforNSI fail.";
    }
    else
        qDebug()<< "open fileNameforNSI success.";

    QTextStream in(&file);
    QMapIterator<qulonglong,NumberSegmentInformation> iteratorNS( records );

    while(iteratorNS.hasNext())
    {
        iteratorNS.next();
        //添加一行，格式(中间以空格隔开)：起始号码+属性（该号段是否需要统计等）+号段中记录的号码个数n + 号码1+号码2+···号码n +"\n"
        in << QString::number(iteratorNS.value().startNumber)+" "
              +QString::number(iteratorNS.value().state)+ " "
              + QString::number(iteratorNS.value().Numbers.count())+" ";
        for(int i =0;i<iteratorNS.value().Numbers.count();i++)
        {
            in << QString::number(iteratorNS.value().Numbers[i])+" ";
        }
        in << "\n";
    }

    in << QString::number(MaxPhoneNumber)+" "+"0" +" "+"0"+" "+"\n";

    file.close();
    
     qDebug()<< "saveKeyNumberSegments success";

}
