#ifndef DistinguishPhrase_H
#define DistinguishPhrase_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QStringList>

//////////////////////////////////////////////////////////////////////////
//��ע��
//////////////////////////////////////////////////////////////////////////
#define ICT_POS_MAP_SECOND 0 //������������ע��
#define ICT_POS_MAP_FIRST 1  //������һ����ע��
#define PKU_POS_MAP_SECOND 2 //���������ע��
#define PKU_POS_MAP_FIRST 3	//����һ����ע��
#define POS_MAP_NUMBER 4 //��ע�� ����
#define  POS_SIZE 8 // ���Ա������ֽ���

/////////////////////////////////////////////////////////////////////////
// �ַ���������
//////////////////////////////////////////////////////////////////////////
enum eCodeType {
    CODE_TYPE_UNKNOWN,//type unknown
    CODE_TYPE_ASCII,//ASCII
    CODE_TYPE_GB,//GB2312,GBK,GB10380
    CODE_TYPE_UTF8,//UTF-8
    CODE_TYPE_BIG5//BIG5
};

//////////////////////////////////////////////////////////////////////////
//�ַ������
//////////////////////////////////////////////////////////////////////////
#pragma pack(1)
struct tagICTCLAS_Result{
  int iStartPos; //��ʼλ��
  int iLength; //����
  char szPOS[POS_SIZE];//����
  int	iPOS; //����ID
  int iWordID; //��ID
  int iWordType; //�������ͣ��û��ʻ㣿(0-��,1-��)
  int iWeight;// ����Ȩ��
 };
#pragma pack()
typedef tagICTCLAS_Result* LPICTCLAS_RESULT;
// ��ӦICTCLAS50.dll�����ڷִʵĺ���, ��DLL�еõ���
/************************************************
���ܣ�
    �ٺ���ָ�룬����ָ��ICTCLAS50.dll�еĽӿ�bool ICTCLAS_Init(const char* pszInitDir=NULL);
    �ڳ�ʼ��ϵͳ�ʵ��������Ϣ
������
    pszInitDir���ٳ�ʼ��·����Ӧ���������ļ���Configure.xml���ʹʵ�Ŀ¼(DataĿ¼)�Լ���Ȩ�ļ�(user.lic).
                �������Щ�ļ���Ŀ¼��ϵͳ���е�ǰĿ¼�£��˲�������Ϊnull��
����ֵ:
    �����ʼ���ɹ�����true, ���򷵻�false
************************************************/
typedef bool (*QICTCLAS_Init)(const char* pszInitDir);

/************************************************
���ܣ�
    �ٺ���ָ�룬����ָ��ICTCLAS50.dll�еĽӿ�int ICTCLAS_SetPOSmap(int nPOSmap);;
    �����ô��Ա�ע��
������
    nPOSmap����ICT_POS_MAP_FIRST  ������һ����ע��
             �� ICT_POS_MAP_SECOND  ������������ע��
             ��PKU_POS_MAP_SECOND   ���������ע��
             ��PKU_POS_MAP_FIRST 	  ����һ����ע��

����ֵ:
    ������óɹ�����true, ���򷵻�false
ע���ú���ֻ����ICTCLAS_Init�ɹ��������������
************************************************/
typedef bool (*QICTCLAS_SetPOSmap)(int nPOSmap);

/************************************************
���ܣ�
    �ٺ���ָ�룬����ָ��ICTCLAS50.dll�еĽӿ�
    LPICTCLAS_RESULT  ICTCLAS_ParagraphProcessA(
                     const char*  pszText,
                     int  iLength,
                     int  &nResultCount, //[out]
                     eCodeType	codeType=CODE_TYPE_UNKNOWN,
                     bool	  bEnablePOS=false
    );
    �ڶ�һ���ַ����зִʴ������ؽ��Ϊ�ַ����ṹ����
������
* Parameter:  const char * pszText<! ��Ҫ�ִʵ��ı�����>
* Parameter:  int iLength<! ��Ҫ�ִʵ��ı�����>
* Parameter:  int & nResultCount [out]<! ������鳤��>
* Parameter:  e_CodeType codeType<! �ַ���������>
* Parameter:  int bEnablePOS<! �Ƿ���Ա�ע>
����ֵ:
    t_pRstVec<! �������>
ע���ٸú���ֻ����ICTCLAS_Init�ɹ��������������
    �ڵ��ô˽ӿں�Ӧ����ICTCLAS_ResultFree() �ͷ�����ڴ�
************************************************/
typedef LPICTCLAS_RESULT (*QICTCLAS_ParagraphProcessA)(
        const char*  pszText,
        int  iLength,
        int  &nResultCount, //[out]
        eCodeType	codeType,
        bool	  bEnablePOS
    );

/************************************************
���ܣ�
    �ٺ���ָ�룬����ָ��ICTCLAS50.dll�еĽӿ�
       int ICTCLAS_ParagraphProcessICTCLAS_ParagraphProcess(
                                                 const char*  pszText,
                                                 int	 iLength,
                                                 char*	pszResult, //[out]
                                                 eCodeType	codeType=CODE_TYPE_UNKNOWN,
                                                 bool	 bEnablePOS=false  );
    �ڶ�һ���ַ����зִʴ��������ش�����
������
* Parameter:  const char * pszText<!��Ҫ�ִʵ��ı�����>
* Parameter:  int iLength<!��Ҫ�ִʵ��ı�����>
* Parameter:  char*	pszResult [out]<!�ִʽ���ַ���>
* Parameter:  e_CodeType codeType<!�ַ���������>
* Parameter:  int bEnablePOS<���Ƿ���Ա�ע >
����ֵ:
    int<! ����ַ�������>
ע���ú���ֻ����ICTCLAS_Init�ɹ��������������
************************************************/
typedef int (*QICTCLAS_ParagraphProcess)(
    const char*  pszText,
    int			    iLength,
    char*		    pszResult, //[out]
    eCodeType	codeType,
    bool		        bEnablePOS
    );
/************************************************
���ܣ�
    �ٺ���ָ�룬����ָ��ICTCLAS50.dll�еĽӿ�
       bool ICTCLAS_ResultFree(LPICTCLAS_RESULT pRetVec)
    ���ͷŷִʽ����ռ���ڴ�
������
* Parameter:   t_pRstVec pRetVec<!Ҫ�ͷŵĽ������ >
����ֵ:
    bool<! �ͷųɹ����>
ע�����ӿ������ͷ� ICTCLAS_ParagraphProcessA ���ɵĽ���ڴ�
************************************************/
typedef int (*QICTCLAS_ResultFree)(LPICTCLAS_RESULT pRetVec);

/************************************************
���ܣ�
    �ٺ���ָ�룬����ָ��ICTCLAS50.dll�еĽӿ�bool ICTCLAS_Exit();
    ���˳����ͷ������Դ
������
    ��
����ֵ:
    ����˳��ɹ�����true, ���򷵻�false
ע:	���в�����ɺ�����ñ��ӿ��ͷ������Դ��
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

    QStringList filterPOS; //��¼��Ҫ���˵Ĵ���(�ַ���ʾ)
    QStringList filterWord; //��¼��Ҫ���˵ĳ��ô�
    const QString fileNameforCommonWords ; //���泣�ô��ļ���·��

    
};

#endif // DistinguishPhrase_H
