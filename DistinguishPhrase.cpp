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
    //加载分词组件
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

    //收集需要过滤的词性标识符
    filterPOS << "r"<<"rr"<<"rz" <<"rzt"<<"rzs"<<"rzv"<<"ry"<<"ryt"<<"rys"<<"ryv"<<"rg";//代词
    filterPOS<<"w"<<"wkz"<<"wky"<<"wyz"<<"wyy"<<"wj"<<"ww"<<"wt"<<"wd"<<"wf"<<"wn"<<"wm"<<"ws"
            <<"wp"<<"wb"<<"wh";//标点符号
    filterPOS <<"p"<<"pba"<<"pbei";//介词
    filterPOS <<"vshi"<<"vyou"/*<<"vi"*/;//动词
    filterPOS <<"u"<<"uzhe "<<"ule"<<"uguo"<<"ude1"<<"ude2"<<"ude3"<<"usuo"<<"udeng"<<"uyy"
             <<"udh"<<"uls"<<"uzhi"<<"ulian";//助词
    filterPOS <<"f"; //方位词
    filterPOS <<"b" <<"bl"; //区别词
    filterPOS <<"y"; //语气词
    filterPOS <<"d"; //副词
    filterPOS <<"m"<<"mq";//数词
    filterPOS <<"q";//数词
    filterPOS <<"c"<<"cc";//连词

    //收集需要过滤的常用词
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
    filterWord <<"在"<<"为"<<"最"<<"会"<<"将"<<"于"<<"有"<<"说"<<"能"<<"可";//常用中文词
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
    <<"your"<<"yourself";//常用英文词

    filterWord<<"a"<<"b"<<"c"<<"d"<<"e"<<"f"<<"g"<<
                "h"<<"i"<<"j"<<"k"<<"l"<<"m"<<"n"<<
                "o"<<"p"<<"q"<<"r"<<"s"<<"t"<<
                "u"<<"v"<<"w"<<"x"<<"y"<<"z";//英文字母


    //所有重写，或部分修改
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
功能：
    将输入词保存到常用词中
参数：
    word: 输入词
返回值:
    无
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
 功能：
     对输入数据进行分词，并做一定过滤，即提取字段的关键词
 参数：
     data: 需要处理的字段
 返回值:
     返回所需的关键词列表
     QMap<
         QString,  //关键词
         int  //词出现才次数
      >
 ************************************************/
 QMap<QString,int> DistinguishPhrase::DistinguishPhraseProcessAndFilter(QString data)
 {
     QMap<QString,int> keyword;
     QMap<QString,int> filteredword;

     if(pICTCLAS_Init == NULL)  //分词库加载失败，不能使用
     {
         qDebug()<<"pICTCLAS_Init == NULL ,can't DistinguishPhrase";
         return keyword;
     }
     //分词![0],初始化分词组件
     if(!pICTCLAS_Init(NULL))
     {
         qDebug()  << "Init fails\n";
         return keyword;
      }
     else
     {
        qDebug() << "Init ok\n";
     }
     //分词![0]
     //分词![1],设置词性标注集(0 计算所二级标注集，1 计算所一级标注集，2 北大二级标注集，3 北大一级标注集)
     pICTCLAS_SetPOSmap(ICT_POS_MAP_SECOND);
     //分词![1]
     //分词![2]，分词
     QByteArray dataByteArray = QTextCodec::codecForName("GBK")->fromUnicode(data);
     char *sSentence = dataByteArray.data();
     unsigned int nPaLen = strlen(sSentence);
     int nRstLen=0; //分词结果的长度
     LPICTCLAS_RESULT rstVec = pICTCLAS_ParagraphProcessA(sSentence,nPaLen,nRstLen,CODE_TYPE_UNKNOWN,1);
     //分词![2]
     //分词![3]，常用词过滤
     for (int i=0;i<nRstLen;i++)
     {//打印分词结果
         //         qDebug("start=%d,length=%d\r\n",rstVec[i].iStartPos,rstVec[i].iLength);
         //         QByteArray  array(sRst+rstVec[i].iStartPos,rstVec[i].iLength);
         QString word = QTextCodec::codecForName("GBK")->toUnicode(sSentence+rstVec[i].iStartPos,
                                                                   rstVec[i].iLength);
         qDebug() << "word is :" + word;
         //         qDebug("word 词性 is %s，词性ID: %d,词ID:%d,词语权重:%d",
         //                rstVec[i].szPOS,rstVec[i].iPOS,rstVec[i].iWordID,rstVec[i].iWeight);
         qDebug("word 词性 is %s，词性ID: %d,词ID:%d,词语权重:%d",
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
     //分词![3]
     //分词![4],释放分词结果所占的内存
     pICTCLAS_ResultFree(rstVec);
     //分词![4]
     //分词![5]退出分词，释放相关资源
     pICTCLAS_Exit();
     //分词![5]

     //显示最后结果
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
 功能：
     根据词性ID，判断是否需要过滤
 参数：
     iPOS: 词性ID
 返回值:
     需要过滤返回true，否则返回false
 ************************************************/

//bool DistinguishPhrase::CommonWordPropertFilter(int iPOS)
//{
//   return filterPOSID.contains(iPOS);
//}
/************************************************
功能：
    根据词性标识符，判断是否需要过滤
参数：
    pos: 词性标识符
返回值:
    需要过滤返回true，否则返回false
************************************************/
bool DistinguishPhrase::CommonWordPropertFilter(QString pos)
{
   return filterPOS.contains(pos);
}

/************************************************
功能：
    根据词符，判断是否需要过滤
参数：
    word: 词符
返回值:
    需要过滤返回true，否则返回false
************************************************/
bool DistinguishPhrase::CommonWordFilter(QString word)
{
   return filterWord.contains(word);
}

