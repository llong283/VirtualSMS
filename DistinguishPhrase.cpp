#include "DistinguishPhrase.h"
#include <QLibrary>
#include <QDebug>
#include <QTextCodec>
#include <QByteArray>
#include <QMap>
#include <QFile>
#include <QStringList>

DistinguishPhrase::DistinguishPhrase(QObject *parent) :
    QObject(parent),fileNameforCommonWords("commonwords.txt")
{    
    init();
}

void DistinguishPhrase::init()
{
    //���طִ����
    QLibrary lib("ICTCLAS50.dll");
    if(lib.load())
    {
        pICTCLAS_Init = (QICTCLAS_Init)lib.resolve("ICTCLAS_Init");
        pICTCLAS_SetPOSmap = (QICTCLAS_SetPOSmap)lib.resolve("ICTCLAS_SetPOSmap");
        pICTCLAS_ParagraphProcess = (QICTCLAS_ParagraphProcess)lib.resolve("ICTCLAS_ParagraphProcess");
        pICTCLAS_ParagraphProcessA = (QICTCLAS_ParagraphProcessA)lib.resolve("ICTCLAS_ParagraphProcessA");
        pICTCLAS_ResultFree = (QICTCLAS_ResultFree)lib.resolve("ICTCLAS_ResultFree");
        pICTCLAS_Exit = (QICTCLAS_Exit)lib.resolve("ICTCLAS_Exit");
    }
    else
    {
        pICTCLAS_Init = NULL;
        qDebug()<< "ICTCLAS50.dllload fail.";
        return;
    }

    //�ռ���Ҫ���˵Ĵ��Ա�ʶ��
    filterPOS << "r"<<"rr"<<"rz" <<"rzt"<<"rzs"<<"rzv"<<"ry"<<"ryt"<<"rys"<<"ryv"<<"rg";//����
    filterPOS<<"w"<<"wkz"<<"wky"<<"wyz"<<"wyy"<<"wj"<<"ww"<<"wt"<<"wd"<<"wf"<<"wn"<<"wm"<<"ws"
            <<"wp"<<"wb"<<"wh";//������
    filterPOS <<"p"<<"pba"<<"pbei";//���
    filterPOS <<"vshi"<<"vyou"/*<<"vi"*/;//����
    filterPOS <<"u"<<"uzhe "<<"ule"<<"uguo"<<"ude1"<<"ude2"<<"ude3"<<"usuo"<<"udeng"<<"uyy"
             <<"udh"<<"uls"<<"uzhi"<<"ulian";//����
    filterPOS <<"f"; //��λ��
    filterPOS <<"b" <<"bl"; //�����
    filterPOS <<"y"; //������
    filterPOS <<"d"; //����
    filterPOS <<"m"<<"mq";//����
    filterPOS <<"q";//����
    filterPOS <<"c"<<"cc";//����

    //�ռ���Ҫ���˵ĳ��ô�
    filterWord.clear();
    if(!QFile::exists(fileNameforCommonWords))
    {
        qDebug()<< "DistinguishPhrase, fileNameforCommonWords no exists.";
        createCommonWords();
        return;
    }
    else
    {
        QFile file(fileNameforCommonWords);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug()<< "PhoneNumberIdentification,open fileNameforPNI fail.";
        }
        QTextStream out(&file);
        QString filedata = out.readAll();
        filterWord = filedata.split(",", QString::SkipEmptyParts);
        file.close();

    }


}

