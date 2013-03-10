/************************************************
类名: DialogMsgManage
功能: 信息管理
说明: 无
************************************************/

#include "dialogmsgmanage.h"
#include "ui_dialogmsgmanage.h"

#include "msgbox.h"
#include "phonebook.h"
#include "msgmanageitemdelegate.h"
#include "itemdelegate.h"
#include "QtSpeech.h"
#include "setting.h"
#include "msgtool.h"

#include "MessageIdentification.h"
#include "phonenumberidentification.h"
#include "numbersegmentidentification.h"

#include "dialogdeletemsg.h"
#include "dialogwritemsg.h"
#include "dialogcontact.h"

const int cons_msg_num_per_page = 20;

DialogMsgManage::DialogMsgManage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMsgManage),
    m_isViewUnread(false)
{
    ui->setupUi(this);
    
    init();
    retranslateUi();
}

DialogMsgManage::~DialogMsgManage()
{
    delete ui;
}

void DialogMsgManage::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
    }
    QDialog::changeEvent(e);    
}

void DialogMsgManage::retranslateUi()
{
    setWindowTitle(tr("Message Manage"));
    m_msgBoxTopItem->setText(0, tr("Message"));
    m_phoneBookItem->setText(0, tr("PhoneBook"));
    for (int i=0; i<=Contact::Type_Stranger; i++) {
        m_contactTypeItems[i]->setText(0, Contact::contactTypeToString((Contact::Type)i));
    }    
    updateMsgBoxNum();
    
    //信息列表菜单
    m_actRead->setText(tr("Read"));
    m_actStopRead->setText(tr("StopRead"));
    m_actForwarding->setText(tr("Forwarding"));
    m_actReply->setText(tr("Reply"));
//    m_actMoveToInbox->setText(tr("MoveToInbox"));
//    m_actMoveToOutbox->setText(tr("MoveToOutbox"));
//    m_actMoveToDraftbox->setText(tr("MoveToDraftbox"));
//    m_actMoveToDustbin->setText(tr("MoveToDustbin"));
    m_actRestore->setText(tr("Restore"));
    m_actAddIntoPhoneBook->setText(tr("AddIntoPhoneBook"));
    m_actDelete->setText(tr("Delete"));
    
    //左边树的菜单
    m_actViewUnread->setText(tr("ViewUnread"));
    m_actSetAllRead->setText(tr("SetAllRead"));
    m_actExport->setText(tr("Export"));
    m_actDeleteAll->setText(tr("DeleteAll"));
    m_actDeleteMoreSetting->setText(tr("DeleteMoreSetting"));
}

void DialogMsgManage::updateMsgBoxNum()
{
    for (int i=0; i<=Message::Box_Dustbin; i++) {
        switch (i) {
        case Message::Box_Inbox:
            m_msgBoxItems[i]->setText(0, tr("Inbox(%1/%2)")
                                      .arg(g_msgbox->getUnreadInboxNum())
                                      .arg(g_msgbox->getBoxNum(Message::Box_Inbox)));
            break;
        case Message::Box_Outbox:
            m_msgBoxItems[i]->setText(0, tr("Outbox(%1)")
                                      .arg(g_msgbox->getBoxNum(Message::Box_Outbox)));
            break;
        case Message::Box_Draftbox:
            m_msgBoxItems[i]->setText(0, tr("Draftbox(%1)")
                                      .arg(g_msgbox->getBoxNum(Message::Box_Draftbox)));
            break;
        case Message::Box_Dustbin:
            m_msgBoxItems[i]->setText(0, tr("Dustbin(%1/%2)")
                                      .arg(g_msgbox->getUnreadDustbinNum())
                                      .arg(g_msgbox->getBoxNum(Message::Box_Dustbin)));
            break;
        default:
            break;
        }
    }        
}

