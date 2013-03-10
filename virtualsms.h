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
    //重新实现该函数，当主窗口显示或隐藏时，更新系统托盘图标右键菜单项
    void setVisible ( bool visible );
    
protected:
    void closeEvent(QCloseEvent *ce);
    bool eventFilter(QObject *o, QEvent *e);
    void showEvent(QShowEvent *se);
    
signals:
    void signNewMsgArrived(int);
    
private slots:
    //响应信箱类的消息添加、修改、删除
    void slotUpdateMsg(int id, int changeEvent, int srcBox);    
    //响应电话本类的所有更改
    void slotUpdatePhoneBook(int id, int changeEvent);
   
    //右键菜单项
    //电话本按钮菜单
    void slotActionShowAll();
    void slotActionShowOnline();
    //电话本列表
    void slotActionGotoChat();
    void slotActionSendMsg();
    void slotActionViewContactDetail();
    void slotActionMoveToBlackList();
    void slotActionMoveToWhiteList();
    void slotActionMoveToOther();
    void slotActionMoveToStranger();
    void slotActionDeleteContact();
    //信箱
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
    void slotTrayIconActivated(QSystemTrayIcon::ActivationReason reason);  //双击系统托盘时还原或最小化
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
    //变量
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
   
    //控件
    Ui::VirtualSMS *ui;
    QVector<QGroupBox*> m_msgGroupBoxs;  
    QVector<QListWidget*> m_msgListWidgets;  //信息栏列表集合
    QVector<QToolButton*> m_msgBtns;    //信息栏按钮集合
    QVector<QGroupBox*> m_phonebookGroupBoxs;  
    QVector<QListWidget*> m_phonebookListWidgets;  //电话本列表集合
    QVector<QToolButton*> m_phonebookBtns;    //电话本按钮集合
    QList<QAction*> m_phoneBookBtnActions; //电话本按钮右键菜单项
    QVector<bool> m_isPhoneBookBtnsExpand;
    QMap<int, QObject*> m_idToChatRoom;
    DialogMsgDetail *m_dMsgDetail;
    DialogMsgManage *m_dMsgManage;
    QListWidget *m_popup;
    
    //电话本按钮菜单
//    QAction *m_actShowBlacklist;
//    QAction *m_actShowWhitelist;
//    QAction *m_actShowOther;
//    QAction *m_actShowStranger;
    QAction *m_actShowAll;
    QAction *m_actShowOnline;
    QMenu *m_menuPhoneBookBtn;    
    //电话本列表菜单
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
    
    //信箱菜单
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
    
    //托盘菜单
    QAction *m_restoreAction;         //还原
    QAction *m_minAction;             //最小化
    QAction *m_quitAction;            //退出
    QSystemTrayIcon *m_trayIcon;      //托盘
    QMenu *m_trayIconMenu;            //托盘菜单
    
    void init();
    void initTrayIcon();                    //初始化托盘
    void CancelFocus(QWidget *w);    
    void openChatRoom(const Contact &contact);
    void retranslateUi();
    void changeMode(RunMode runMode);
    void showCompletion(const QList<Contact> &choices);
    IdentificationState identifyRemind(const QString &tip);

    //联系人
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
    //信箱    
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
