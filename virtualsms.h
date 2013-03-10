#ifndef VIRTUALSMS_H
#define VIRTUALSMS_H

#include <QMainWindow>
#include <QUdpSocket>
#include "dialogchatroom.h"
#include "dialogmsgdetail.h"
#include "dialogmsgmanage.h"

namespace Ui {
    class VirtualSMS;
}

class VirtualSMS : public QMainWindow
{
    Q_OBJECT

public:
    explicit VirtualSMS(QWidget *parent = 0);
    ~VirtualSMS();
    QRect getTrayIconRect();
    
public slots:
    //����ʵ�ָú���������������ʾ������ʱ������ϵͳ����ͼ���Ҽ��˵���
    void setVisible ( bool visible );
    
protected:
    void closeEvent(QCloseEvent *ce);
    bool eventFilter(QObject *o, QEvent *e);
    void showEvent(QShowEvent *se);
    
signals:
    void signNewMsgArrived(int);
    
private slots:
    //��Ӧ���������Ϣ��ӡ��޸ġ�ɾ��
    void slotUpdateMsg(int id, int changeEvent, int srcBox);    
    //��Ӧ�绰��������и���
    void slotUpdatePhoneBook(int id, int changeEvent);
   
    //�Ҽ��˵���
    //�绰����ť�˵�
    void slotActionShowAll();
    void slotActionShowOnline();
    //�绰���б�
    void slotActionGotoChat();
    void slotActionSendMsg();
    void slotActionViewContactDetail();
    void slotActionMoveToBlackList();
    void slotActionMoveToWhiteList();
    void slotActionMoveToOther();
    void slotActionMoveToStranger();
    void slotActionDeleteContact();
    //����
    void slotActionViewMsgDetail();
    void slotActionEditUnsendMsg();
    void slotActionGotoChatFromMsg();
    void slotActionRead();
    void slotActionStopRead();
    void slotActionForwarding();    
    void slotActionReply();
    void slotActionMoveToInbox();
    void slotActionMoveToOutbox();
    void slotActionMoveToDraftbox();
//    void slotActionMoveToDustbin();
    void slotActionAddIntoPhoneBook();
    void slotActionViewMoreMsg();
    void slotActionRemoveMsg();
    
    void slotMsgBtnClicked();
    void slotPhonebookBtnClicked();
    void slotChatRoomClosed(QObject*);    
    void slotProcessMsgDoubledClicked(QModelIndex index);    
    void slotProcessMsgContextMenu();
    void slotProcessPhoneBookClicked(QListWidgetItem *item);
    void slotProcessPhoneBookDoubledClicked(QModelIndex index);
    void slotProcessPhoneBookContextMenu(); 
    void slotProcessPhoneBookBtnContextMenu();
    void slotChangeLanguage();
    void slotChangeRunMode();
    void slotTrayIconActivated(QSystemTrayIcon::ActivationReason reason);  //˫��ϵͳ����ʱ��ԭ����С��
    void slotOpen();
    void slotProcessNewInMsg(const QString &phoneNum, const QString &content);
    void slotFlicker();
    void slotAutoSuggest();
    void slotDoneCompletion();
    void slotFinishRead();
    void slotPhoneNumConflict(const QString &name, const QHostAddress &ipAddr);
    void slotContactLogin(const QString &phoneNum);
    void slotQuit();
    void slotCheck();
    void slotMsgManageDestroyed();
    
    void on_btnWriteMsg_clicked();    
    void on_btnSet_clicked();        
    void on_btnAddContact_clicked();          
    void on_btnMsgManage_clicked();
    
private:
    //����
    QTimer m_timerRemind;
    QTimer m_timerSearch;
    QTimer m_timerCheck;
    bool m_remindFlag;
    bool m_isInit;
    QMultiMap<QString, int> m_newInMsgPhoneNumToIds;
    QMultiMap<QString, int> m_phoneNumToSendFailMsgs;
    QTranslator m_tr;
    QTranslator m_trInner;
    const char *m_strInbox;
    const char *m_strOutbox;
    const char *m_strDraftBox;
    const char *m_strDustbin;    
    const char *m_strImportMsg;
    enum TabIndex {
        Tab_PhoneBook,
        Tab_Message
    };
   
