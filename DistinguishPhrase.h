#ifndef DistinguishPhrase_H
#define DistinguishPhrase_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QStringList>

//////////////////////////////////////////////////////////////////////////
//标注集
//////////////////////////////////////////////////////////////////////////
#define ICT_POS_MAP_SECOND 0 //计算所二级标注集
#define ICT_POS_MAP_FIRST 1  //计算所一级标注集
#define PKU_POS_MAP_SECOND 2 //北大二级标注集
#define PKU_POS_MAP_FIRST 3	//北大一级标注集
#define POS_MAP_NUMBER 4 //标注集 数量
#define  POS_SIZE 8 // 词性标记最大字节数

/////////////////////////////////////////////////////////////////////////
// 字符编码类型
//////////////////////////////////////////////////////////////////////////
enum eCodeType {
    CODE_TYPE_UNKNOWN,//type unknown
    CODE_TYPE_ASCII,//ASCII
    CODE_TYPE_GB,//GB2312,GBK,GB10380
    CODE_TYPE_UTF8,//UTF-8
    CODE_TYPE_BIG5//BIG5
};

//////////////////////////////////////////////////////////////////////////
//字符串结果
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
struct tagICTCLAS_Result{
  int iStartPos; //开始位置
  int iLength; //长度
  char szPOS[POS_SIZE];//词性
  int	iPOS; //词性ID
  int iWordID; //词ID
  int iWordType; //词语类型，用户词汇？(0-否,1-是)
  int iWeight;// 词语权重
 };
#pragma pack()
typedef tagICTCLAS_Result* LPICTCLAS_RESULT;
// 对应ICTCLAS50.dll中用于分词的函数, 从DLL中得到的
/************************************************
功能：
    ①函数指针，用于指向ICTCLAS50.dll中的接口bool ICTCLAS_Init(const char* pszInitDir=NULL);
    ②初始化系统词典和配置信息
参数：
    pszInitDir：①初始化路径，应包含配置文件（Configure.xml）和词典目录(Data目录)以及授权文件(user.lic).
                ②如果这些文件及目录在系统运行当前目录下，此参数可以为null。
返回值:
    如果初始化成功返回true, 否则返回false
************************************************/
typedef bool (*QICTCLAS_Init)(const char* pszInitDir);

/************************************************
功能：
    ①函数指针，用于指向ICTCLAS50.dll中的接口int ICTCLAS_SetPOSmap(int nPOSmap);;
    ②设置词性标注集
参数：
    nPOSmap：①ICT_POS_MAP_FIRST  计算所一级标注集
             ② ICT_POS_MAP_SECOND  计算所二级标注集
             ③PKU_POS_MAP_SECOND   北大二级标注集
             ④PKU_POS_MAP_FIRST 	  北大一级标注集

返回值:
    如果设置成功返回true, 否则返回false
注：该函数只有在ICTCLAS_Init成功后才能正常工作
************************************************/
typedef bool (*QICTCLAS_SetPOSmap)(int nPOSmap);

/************************************************
功能：
    ①函数指针，用于指向ICTCLAS50.dll中的接口
    LPICTCLAS_RESULT  ICTCLAS_ParagraphProcessA(
                     const char*  pszText,
                     int  iLength,
                     int  &nResultCount, //[out]
                     eCodeType	codeType=CODE_TYPE_UNKNOWN,
                     bool	  bEnablePOS=false
    );
    ②对一段字符进行分词处理，返回结果为字符串结构数组
参数：
* Parameter:  const char * pszText<! 需要分词的文本内容>
* Parameter:  int iLength<! 需要分词的文本长度>
* Parameter:  int & nResultCount [out]<! 结果数组长度>
* Parameter:  e_CodeType codeType<! 字符编码类型>
* Parameter:  int bEnablePOS<! 是否词性标注>
返回值:
    t_pRstVec<! 结果数组>
注：①该函数只有在ICTCLAS_Init成功后才能正常工作
    ②调用此接口后，应调用ICTCLAS_ResultFree() 释放相关内存
************************************************/
typedef LPICTCLAS_RESULT (*QICTCLAS_ParagraphProcessA)(
        const char*  pszText,
        int  iLength,
        int  &nResultCount, //[out]
        eCodeType	codeType,
        bool	  bEnablePOS
    );

/************************************************
功能：
    ①函数指针，用于指向ICTCLAS50.dll中的接口
       int ICTCLAS_ParagraphProcessICTCLAS_ParagraphProcess(
                                                 const char*  pszText,
                                                 int	 iLength,
                                                 char*	pszResult, //[out]
                                                 eCodeType	codeType=CODE_TYPE_UNKNOWN,
                                                 bool	 bEnablePOS=false  );
    ②对一段字符进行分词处理，并返回处理结果
参数：
* Parameter:  const char * pszText<!需要分词的文本内容>
* Parameter:  int iLength<!需要分词的文本长度>
* Parameter:  char*	pszResult [out]<!分词结果字符串>
* Parameter:  e_CodeType codeType<!字符编码类型>
* Parameter:  int bEnablePOS<！是否词性标注 >
返回值:
    int<! 结果字符串长度>
注：该函数只有在ICTCLAS_Init成功后才能正常工作
************************************************/
typedef int (*QICTCLAS_ParagraphProcess)(
    const char*  pszText,
    int			    iLength,
    char*		    pszResult, //[out]
    eCodeType	codeType,
    bool		        bEnablePOS
    );
/************************************************
功能：
    ①函数指针，用于指向ICTCLAS50.dll中的接口
       bool ICTCLAS_ResultFree(LPICTCLAS_RESULT pRetVec)
    ②释放分词结果所占的内存
参数：
* Parameter:   t_pRstVec pRetVec<!要释放的结果数组 >
返回值:
    bool<! 释放成功与否>
注：本接口用于释放 ICTCLAS_ParagraphProcessA 生成的结果内存
************************************************/
typedef int (*QICTCLAS_ResultFree)(LPICTCLAS_RESULT pRetVec);

/************************************************
功能：
    ①函数指针，用于指向ICTCLAS50.dll中的接口bool ICTCLAS_Exit();
    ②退出，释放相关资源
参数：
    无
返回值:
    如果退出成功返回true, 否则返回false
注:	所有操作完成后，请调用本接口释放相关资源！
************************************************/
typedef bool (*QICTCLAS_Exit)();

class DistinguishPhrase : public QObject
{
    Q_OBJECT
public:
    explicit DistinguishPhrase(QObject *parent = 0);
    QMap<QString,int> DistinguishPhraseProcessAndFilter(QString data);
    bool addCommonWord(QString word);

signals:
    
public slots:

private:
    QICTCLAS_Init pICTCLAS_Init;
    QICTCLAS_SetPOSmap pICTCLAS_SetPOSmap;
    QICTCLAS_ParagraphProcess pICTCLAS_ParagraphProcess;
    QICTCLAS_ParagraphProcessA pICTCLAS_ParagraphProcessA;
    QICTCLAS_ResultFree pICTCLAS_ResultFree;
    QICTCLAS_Exit pICTCLAS_Exit;

    void init();
    void createCommonWords();
//    bool CommonWordPropertFilter(int iPOS);
    bool CommonWordPropertFilter(QString pos);
    bool CommonWordFilter(QString pos);

    QStringList filterPOS; //记录需要过滤的词性(字符表示)
    QStringList filterWord; //记录需要过滤的常用词
    const QString fileNameforCommonWords ; //保存常用词文件的路径

    
};

#endif // DistinguishPhrase_H
