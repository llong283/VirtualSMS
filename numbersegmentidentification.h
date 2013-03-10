#ifndef NUMBERSEGMENTIDENTIFICATION_H
#define NUMBERSEGMENTIDENTIFICATION_H

#include <QObject>
#include <QList>
#include <QMap>
#include "config.h"

#define g_NumberSegmentIdentification NumberSegmentIdentification::instance()

//注：号码为最多11位十进制的整数
class NumberSegmentIdentification : public QObject
{
    Q_OBJECT
public:
    explicit NumberSegmentIdentification(QObject *parent = 0);
    static NumberSegmentIdentification* instance();

    QList<NumberSegmentInformation> getShowList(int count = 6);

//    void setNumberSegmentPropety(NumberSegmentInformation information);
    bool setNumberSegmentPropety(qulonglong startNumber,qulonglong endNumber,IdentificationState command);

    void NSIProcessAndSave(qulonglong Number);
    NumberSegmentInformation numberSegmentIdentification(qulonglong Number);
    void showresult();


signals:
    void NewUserhabit(NumberSegmentInformation information);
    
public slots:

private:
    static NumberSegmentIdentification* NSIInstance;
    const QString fileNameforNSI ;
    int filterCount;
    void init();
    void createNewNumberSegment();
    void saveKeyNumberSegments();

    QList<qulonglong> allNotingNumbers;
    QMap<qulonglong,NumberSegmentInformation> records;
};

#endif // NUMBERSEGMENTIDENTIFICATION_H