    //�ؼ�
    Ui::VirtualSMS *ui;
    QVector<QGroupBox*> m_msgGroupBoxs;  
    QVector<QListWidget*> m_msgListWidgets;  //��Ϣ���б���
    QVector<QToolButton*> m_msgBtns;    //��Ϣ����ť����
    QVector<QGroupBox*> m_phonebookGroupBoxs;  
    QVector<QListWidget*> m_phonebookListWidgets;  //�绰���б���
    QVector<QToolButton*> m_phonebookBtns;    //�绰����ť����
    QList<QAction*> m_phoneBookBtnActions; //�绰����ť�Ҽ��˵���
    QVector<bool> m_isPhoneBookBtnsExpand;
    QMap<int, QObject*> m_idToChatRoom;
    DialogMsgDetail *m_dMsgDetail;
    DialogMsgManage *m_dMsgManage;
    QListWidget *m_popup;
    
    //�绰����ť�˵�
//    QAction *m_actShowBlacklist;
//    QAction *m_actShowWhitelist;
//    QAction *m_actShowOther;
//    QAction *m_actShowStranger;
    QAction *m_actShowAll;
    QAction *m_actShowOnline;
    QMenu *m_menuPhoneBookBtn;    
    //�绰���б�˵�
    QAction *m_actGoToChat;
    QAction *m_actSendMsg;
    QAction *m_actViewContactDetail;
    QAction *m_actMoveToBlackList;
    QAction *m_actMoveToWhiteList;
    QAction *m_actMoveToOther;
    QAction *m_actMoveToStranger;
    QAction *m_actDeleteContact;
    QMenu *m_menuMoveTo;
    QMenu *m_menuPhoneBook;
    
    //����˵�
    QAction *m_actViewMsgDetail;
    QAction *m_actEditUnsendMsg;
    QAction *m_actGotoChatFromMsg;
    QAction *m_actRead;
    QAction *m_actStopRead;
    QAction *m_actForwarding;           
    QAction *m_actReply;
    QAction *m_actMoveToInbox;
    QAction *m_actMoveToOutbox;
    QAction *m_actMoveToDraftbox;
//    QAction *m_actMoveToDustbin;
    QAction *m_actAddIntoPhoneBook;
    QAction *m_actViewMoreMsg;
    QAction *m_actRemoveMsg;
    QMenu *m_menuMsg;    
    
    //���̲˵�
    QAction *m_restoreAction;         //��ԭ
    QAction *m_minAction;             //��С��
    QAction *m_quitAction;            //�˳�
    QSystemTrayIcon *m_trayIcon;      //����
    QMenu *m_trayIconMenu;            //���̲˵�
    
    void init();
    void initTrayIcon();                    //��ʼ������
    void CancelFocus(QWidget *w);    
    void openChatRoom(const Contact &contact);
    void retranslateUi();
    void changeMode(RunMode runMode);
    void showCompletion(const QList<Contact> &choices);
    IdentificationState identifyRemind(const QString &tip);

    //��ϵ��
    void addContact(int id);
    void removeContact(int id);
    void modifyContact(int id);
    void changeContactBox(int id);
    void setContactHead(const Contact &contact, QListWidgetItem *lwItem);
    void setOfflineContactsVisible(bool isVisible);
    void viewContactDetail(Contact contact=Contact());
    QToolButton* getPhoneBookBtn(int type);
    QListWidget* getPhoneBookBox(int type);
    QListWidget* getFocusedPhoneBookBox();
    Contact getSelectedContact();
    void updatePhoneBookList();
    //����    
    void addMsg(int id);
    void addMsg(Message msg);
    void removeMsg(int id);
    void changeMsgBox(int id);
    void changeMsgState(int id);
    void updateMsgBoxNum();
    void viewMsgDetail(const Message &msg);
    void viewMsgManage();
    QListWidget* getMsgBox(int box);
    QListWidget* getFocusedMsgBox();
    Message getSelectedMesssage();
};

#endif // VIRTUALSMS_H