void DialogMsgManage::init()
{
    setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);
    //加载qss
    QFile file(":/qss/Dialog.qss");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
    } else {
        setStyleSheet(file.readAll());
    }
    file.close();    
    connect(g_msgbox, SIGNAL(signMsgChanged(int,int,int))
            , this, SLOT(slotProcessMsgChange(int,int,int)), Qt::QueuedConnection);
    
    ui->btnFirstPage->setEnabled(false);
    ui->btnPrevPage->setEnabled(false);
    ui->btnNextPage->setEnabled(false);
    ui->btnLastPage->setEnabled(false);

    m_valid.setRange(0, 0);
    ui->lineEditCurPage->setValidator(&m_valid);
    ui->lineEditCurPage->setText("0");
    ui->lineEditCurPage->setAlignment(Qt::AlignCenter);
    ui->labelPageNum->setAlignment(Qt::AlignCenter);
    
    //信息列表菜单
    m_actRead = new QAction(this);
    connect(m_actRead, SIGNAL(triggered()), this, SLOT(slotActionRead()));
    m_actStopRead = new QAction(this);
    connect(m_actStopRead, SIGNAL(triggered()), this, SLOT(slotActionStopRead()));
    m_actForwarding = new QAction(this);
    connect(m_actForwarding, SIGNAL(triggered()), this, SLOT(slotActionForwarding()));
    m_actReply = new QAction(this);
    connect(m_actReply, SIGNAL(triggered()), this, SLOT(slotActionReply()));
//    m_actMoveToInbox = new QAction(this);
//    connect(m_actMoveToInbox, SIGNAL(triggered()), this, SLOT(slotActionMoveToInbox()));
//    m_actMoveToOutbox = new QAction(this);
//    connect(m_actMoveToOutbox, SIGNAL(triggered()), this, SLOT(slotActionMoveToOutbox()));
//    m_actMoveToDraftbox = new QAction(this);
//    connect(m_actMoveToDraftbox, SIGNAL(triggered()), this, SLOT(slotActionMoveToDraftbox()));
//    m_actMoveToDustbin = new QAction(this);
//    connect(m_actMoveToDustbin, SIGNAL(triggered()), this, SLOT(slotActionMoveToDustbin()));
    m_actRestore = new QAction(this);
    connect(m_actRestore, SIGNAL(triggered()), this, SLOT(slotActionRestore()));
    m_actAddIntoPhoneBook = new QAction(this);
    connect(m_actAddIntoPhoneBook, SIGNAL(triggered()), this, SLOT(slotActionAddIntoPhoneBook()));
    m_actDelete = new QAction(this);
    connect(m_actDelete, SIGNAL(triggered()), this, SLOT(slotActionDelete()));
    m_menuMsgList = new QMenu(this);
    m_menuMsgList->addAction(m_actRead);
    m_menuMsgList->addAction(m_actStopRead);
    m_menuMsgList->addAction(m_actForwarding);
    m_menuMsgList->addAction(m_actReply);
    m_menuMsgList->addSeparator();
    m_menuMsgList->addAction(m_actAddIntoPhoneBook);
    m_menuMsgList->addSeparator();
