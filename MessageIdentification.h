#ifndef MessageIdentification_H
#define MessageIdentification_H

#include <QObject>
#include "DistinguishPhrase.h"
#include <QMap>
#include "config.h"

#define g_MessageIdentification MessageIdentification::instance()

class MessageIdentification : public QObject
{
    Q_OBJECT
public:
    explicit MessageIdentification(QObject *parent = 0);
    static MessageIdentification* instance();


    void MessageIdentificationProcessAndSave(QString &data);
    void setWordPropety(const QString data,IdentificationState command);
    QList<MIInformation> getShowList(int count = 8);
    bool addCommonWord(QString word);
    MIInformation messageIdentification(QString data);

signals:
    void NewUserMessagehabit(MIInformation information);
    
public slots:

private :
    DistinguishPhrase distinguishPhrase; //分词工具
    const QString fileNameforMI ;
    int filterCount;
    QMap<QString,MIInformation> messageRecords;
    QMap<QString,MIInformation> filterRecords; //用户已经形成习惯的记录

    static MessageIdentification* MIInstance;
    void Init();
    void saveKeywords();

    
};

#endif // MessageIdentification_H
