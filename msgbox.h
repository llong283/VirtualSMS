#ifndef MSGBOX_H
#define MSGBOX_H

#include <QObject>
#include "abstractmsg.h"

#define g_msgbox MsgBox::instance()

class MsgBox : public AbstractMsg
{
    Q_OBJECT
public:
    static MsgBox *instance();
    
    bool init();

private:    
    MsgBox(QObject *parent = 0);
    static MsgBox* msgbox;
};

#endif // MSGBOX_H