//    m_menuMsgList->addAction(m_actMoveToInbox);
//    m_menuMsgList->addAction(m_actMoveToOutbox);
//    m_menuMsgList->addAction(m_actMoveToDraftbox);
//    m_menuMsgList->addAction(m_actMoveToDustbin);
    m_menuMsgList->addAction(m_actRestore);
    m_menuMsgList->addSeparator();
    m_menuMsgList->addAction(m_actDelete);
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    m_actSetAllRead = new QAction(this);
    connect(m_actSetAllRead, SIGNAL(triggered()), this, SLOT(slotActionSetAllRead()));
    m_actViewUnread = new QAction(this);
    connect(m_actViewUnread, SIGNAL(triggered()), this, SLOT(slotActionViewUnread()));
    m_actExport = new QAction(this);
    connect(m_actExport, SIGNAL(triggered()), this, SLOT(slotActionExport()));
    m_actDeleteAll = new QAction(this);
    connect(m_actDeleteAll, SIGNAL(triggered()), this, SLOT(slotActionDeleteAll()));
    m_actDeleteMoreSetting = new QAction(this);
    connect(m_actDeleteMoreSetting, SIGNAL(triggered()), this, SLOT(slotActionDeleteMoreSetting()));
    m_menuTree = new QMenu(this);
    m_menuTree->addAction(m_actViewUnread);
    m_menuTree->addAction(m_actSetAllRead);
    m_menuTree->addSeparator();
    m_menuTree->addAction(m_actExport);
    m_menuTree->addSeparator();
    m_menuTree->addAction(m_actDeleteAll);
    m_menuTree->addAction(m_actDeleteMoreSetting);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->treeWidget->header()->hide();
    m_msgBoxTopItem = new QTreeWidgetItem(ui->treeWidget);
    m_msgBoxTopItem->setSizeHint(0, QSize(0, cons_tree_item_height));
    for (int i=0; i<=Message::Box_Dustbin; i++) {
        QTreeWidgetItem *twItem = new QTreeWidgetItem(m_msgBoxTopItem);
        twItem->setData(0, Qt::UserRole + DataIndex_Type, DataType_MessageBox);
        twItem->setData(0, Qt::UserRole + DataIndex_Value, i);
        twItem->setSizeHint(0, QSize(0, cons_tree_item_height));
        m_msgBoxItems << twItem;
    }
    m_phoneBookItem = new QTreeWidgetItem(ui->treeWidget);
    m_phoneBookItem->setSizeHint(0, QSize(0, cons_tree_item_height));
    for (int i=0; i<=Contact::Type_Stranger; i++) {
        QTreeWidgetItem *twItem = new QTreeWidgetItem(m_phoneBookItem);
        twItem->setSizeHint(0, QSize(0, cons_tree_item_height));
        QList<Contact> contacts = g_phoneBook->getContactsOfType((Contact::Type)i);
        foreach (Contact contact, contacts) {
            QTreeWidgetItem *twItemT = new QTreeWidgetItem(twItem);
            twItemT->setText(0, QString("%1(%2)").arg(contact.name).arg(contact.phonenum));
            twItemT->setData(0, Qt::UserRole + DataIndex_Type, DataType_PhoneBook);
            twItemT->setData(0, Qt::UserRole + DataIndex_Value, QVariant::fromValue<Contact>(contact));
            twItemT->setSizeHint(0, QSize(0, cons_tree_item_height));
        }
        m_contactTypeItems << twItem;
    }
    
    ui->listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listWidget->setItemDelegate(new MsgManageItemDelegate(this));
    
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->addWidget(ui->treeWidget);
    mainSplitter->addWidget(ui->groupBoxMsgList);
    mainSplitter->setStretchFactor(1, 1);
    layout()->addWidget(mainSplitter);    
}

void DialogMsgManage::slotProcessMsgChange(int id, int changeEvent, int parameter)
{
    switch (changeEvent) {
    case MsgBox::ChangeEvent_Add:
    case MsgBox::ChangeEvent_Remove:
    case MsgBox::ChangeEvent_ChangeBox:
        updateMsgList();
        break;
    case MsgBox::ChangeEvent_ChangeState:
        if (parameter==Message::State_Unread && m_isViewUnread) {
            m_setReadMsgIds << id;
        }
        updateMsgList();
        break;
    default:
        break;
    }
}

void DialogMsgManage::addMsg(Message msg, DataType dataType)
{
    if (msg.isEmpty()) {
        qDebug() << "msg is empty" << msg.id << msg.content << __FILE__ << __LINE__;
        return;
    }
//    qDebug() << "add msg" << msg.content << msg.box << __FILE__ << __LINE__;
    QListWidgetItem *lwItem = new QListWidgetItem;
    //设置索引，好让代理能够识别，做出正确显示
    lwItem->setData(Qt::UserRole + DataIndex_Type, dataType);
    lwItem->setData(Qt::UserRole + DataIndex_Value, QVariant::fromValue<Message>(msg));

    int count = ui->listWidget->count();
    if (count > 0) {
        for (int i=count-1; i>=0; i--) {
            Message msgT = ui->listWidget->item(i)->data(Qt::UserRole + DataIndex_Value).value<Message>();
            if (msgT.datetime<msg.datetime || (msgT.datetime==msg.datetime && msgT.id<msg.id)) {
//                qDebug() << "insert after position" << i << "insert msg content" << msg.content
//                         << "listWidget count" << ui->listWidget->count() << __FILE__ << __LINE__;
                ui->listWidget->insertItem(i+1, lwItem);
                break;
            }
            if (i==0) {
                ui->listWidget->insertItem(0, lwItem);
            }
        }
    } else {
        ui->listWidget->addItem(lwItem);
    }
}

