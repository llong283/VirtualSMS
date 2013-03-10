#ifndef DIALOGWRITEMSG_H
#define DIALOGWRITEMSG_H

#include <QDialog>
#include "config.h"
#include "messagesender.h"

namespace Ui {
    class DialogWriteMsg;
}

class DialogWriteMsg : public QDialog
{
    Q_OBJECT

public:
    explicit DialogWriteMsg(QWidget *parent = 0);
    ~DialogWriteMsg();
    void init();
    void initReply(QString phoneNum);
    void initForwarding(QString content);
    void initSendUnsendMsg(Message msg);

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *ce);
    void keyPressEvent(QKeyEvent *ke);
    void showEvent(QShowEvent *se);
    
private slots:
    void on_treeWidget_itemClicked(QTreeWidgetItem* item, int column);    
    
    void on_btnSend_clicked();    
    void on_btnClose_clicked();
    void on_textEditReceiver_textChanged();        
    void on_checkBoxManualInputNum_clicked(bool checked);    
    void on_textEditTip_textChanged();
    
private:
    Ui::DialogWriteMsg *ui;    
    QList<QTreeWidgetItem*> m_contactTypeItems;

    QList<Contact> m_contacts;
    MessageSend m_udpSender;
    int m_type;
    enum Type {
        Type_WriteMsg,
        Type_Reply,
        Type_Forwarding,
        Type_Invalid
    };
    
    void initContactList();
    void retranslateUi();
    bool checkValidity();
    void showTip(const QString &strTip);
    void hideTip();
};

#endif // DIALOGWRITEMSG_H
