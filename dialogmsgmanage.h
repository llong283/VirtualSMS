#ifndef DIALOGMSGMANAGE_H
#define DIALOGMSGMANAGE_H

#include <QDialog>
#include "config.h"

namespace Ui {
    class DialogMsgManage;
}

class DialogMsgManage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMsgManage(QWidget *parent = 0);
    ~DialogMsgManage();
    
    enum DataIndex {
        DataIndex_Type,
        DataIndex_Value
    };

    enum DataType {
        DataType_MessageBox,
        DataType_PhoneBook
    };
    
protected:
    void changeEvent(QEvent *e);

private slots:
    //树的菜单
    void slotActionExport();
    void slotActionViewUnread();
    void slotActionSetAllRead();
    void slotActionDeleteAll();
    void slotActionDeleteMoreSetting();

    //信息列表菜单
    void slotActionRead();
    void slotActionStopRead();
    void slotActionForwarding();    
    void slotActionReply();
    void slotActionMoveToInbox();
    void slotActionMoveToOutbox();
    void slotActionMoveToDraftbox();
//    void slotActionMoveToDustbin();
    void slotActionRestore();
    void slotActionAddIntoPhoneBook();
    void slotActionSetRead();
    void slotActionDelete();

    void slotFinishRead();    
    void slotDeleteMsgsBefore(const QDateTime &dateTime);
    void slotProcessMsgChange(int id, int changeEvent, int parameter);
    
    void on_treeWidget_clicked(QModelIndex index);    
    void on_treeWidget_customContextMenuRequested(QPoint pos);
    void on_listWidget_customContextMenuRequested(QPoint pos);          
    void on_listWidget_itemDoubleClicked(QListWidgetItem* item);
    void on_btnFirstPage_clicked();    
    void on_btnPrevPage_clicked();    
    void on_btnNextPage_clicked();    
    void on_btnLastPage_clicked();    
    void on_lineEditCurPage_returnPressed();    
    void on_btnImport_clicked();
    void on_btnExport_clicked();       
    
    void on_btnRepair_clicked();
    
private:
    Ui::DialogMsgManage *ui;
    QTreeWidgetItem *m_msgBoxTopItem;
    QTreeWidgetItem *m_phoneBookItem;
    QList<QTreeWidgetItem*> m_msgBoxItems;
    QList<QTreeWidgetItem*> m_contactTypeItems;
    QIntValidator m_valid;
    bool m_isViewUnread;
    QList<int> m_setReadMsgIds;
    
    //信息列表菜单
    QAction *m_actRead;
    QAction *m_actStopRead;
    QAction *m_actForwarding;           
    QAction *m_actReply;
//    QAction *m_actMoveToInbox;
//    QAction *m_actMoveToOutbox;
//    QAction *m_actMoveToDraftbox;
//    QAction *m_actMoveToDustbin;
    QAction *m_actRestore;
    QAction *m_actAddIntoPhoneBook;
    QAction *m_actDelete;    
    QMenu *m_menuMsgList;
    
    //左边的树的菜单
    QAction *m_actViewUnread;
    QAction *m_actExport;
    QAction *m_actSetAllRead;
    QAction *m_actDeleteAll;
    QAction *m_actDeleteMoreSetting;
    QMenu *m_menuTree;
    
    void init();
    void retranslateUi();
    void updateMsgBoxNum();
    void updateMsgList();
    void addMsg(Message msg, DataType dataType);  
    QList<Message> getAllMessagesOfTreeItem();
    Message getSelectedMesssage();
    QList<Message> getSelectedMesssages();
    void viewContactDetail(Contact contact=Contact());
};

#endif // DIALOGMSGMANAGE_H
