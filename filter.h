#ifndef FILTER_H
#define FILTER_H

#include <QObject>
#include "config.h"
#include "setting.h"
#include "phonebook.h"

class Filter : public QObject
{
    Q_OBJECT
public:
    explicit Filter(QObject *parent = 0);

    Message::Box messageFilter(const QString& senderPhoneNum, const QString &messageContent);

    
signals:
    
public slots:

private:
    bool keyWordsFilter(const QString &messageContent);
    bool whiteListFilter(const QString& senderPhoneNum);
    bool blackListFilter(const QString& senderPhoneNum);
    
};

#endif // FILTER_H
