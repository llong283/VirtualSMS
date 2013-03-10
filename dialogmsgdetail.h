#ifndef DIALOGMSGDETAIL_H
#define DIALOGMSGDETAIL_H

#include <QDialog>
#include "config.h"

namespace Ui {
    class DialogMsgDetail;
}

class DialogMsgDetail : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMsgDetail(QWidget *parent = 0);
    ~DialogMsgDetail();
    void loadMsg(Message msg);
    
protected:
    void changeEvent(QEvent *e);

private slots:
    void slotMoveToInbox();
    void slotMoveToOutbox();
    void slotMoveToDraftbox();
    void slotMoveToDustbin();  
    void slotFinishRead();
    
    void on_btnReply_clicked();    
    void on_btnForwarding_clicked();    
    void on_btnRead_clicked();
    
private:
    Ui::DialogMsgDetail *ui;
    Message m_msg;
    QAction *m_actMoveToInbox;
    QAction *m_actMoveToOutbox;
    QAction *m_actMoveToDraftbox;
    QAction *m_actMoveToDustbin;
    QMenu *m_menuInMsg;    
    bool m_isReading;
    const char *m_strSender;
    const char *m_strDateTime;
    const char *m_strReceiver;
    const char *m_strState;
    
    void init();
    void updateActionState();
    void retranslateUi();
};

#endif // DIALOGMSGDETAIL_H
