#ifndef MSGTOOL_H
#define MSGTOOL_H

#include "config.h"
#include "abstractmsg.h"

class MsgTool : public AbstractMsg
{
    Q_OBJECT
public:
    explicit MsgTool(QObject *parent = 0);
    
    void importMsg();
    void exportMsg(const QList<Message> &msgs);
    void repairMsg();
    
private:
    bool isFileValid(const QString &fileName);
    bool isMsgExist(const Message &msg);
    QString convertMsgToLog(const Message &msg);

signals:

public slots:

};

#endif // MSGTOOL_H