void DistinguishPhrase::createCommonWords()
{
    qDebug()<< "DistinguishPhrase::createCommonWords.";
    filterWord <<"��"<<"Ϊ"<<"��"<<"��"<<"��"<<"��"<<"��"<<"˵"<<"��"<<"��";//�������Ĵ�
    filterWord <<"the"<<"for"<<"in"<<"to"<<"on"<<"about"<<"above"<<"after"<<"again"
              <<"all"<<"also"<<"am"<<"an"<<"and"<<"any"<<"are"<<"as"<<"at"<<"back"
             <<"be"<<"been"<<"before"<<"behind"<<"being"<<"below"<<"but" <<"by"
            <<"can"<<"click"<<"do"<<"does"<<"done"<<"each"<<"else"<<"etc"<<"ever"
           <<"every"<<"few"<<"for"<<"from"<<"generally"<<"get"<<"go"<<"gone"<<"has"
          <<"have"<<"hello"<<"here"<<"how"<<"if"<<"in"<<"into"<<"is"<<"just"<<"keep"
         <<"later"<<"let"<<"like"<<"lot"<<"lots"<<"made"<<"make"<<"makes"<<"many"<<"may"
        <<"me"<<"more"<<"most"<<"much"<<"must"<<"my"<<"need"<<"no"<<"not"<<"now"<<"of"
       <<"often"<<"on"<<"only"<<"or"<<"other"<<"others"<<"our"<<"out"<<"over"<<"please"
      <<"put"<<"so"<<"some"<<"such"<<"than"<<"that"<<"the"<<"their"<<"them"<<"then"<<"there"
     <<"these"<<"they"<<"this"<<"try"<<"to"<<"up"<<"us"<<"very"<<"want"<<"was"<<"we"<<"well"
    <<"what"<<"when"<<"where"<<"which"<<"why"<<"will"<<"with"<<"within"<<"you"
    <<"your"<<"yourself";//����Ӣ�Ĵ�

    filterWord<<"a"<<"b"<<"c"<<"d"<<"e"<<"f"<<"g"<<
                "h"<<"i"<<"j"<<"k"<<"l"<<"m"<<"n"<<
                "o"<<"p"<<"q"<<"r"<<"s"<<"t"<<
                "u"<<"v"<<"w"<<"x"<<"y"<<"z";//Ӣ����ĸ


    //������д���򲿷��޸�
    QString filedata = filterWord.join(",");
    QFile file(fileNameforCommonWords);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        //         QMessageBox::warning(this,fileNameforMI,"can't open",QMessageBox::Yes);
        qDebug()<< "open fileNameforCommonWords fail.";
    }
    else
        qDebug()<< "open fileNameforCommonWords success.";

    QTextStream in(&file);
    in << filedata;
    file.close();
}

/************************************************
���ܣ�
    ������ʱ��浽���ô���
������
    word: �����
����ֵ:
    ��
************************************************/