void DialogMsgManage::updateMsgList()
{
    ui->listWidget->clear();
    if (!ui->lineEditCurPage->isEnabled()) {
        ui->lineEditCurPage->setEnabled(true);
    }
    updateMsgBoxNum();
    
    //当前是未读状态时，采用另一种更新策略
    if (m_isViewUnread) {
        slotActionViewUnread();
        foreach (int id, m_setReadMsgIds) {
            addMsg(g_msgbox->getMessageOfId(id), DataType_MessageBox);
        }
        return;        
    }
    
    int startNo;
    int recordNum;
    int pageNum;
    int currentPage;
    QVariant var = ui->treeWidget->currentIndex().data(Qt::UserRole + DataIndex_Type);
    if (var.isNull()) {
        return;
    }
    if (var.toInt() == DataType_MessageBox) {
        Message::Box box = (Message::Box)ui->treeWidget->currentIndex()
                .data(Qt::UserRole + DataIndex_Value).toInt();
        recordNum = g_msgbox->getBoxNum(box);
    } else {
        Contact contact = ui->treeWidget->currentIndex()
                .data(Qt::UserRole + DataIndex_Value).value<Contact>();
        recordNum = g_msgbox->getMessageNumOfPhoneNum(contact.phonenum);
    }
    m_valid.setRange(1, recordNum / cons_msg_num_per_page + 1);
    pageNum = recordNum / cons_msg_num_per_page + (recordNum % cons_msg_num_per_page != 0);
    ui->labelPageNum->setText(QString::number(pageNum));
    currentPage = ui->lineEditCurPage->text().toInt();
    if (currentPage == 0) {
        if (recordNum > 0) {
            currentPage = 1;
            ui->lineEditCurPage->setText(QString("%1").arg(currentPage));
        }
        ui->btnFirstPage->setEnabled(false);
        ui->btnPrevPage->setEnabled(false);
    } else if (currentPage == 1) {
        ui->btnFirstPage->setEnabled(false);
        ui->btnPrevPage->setEnabled(false);
    } else {
        ui->btnFirstPage->setEnabled(true);
        ui->btnPrevPage->setEnabled(true);
    }
    if (currentPage == pageNum) {
        ui->btnLastPage->setEnabled(false);
        ui->btnNextPage->setEnabled(false);
    } else if (currentPage > pageNum) {
        ui->btnLastPage->setEnabled(false);
        ui->btnNextPage->setEnabled(false);
        ui->lineEditCurPage->setText(QString("%1").arg(pageNum));
    } else {
        ui->btnLastPage->setEnabled(true);
        ui->btnNextPage->setEnabled(true);
    }

    startNo = (ui->lineEditCurPage->text().toInt() - 1) * cons_msg_num_per_page;
    qDebug() << "startNo" << startNo << "recordNum" << recordNum << __FILE__ << __LINE__;
    if (startNo >= 0) {
        QList<Message> msgs;
        if (var.toInt() == DataType_MessageBox) {
            Message::Box box = (Message::Box)ui->treeWidget->currentIndex()
                    .data(Qt::UserRole + DataIndex_Value).toInt();
            msgs = g_msgbox->getMessagesOfBox(box, startNo, cons_msg_num_per_page);
            qDebug() << "msg count" << msgs.count() << "box" << box << __FILE__ << __LINE__;
            foreach (Message msg, msgs) {
                addMsg(msg, DataType_MessageBox);
            }
        } else {
            Contact contact = ui->treeWidget->currentIndex()
                    .data(Qt::UserRole + DataIndex_Value).value<Contact>();
            msgs = g_msgbox->getMessagesOfPhoneNum(contact.phonenum, startNo, cons_msg_num_per_page);
            foreach (Message msg, msgs) {
                addMsg(msg, DataType_PhoneBook);
            }
        }        
    }
}

//获取选中的信息
Message DialogMsgManage::getSelectedMesssage()
{
    return getSelectedMesssages().at(0);
}

