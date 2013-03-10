#ifndef DIALOGCHATROOM_H
#define DIALOGCHATROOM_H

#include "config.h"
#include "messagesender.h"

namespace Ui {
    class DialogChatRoom;
}

class DialogChatRoom : public QDialog
{
    Q_OBJECT

public:
    DialogChatRoom(Contact contact, QWidget *parent = 0);
    ~DialogChatRoom();
    void addMsg(Message msg, QString tip=QString());
        
protected:
    void changeEvent(QEvent *e);
    
private:
    Ui::DialogChatRoom *ui;
    Contact m_contact;
    QSplitter *m_mainSplitter;
    MessageSend m_sender;  
    QQueue<QString> m_msgQueue;
    Contact m_myself;
    const char *m_strInfo;
    
    void init();
    void append(QString strMsg);
    void retranslateUi();
    
signals:
    void signClose(int);
    
private slots:
    void slotStartNextSend();
    void on_btnSend_clicked();
    void on_btnClose_clicked();
};

#endif // DIALOGCHATROOM_H
