#ifndef PHONENUMBERIDENTIFICATION_H
#define PHONENUMBERIDENTIFICATION_H



#include <QObject>
#include <QMap>
#include <QDebug>
#include <QFile>
#include "config.h"

#define g_PhoneNumberIdentification PhoneNumberIdentification::instance()

class PhoneNumberIdentification : public QObject
{
    Q_OBJECT
public:
    explicit PhoneNumberIdentification(QObject *parent = 0);
    static PhoneNumberIdentification * instance();

    void setNumberPropety(const QString Number,IdentificationState command);

    void NumberIdentificationProcessAndSave(QString Number);

    PNIInformation numberIdentification(QString Number);

    QList<PNIInformation> getShowList(int count = 4);

signals:
     void NewUserNumberhabit(PNIInformation information);
    
public slots:

private:
    QMap<QString ,int> notedNumbers;     //��Ҫ��¼�ĺ��룬ͳ���û�����������뵽���������
    QMap<QString ,int> noNotedNumbers;   //�û���ǲ���Ҫͳ�ƣ������˵ĺ���

    QMap<QString ,PNIInformation> numberRecords;

    int filterCount ;

    const QString fileNameforPNI ;
    static PhoneNumberIdentification* phoneNumberIdentification;
    void init();
    void saveKeyNumbers();
};

#endif // PHONENUMBERIDENTIFICATION_H