QList<Message> DialogMsgManage::getSelectedMesssages()
{
    QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
    QList<Message> msgs;
    foreach (QListWidgetItem *item, items) {
        Message msg = item->data(Qt::UserRole + DataIndex_Value).value<Message>();
        msgs << msg;
    }
    return msgs;
}

//收件箱、垃圾箱、发件箱共有
//信息朗读
void DialogMsgManage::slotActionRead()
{
    Message msg = getSelectedMesssage();
    g_speech->setVoice(g_setting->getVoiceInfo());
    g_speech->tell(msg.content, this, SLOT(slotFinishRead()));
    if (msg.state == Message::State_Unread) {
        g_msgbox->setRead(msg.id, true);
    }
//    updateMsgList();
}

//停止朗读
void DialogMsgManage::slotActionStopRead()
{
    g_speech->stop();
}

//收件箱、垃圾箱、发件箱共有
//信息转发
void DialogMsgManage::slotActionForwarding()
{
    DialogWriteMsg *dwMsg = new DialogWriteMsg;
    dwMsg->initForwarding(getSelectedMesssage().content);
    dwMsg->show();
}

//收件箱、垃圾箱共有
//回复
void DialogMsgManage::slotActionReply()
{    
    DialogWriteMsg *dwMsg = new DialogWriteMsg;
    dwMsg->initReply(getSelectedMesssage().phonenum);
    dwMsg->show();        
}

//移至收件箱
void DialogMsgManage::slotActionMoveToInbox()
{    
    g_msgbox->changeBox(getSelectedMesssage().id, Message::Box_Inbox);
//    updateMsgList();
}

//移至发件箱
void DialogMsgManage::slotActionMoveToOutbox()
{    
    g_msgbox->changeBox(getSelectedMesssage().id, Message::Box_Outbox);
//    updateMsgList();
}

//移至草稿箱
void DialogMsgManage::slotActionMoveToDraftbox()
{
    g_msgbox->changeBox(getSelectedMesssage().id, Message::Box_Draftbox);
//    updateMsgList();
}

////移至垃圾箱
//void DialogMsgManage::slotActionMoveToDustbin()
//{
//    Message msg = getSelectedMesssage();
//    g_msgbox->changeBox(msg.id, Message::Box_Dustbin);
//    g_MessageIdentification->MessageIdentificationProcessAndSave(msg.content);
////    updateMsgList();
//}

//还原
void DialogMsgManage::slotActionRestore()
{
    QList<Message> msgs = getSelectedMesssages();
    foreach (Message msg, msgs) {
        if (msg.msgtype==Message::MsgType_InMsg) {
            g_msgbox->changeBox(msg.id, Message::Box_Inbox);
        } else {
            if (msg.state==Message::State_Sent) {
                g_msgbox->changeBox(msg.id, Message::Box_Outbox);
            } else {
                g_msgbox->changeBox(msg.id, Message::Box_Draftbox);
            }
        }
    }
}

//设置为已读
void DialogMsgManage::slotActionSetRead()
{
    QList<Message> msgs = getSelectedMesssages();
    foreach (Message msg, msgs) {
        if (msg.state == Message::State_Unread) {
            g_msgbox->setRead(msg.id, true);
        }
    }
}