bool DistinguishPhrase::addCommonWord(QString word)
{
    if(filterWord.contains(word))
    {        
        qDebug()<< "word exist";
        return false;
    }
    else
    {
        filterWord<<word;

        QFile file(fileNameforCommonWords);

        if(!file.open(QIODevice::Append | QIODevice::Text))
        {
            //         QMessageBox::warning(this,fileNameforMI,"can't open",QMessageBox::Yes);
            qDebug()<< "open fileNameforCommonWords fail.";
        }
        else
            qDebug()<< "open fileNameforCommonWords success.";

        QTextStream in(&file);
        in << ","+word;
        file.close();
        return true;
    }
}

 /************************************************
 ���ܣ�
     ���������ݽ��зִʣ�����һ�����ˣ�����ȡ�ֶεĹؼ���
 ������
     data: ��Ҫ������ֶ�
 ����ֵ:
     ��������Ĺؼ����б�
     QMap<
         QString,  //�ؼ���
         int  //�ʳ��ֲŴ���
      >
 ************************************************/
 QMap<QString,int> DistinguishPhrase::DistinguishPhraseProcessAndFilter(QString data)
 {
     QMap<QString,int> keyword;
     QMap<QString,int> filteredword;

     if(pICTCLAS_Init == NULL)  //�ִʿ����ʧ�ܣ�����ʹ��
     {
         qDebug()<<"pICTCLAS_Init == NULL ,can't DistinguishPhrase";
         return keyword;
     }
     //�ִ�![0],��ʼ���ִ����
     if(!pICTCLAS_Init(NULL))
     {
         qDebug()  << "Init fails\n";
         return keyword;
      }
     else
     {
        qDebug() << "Init ok\n";
     }
     //�ִ�![0]
     //�ִ�![1],���ô��Ա�ע��(0 ������������ע����1 ������һ����ע����2 ���������ע����3 ����һ����ע��)
     pICTCLAS_SetPOSmap(ICT_POS_MAP_SECOND);
     //�ִ�![1]
     //�ִ�![2]���ִ�
     QByteArray dataByteArray = QTextCodec::codecForName("GBK")->fromUnicode(data);
     char *sSentence = dataByteArray.data();
     unsigned int nPaLen = strlen(sSentence);
     int nRstLen=0; //�ִʽ���ĳ���
     LPICTCLAS_RESULT rstVec = pICTCLAS_ParagraphProcessA(sSentence,nPaLen,nRstLen,CODE_TYPE_UNKNOWN,1);
     //�ִ�![2]
     //�ִ�![3]�����ôʹ���
     for (int i=0;i<nRstLen;i++)
     {//��ӡ�ִʽ��
         //         qDebug("start=%d,length=%d\r\n",rstVec[i].iStartPos,rstVec[i].iLength);
         //         QByteArray  array(sRst+rstVec[i].iStartPos,rstVec[i].iLength);
         QString word = QTextCodec::codecForName("GBK")->toUnicode(sSentence+rstVec[i].iStartPos,
                                                                   rstVec[i].iLength);
         qDebug() << "word is :" + word;
         //         qDebug("word ���� is %s������ID: %d,��ID:%d,����Ȩ��:%d",
         //                rstVec[i].szPOS,rstVec[i].iPOS,rstVec[i].iWordID,rstVec[i].iWeight);
         qDebug("word ���� is %s������ID: %d,��ID:%d,����Ȩ��:%d",
                rstVec[i].szPOS,rstVec[i].iPOS,rstVec[i].iWordID,rstVec[i].iWeight);


         if(CommonWordFilter(word.toLower()) || CommonWordPropertFilter(QString( rstVec[i].szPOS )))
         {
             if(filteredword.contains(word))
             {
                 filteredword[word]++;
             }
             else
             {
                 filteredword.insert(word,1);
             }
         }
         else
          {
             if(keyword.contains(word))
             {
                 keyword[word]++;
             }
             else
             {
                 keyword.insert(word,1);
             }
         }

     }
     //�ִ�![3]
     //�ִ�![4],�ͷŷִʽ����ռ���ڴ�
     pICTCLAS_ResultFree(rstVec);
     //�ִ�![4]
     //�ִ�![5]�˳��ִʣ��ͷ������Դ
     pICTCLAS_Exit();
     //�ִ�![5]

     //��ʾ�����
     qDebug()<< "the last result.";
     QMapIterator<QString,int> iterator(keyword);
     while(iterator.hasNext())
     {
         iterator.next();
         qDebug() <<"keyword is:" +iterator.key()+"; weight is:" +QString::number(iterator.value());

     }
     QMapIterator<QString,int> iteratorF(filteredword);
     while(iteratorF.hasNext())
     {
         iteratorF.next();
         qDebug() <<"filteredword is:" +iteratorF.key()+"; weight is:" +QString::number(iteratorF.value());

     }

     return keyword;

 }

 /************************************************
 ���ܣ�
     ���ݴ���ID���ж��Ƿ���Ҫ����
 ������
     iPOS: ����ID
 ����ֵ:
     ��Ҫ���˷���true�����򷵻�false
 ************************************************/

//bool DistinguishPhrase::CommonWordPropertFilter(int iPOS)
//{
//   return filterPOSID.contains(iPOS);
//}
/************************************************
���ܣ�
    ���ݴ��Ա�ʶ�����ж��Ƿ���Ҫ����
������
    pos: ���Ա�ʶ��
����ֵ:
    ��Ҫ���˷���true�����򷵻�false
************************************************/
bool DistinguishPhrase::CommonWordPropertFilter(QString pos)
{
   return filterPOS.contains(pos);
}

/************************************************
���ܣ�
    ���ݴʷ����ж��Ƿ���Ҫ����
������
    word: �ʷ�
����ֵ:
    ��Ҫ���˷���true�����򷵻�false
************************************************/
bool DistinguishPhrase::CommonWordFilter(QString word)
{
   return filterWord.contains(word);
}

