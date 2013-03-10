#include "numbersegmentidentification.h"
#include <QFile>
#include <QDebug>
#include <QtGlobal>


const quint64 MinPhoneNumber = 1;
const quint64 MaxPhoneNumber = Q_UINT64_C(1000*1000*1000*100); //С��11λʮ������

NumberSegmentIdentification* NumberSegmentIdentification::NSIInstance = NULL;

NumberSegmentIdentification::NumberSegmentIdentification(QObject *parent) :
    QObject(parent),fileNameforNSI("NumberSegmentIdentification.txt")
{
//  quint64 MaxPhoneNumber = Q_UINT64_C(1000*1000*1000*1000-1); //С��11λʮ������
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
        //�������ڵļ�¼��������ʼ����֮�乹��һ�������
        numberSegmentInformation.startNumber = 0; //��ʼΪ0��Ϊ���ж��Ƿ��Ѿ��ռ����1����¼����ʱ��Ϊ0
        struct NumberSegmentInformation numberSegmentInformationTmp;

        while((dataLine=out.readLine()) != NULL)
        {
            //��ʼ����
            int firstPosition = dataLine.indexOf(" ");
            if(firstPosition>0)
            {
                 startNumber = dataLine.mid(0,firstPosition);
                 qDebug()<< "startNumber is:"+startNumber;
                 numberSegmentInformationTmp.startNumber = startNumber.toULongLong();
            }
            else
            {
//                //��ʽ���ԣ�ɾ�����У���������
                qDebug()<<"firstPosition <=0";
//                continue;
                  goto FormatError;
            }
            //״̬
            int secondPosition = dataLine.indexOf(" ",firstPosition+1);
            if(secondPosition>0)
            {
                numberSegmentInformationTmp.state = (IdentificationState)dataLine.mid(firstPosition+1,secondPosition-(firstPosition+1)).toInt();
                qDebug()<< "state is:"+QString::number(numberSegmentInformationTmp.state);
            }
            else
            {
//                //��ʽ���ԣ�ɾ�����У���������
                qDebug()<<"secondPosition <=0";
//                continue;
                  goto FormatError;
            }
            //��¼�ĺ������
            int thirdPosition = dataLine.indexOf(" ",secondPosition+1);
            if(thirdPosition>0)
            {
                numberSegmentInformationTmp.count = dataLine.mid(secondPosition+1,thirdPosition-(secondPosition+1)).toInt();
                qDebug()<< "count is:"+QString::number(numberSegmentInformationTmp.count);
            }
            else
            {
                //��ʽ���ԣ�ɾ�����У���������
                qDebug()<<"thirdPosition <=0";
                goto FormatError;
            }

            //��ȡ��¼�ĺ���
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
                    //��ʽ���ԣ�ɾ�����У���������
                    qDebug()<<"position2 <=0";

                    goto FormatError;
                }
                count--;

            }

            //����εĽ�������
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
            qDebug()<<"NumberSegmentIdentification init ����ʽ����";

        }

        file.close();

    }
    else
    {
         //�ļ�������ʱ�Ĵ�����
        qDebug()<< "PhoneNumberIdentification, fileNameforPNI no exists.";
        createNewNumberSegment();
    }


    //��ʾ���
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
//��ʼ���ŶΣ��Ժ���λ����ͬ��λһ���Ŷ�


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
         //��ʽ����ʼ���� + ���ԣ��úŶ��Ƿ���Ҫͳ�ƣ�+�Ŷ��м�¼�ĺ������n + ����1+����2+����������n +"\n"
         in << QString::number(numberStart)+" "+QString::number(Noting)+ " " + QString::number(0)+" " +  "\n";
         numberStart *= 10;
     }
     in << QString::number(numberStart)+" "+QString::number(Noting)+ " " + QString::number(0)+" " + "\n";

     file.close();
    

}