//删除消息，只有当前是处于垃圾箱时才是彻底的删除
void DialogMsgManage::slotActionDelete()
{
    if (QMessageBox::No == QMessageBox::information(
                this, tr("msg"), tr("Are you sure to delete?")
                , QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        return;
    }
    QModelIndex index = ui->treeWidget->currentIndex();
    bool isThroughDelete = false;
    if (index.data(Qt::UserRole + DataIndex_Type).toInt() == DataType_MessageBox) {            
        if (index.data(Qt::UserRole + DataIndex_Value).toInt() == Message::Box_Dustbin) {
            isThroughDelete = true;
        }
    }
    
    QList<Message> msgs = getSelectedMesssages();
    if (isThroughDelete) {
        foreach (Message msg, msgs) {
            g_msgbox->removeMessage(msg.id);
        }
    } else {
        foreach (Message msg, msgs) {
            if (msg.box != Message::Box_Dustbin) {
                g_msgbox->changeBox(msg.id, Message::Box_Dustbin);
                if (ui->checkBoxRecord->isChecked()) {
                    //识别
                    g_MessageIdentification->MessageIdentificationProcessAndSave(msg.content);
                    g_PhoneNumberIdentification->NumberIdentificationProcessAndSave(msg.phonenum);
                    g_NumberSegmentIdentification->NSIProcessAndSave(msg.phonenum.toULongLong());
                }
            }
        }
    }
}

//陌生号码存入电话本
void DialogMsgManage::slotActionAddIntoPhoneBook()
{
    Contact contact;
    contact.phonenum = getSelectedMesssage().phonenum;
    viewContactDetail(contact);
}

//查看联系人信息
void DialogMsgManage::viewContactDetail(Contact contact)
{
    DialogContact *dContact = new DialogContact;
    dContact->init(contact);
    dContact->showNormal();        
}

//朗读结束
void DialogMsgManage::slotFinishRead()
{
    qDebug() << "finish read" << __FILE__ << __LINE__;
    m_actRead->setVisible(true);
    m_actStopRead->setVisible(false);
}

//查看未读
void DialogMsgManage::slotActionViewUnread()
{
    m_isViewUnread = true;
    ui->listWidget->clear();
    ui->btnFirstPage->setEnabled(false);
    ui->btnLastPage->setEnabled(false);
    ui->btnNextPage->setEnabled(false);
    ui->btnPrevPage->setEnabled(false);
    ui->lineEditCurPage->setEnabled(false);
    ui->lineEditCurPage->setText("1");
    ui->labelPageNum->setText("1");
    
    QList<Message> msgs = getAllMessagesOfTreeItem();
    foreach (Message msg, msgs) {
        if (msg.state == Message::State_Unread) {
            addMsg(msg, DataType_MessageBox);
        }
    }
}

//设置所有已读
void DialogMsgManage::slotActionSetAllRead()
{
    QList<Message> msgs = getAllMessagesOfTreeItem();
    if (!msgs.isEmpty()) {
        foreach (Message msg, msgs) {
            if (msg.state == Message::State_Unread) {
                g_msgbox->setRead(msg.id, true);
            }
        }
    }
//    updateMsgList();
}

void DialogMsgManage::slotActionExport()
{
    MsgTool ie;
    QList<Message> msgs = getAllMessagesOfTreeItem();
    if (msgs.count() > 0) {
        ie.exportMsg(getAllMessagesOfTreeItem());
    } else {
        QMessageBox::information(NULL, tr("msg"), tr("Messag is empty"));
    }
}

//删除当前树选择对象的全部信息，如果是垃圾箱，则彻底删除
void DialogMsgManage::slotActionDeleteAll()
{
    if (QMessageBox::No == QMessageBox::information(
                this, tr("msg"), tr("Are you sure to delete?")
                , QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        return;
    }
    
    QModelIndex index = ui->treeWidget->currentIndex();
    bool isThroughDelete = false;
    if (index.data(Qt::UserRole + DataIndex_Type).toInt() == DataType_MessageBox) {            
        if (index.data(Qt::UserRole + DataIndex_Value).toInt() == Message::Box_Dustbin) {
            isThroughDelete = true;
        }
    }
    QList<Message> msgs = getAllMessagesOfTreeItem();
    if (isThroughDelete) {
        foreach (Message msg, msgs) {
            g_msgbox->removeMessage(msg.id);
        }
    } else {
        foreach (Message msg, msgs) {
            if (msg.box != Message::Box_Dustbin) {
                g_msgbox->changeBox(msg.id, Message::Box_Dustbin);
                if (ui->checkBoxRecord->isChecked()) {
                    //识别
                    g_MessageIdentification->MessageIdentificationProcessAndSave(msg.content);
                    g_PhoneNumberIdentification->NumberIdentificationProcessAndSave(msg.phonenum);
                    g_NumberSegmentIdentification->NSIProcessAndSave(msg.phonenum.toULongLong());
                }
            }
        }
    }    
}

//删除指定日期前的信息
void DialogMsgManage::slotActionDeleteMoreSetting()
{
    DialogDeleteMsg *dDeleteMsg = new DialogDeleteMsg(this);
    connect(dDeleteMsg, SIGNAL(signDeleteMsgsBefore(QDateTime)), this, SLOT(slotDeleteMsgsBefore(QDateTime)));
    dDeleteMsg->exec();
}

//删除指定日期前的信息
void DialogMsgManage::slotDeleteMsgsBefore(const QDateTime &dateTime)
{
    if (QMessageBox::No == QMessageBox::information(
                this, tr("msg"), tr("Are you sure to delete?")
                , QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        return;
    }      
    
    QModelIndex index = ui->treeWidget->currentIndex();
    bool isThroughDelete = false;
    if (index.data(Qt::UserRole + DataIndex_Type).toInt() == DataType_MessageBox) {            
        if (index.data(Qt::UserRole + DataIndex_Value).toInt() == Message::Box_Dustbin) {
            isThroughDelete = true;
        }
    }
    QList<Message> msgs = getAllMessagesOfTreeItem();
    if (isThroughDelete) {
        foreach (Message msg, msgs) {
            if (msg.datetime < dateTime) {
                g_msgbox->removeMessage(msg.id);
            }
        }
    } else {
        foreach (Message msg, msgs) {
            if (msg.box != Message::Box_Dustbin && msg.datetime < dateTime) {
                g_msgbox->changeBox(msg.id, Message::Box_Dustbin);
                if (ui->checkBoxRecord->isChecked()) {
                    //识别
                    g_MessageIdentification->MessageIdentificationProcessAndSave(msg.content);
                    g_PhoneNumberIdentification->NumberIdentificationProcessAndSave(msg.phonenum);
                    g_NumberSegmentIdentification->NSIProcessAndSave(msg.phonenum.toULongLong());
                }
            }
        }
    }  
}

//获取树项的信息
QList<Message> DialogMsgManage::getAllMessagesOfTreeItem()
{
    QModelIndex index = ui->treeWidget->currentIndex();
    QVariant var = index.data(Qt::UserRole + DataIndex_Type);    
    QList<Message> msgs;
    if (!var.isNull()) {
        if (var.toInt() == DataType_MessageBox) {            
            Message::Box box = (Message::Box)index.data(Qt::UserRole + DataIndex_Value).toInt();
            msgs = g_msgbox->getMessagesOfBox(box);
        } else if (var.toInt() == DataType_PhoneBook) {
            Contact contact = index.data(Qt::UserRole + DataIndex_Value).value<Contact>();
            msgs = g_msgbox->getMessagesOfPhoneNum(contact.phonenum);
        } else {
            qDebug() << "this is a bug" << __FILE__ << __LINE__;
        }
    }
    return msgs;
}

void DialogMsgManage::on_btnFirstPage_clicked()
{
    ui->lineEditCurPage->setText("1");
    updateMsgList();
}

void DialogMsgManage::on_btnPrevPage_clicked()
{
    int currentPage = ui->lineEditCurPage->text().toInt() - 1;
    ui->lineEditCurPage->setText(QString::number(currentPage));    
    updateMsgList();        
}

void DialogMsgManage::on_btnNextPage_clicked()
{
    int currentPage = ui->lineEditCurPage->text().toInt() + 1;
    ui->lineEditCurPage->setText(QString::number(currentPage));    
    updateMsgList();        
}

void DialogMsgManage::on_btnLastPage_clicked()
{
    ui->lineEditCurPage->setText(ui->labelPageNum->text());
    updateMsgList();        
}

void DialogMsgManage::on_lineEditCurPage_returnPressed()
{
    updateMsgList();    
}

void DialogMsgManage::on_listWidget_itemDoubleClicked(QListWidgetItem* item)
{
    DataType dataType = (DataType)item->data(Qt::UserRole + DataIndex_Type).toInt();
    Message msg = item->data(Qt::UserRole + DataIndex_Value).value<Message>();
    if (dataType==DataType_MessageBox && msg.state==Message::State_Unread) {
        g_msgbox->setRead(msg.id, true);
    }
//    updateMsgList();
}

void DialogMsgManage::on_listWidget_customContextMenuRequested(QPoint pos)
{
    QModelIndex index;
    if ((index = ui->listWidget->currentIndex()).isValid() && !ui->listWidget->selectedItems().isEmpty()) {
        DataType dataType = (DataType)index.data(Qt::UserRole + DataIndex_Type).toInt();
        Message msg = index.data(Qt::UserRole + DataIndex_Value).value<Message>();
        
        bool isSpeaking = g_speech->getStatus()==QtSpeech::State_Speaking;
        bool isMultiSelected = ui->listWidget->selectedItems().count() > 1;
        m_actForwarding->setVisible(!isMultiSelected);
        m_actReply->setVisible(!isMultiSelected);
        m_actRead->setVisible(!isSpeaking && !isMultiSelected);
        m_actStopRead->setVisible(isSpeaking);
        
//        m_actMoveToInbox->setVisible(false);
//        m_actMoveToOutbox->setVisible(false);
//        m_actMoveToDraftbox->setVisible(false);
//        m_actMoveToDustbin->setVisible(false);
        m_actAddIntoPhoneBook->setVisible(false);
        
        if (dataType == DataType_MessageBox) {
//            switch (msg.msgtype) {
//            case Message::MsgType_InMsg:
//                if (msg.box == Message::Box_Inbox) {
//                    m_actMoveToDustbin->setVisible(true);            
//                } else {
//                    m_actMoveToInbox->setVisible(true);                        
//                }
//                break;
//            case Message::MsgType_OutMsg:
//                m_actReply->setVisible(false);
//                if (msg.box == Message::Box_Dustbin) {
//                    m_actMoveToOutbox->setVisible(true);
//                    m_actMoveToDraftbox->setVisible(true);
//                } else {
//                    m_actMoveToDustbin->setVisible(true);
//                }
//                break;
//            default:
//                break;
//            }
            if (msg.msgtype==Message::MsgType_OutMsg) {
                m_actReply->setVisible(false);
            }                    
            
            bool isValid = g_phoneBook->getContactOfPhoneNum(msg.phonenum).isValid();
            m_actAddIntoPhoneBook->setVisible(!isValid);
        }
        
        int curBox = ui->treeWidget->currentIndex().data(Qt::UserRole + DataIndex_Value).toInt();
        m_actRestore->setVisible(curBox==Message::Box_Dustbin);
        
        m_menuMsgList->exec(QCursor::pos());
    }
}

void DialogMsgManage::on_treeWidget_clicked(QModelIndex index)
{
    m_isViewUnread = false;
    m_setReadMsgIds.clear();
    
    int recordNum = getAllMessagesOfTreeItem().count();
    int pageNum = recordNum / cons_msg_num_per_page + (recordNum % cons_msg_num_per_page != 0);
    ui->labelPageNum->setText(QString::number(pageNum));
    on_btnLastPage_clicked();
}

void DialogMsgManage::on_treeWidget_customContextMenuRequested(QPoint pos)
{
    m_isViewUnread = false;
    m_setReadMsgIds.clear();
          
    QModelIndex index = ui->treeWidget->currentIndex();
    QVariant var = index.data(Qt::UserRole + DataIndex_Type);
    if (index.isValid() && !var.isNull()) {
        m_actViewUnread->setVisible(false);
        m_actSetAllRead->setVisible(false);
        
        DataType dataType = (DataType)var.toInt();
        if (dataType == DataType_MessageBox) {
            Message::Box box = (Message::Box)index.data(Qt::UserRole + DataIndex_Value).toInt();
            if (box==Message::Box_Inbox || box==Message::Box_Dustbin) {
                m_actViewUnread->setVisible(true);
                m_actSetAllRead->setVisible(true);                
            }
        }
        m_menuTree->exec(QCursor::pos());
    }
}

void DialogMsgManage::on_btnImport_clicked()
{
    MsgTool ie;
    ie.importMsg();
}

void DialogMsgManage::on_btnExport_clicked()
{
    MsgTool ie;
    QList<Message> msgs = g_msgbox->getAllMessages();
    if (!msgs.isEmpty()) {
        ie.exportMsg(msgs);
    } else {
        QMessageBox::information(NULL, tr("msg"), tr("Messag is empty"));        
    }
}

void DialogMsgManage::on_btnRepair_clicked()
{
    MsgTool ie;
    ie.repairMsg();    
}