//���ݼ�¼������Ĭ��Ϊ5�Σ��������Ҫ��ʾ�ڽ���������б�
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
        //����¼�����ı߽�ֵ�����ı�ʱ���Ѿ����ΪCaution�����Ƿ���Ҫ���¼���
        if(state == Allow ||  state == Refuse /*||state == Caution*/ )
            showlist.append(iteratorNS.value());
        else if( state == Noting)
        {
            if(iteratorNS.value().count>=count)
            {
                records[iteratorNS.key()].state= Caution;//״̬�ı�

                changing =true;
                showlist.append(records[iteratorNS.key()]);
            }
        }
        else if(state ==Caution)
        {
            if(iteratorNS.value().count<count)
            {
                records[iteratorNS.key()].state= Noting;//״̬�ı�
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

//�����û������ã����µ����ֶμ���Ӧ��Ϣ
bool NumberSegmentIdentification::setNumberSegmentPropety(qulonglong startNumber,
                                                          qulonglong endNumber,
                                                          IdentificationState command)
{
    qDebug()<<"setNumberSegmentPropety";
    if( startNumber>=endNumber || startNumber<MinPhoneNumber || endNumber>=MaxPhoneNumber)
    {
        qDebug()<<" setNumberSegmentPropety NumberSegment format wrong��";
        return false;
    }
//    QString startNumber = QString::number(startNumber);
//    QString endNumberString = QString::number(endNumber);

    //![0]�ڵ�����
    QList<qulonglong> allStartNumber = records.keys();
    if(!allStartNumber.contains(startNumber)) //ĳһ�Ŷ��µ�������
        allStartNumber.append(startNumber);
    if(!allStartNumber.contains(endNumber+1)) //ĳһ�Ŷ��µ�������
        allStartNumber.append(endNumber+1);
    if(!allStartNumber.contains(MaxPhoneNumber)) //���ӱ߽�
        allStartNumber.append(MaxPhoneNumber);

    std::list<qulonglong> allStartNumber_stl = allStartNumber.toStdList();
    allStartNumber_stl.sort(); //����
    allStartNumber = QList<qulonglong>::fromStdList(allStartNumber_stl);

    //�����ڵ�����ڵ�λ�ã��Ѿ��ź���
    int startNumberIndex = allStartNumber.indexOf(startNumber);
    int endNumberIndex = allStartNumber.indexOf(endNumber+1);

    //![1]����ȷ���û����õĺŶΣ�startNumber��endNumber֮�䣩
    if(!records.contains(startNumber))
    {
        //�µ��м�Σ�startNumber��endNumberΪ�Ŷ�
        struct NumberSegmentInformation numberSegmentInformation;
        numberSegmentInformation.startNumber = startNumber;
        numberSegmentInformation.endNumber = endNumber;
        numberSegmentInformation.count = 0;
        numberSegmentInformation.state = command;

        //���뵽��¼����
        records.insert(startNumber,numberSegmentInformation);
    }
    else
    {
        records[startNumber].state = command;
        records[startNumber].endNumber = endNumber;
    }
    //����ͳ�ƸúŶεĺ������
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
    //![2]ȷ��endNumber+1�����ĺŶκϲ�,��endNumber+1Ϊ��㣬����һ���ŶεĽ��Ϊ���
    if(!records.contains(endNumber+1))
    {
        struct NumberSegmentInformation numberSegmentInformationEnd;
        numberSegmentInformationEnd.startNumber = endNumber+1;
        numberSegmentInformationEnd.endNumber = allStartNumber[endNumberIndex+1]-1; //��һ��������ʼλ��-1
        numberSegmentInformationEnd.count = 0;
        if(records.contains(allStartNumber[endNumberIndex+1]))
        {
            numberSegmentInformationEnd.state =records[allStartNumber[endNumberIndex+1]].state;
            //����ظ��ļ�¼
            records.remove(allStartNumber[endNumberIndex+1]);
        }
        else
            numberSegmentInformationEnd.state = Noting;

        records.insert(numberSegmentInformationEnd.startNumber,numberSegmentInformationEnd);
        //����ͳ�ƸúŶεĺ������
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

    //![3]ȷ��startNumber��ǰ��ĺŶκϲ�����ǰһ���Ŷε����Ϊ��㣬��startNumber-1Ϊ���
    if(startNumberIndex>0)
    {
        qulonglong prestartNumber = allStartNumber[startNumberIndex-1];
        records[prestartNumber].endNumber = startNumber -1;
        //����ͳ�ƸúŶεĺ������
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
    //![4]ȥ��startNumber��endNumber�Ŷμ�����ļ�¼��
    for(int i = startNumberIndex+1;i<endNumberIndex;i++)
    {
        records.remove(allStartNumber[i]);
    }
    //![5]��������
    saveKeyNumberSegments();
    return true;
}

void NumberSegmentIdentification::showresult()
{
    //��ʾ���
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
//��¼�û���Ϊ
void NumberSegmentIdentification::NSIProcessAndSave(qulonglong Number)
{
    qDebug()<<"NumberSegmentInformationProcessAndSave.";
    if(allNotingNumbers.contains(Number)) //�Ѿ���¼
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

//�жϺ����Ƿ�����û���ĳ��ϰ��
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
//�����ݱ��浽�ļ�
void NumberSegmentIdentification::saveKeyNumberSegments()
{

    //������д���򲿷��޸�
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
        //���һ�У���ʽ(�м��Կո����)����ʼ����+���ԣ��úŶ��Ƿ���Ҫͳ�Ƶȣ�+�Ŷ��м�¼�ĺ������n + ����1+����2+����������n +"\n"
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
