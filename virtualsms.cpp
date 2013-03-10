/************************************************
类名: VirtualSMS
功能: 主界面
说明: 主界面以标签页显示，分为电话本和信息2个标签页，主界面
     下方工具栏提供设置、添加联系人、导入三个按钮。通过qt的
     信号和槽的机制把数据的变更信号与主界面的更新槽连接起来，
     在界面上的操作最终会通过数据管理类来操作数据，然后数据
     管理类发送变更信号，通知主界面进行更新
************************************************/

#include "virtualsms.h"
#include "ui_virtualsms.h"

#include "itemdelegate.h"
#include "dialogsetting.h"
#include "dialogwritemsg.h"
#include "dialogwritemsg.h"
#include "dialogcontact.h"

#include "messagereceiver.h"
#include "phonebook.h"
#include "setting.h"
#include "msgtool.h"
#include "msgbox.h"
#include "QtSpeech.h"
#include "udpbroadcast.h"
#include "DistinguishPhrase.h"
#include "MessageIdentification.h"
#include "phonenumberidentification.h"

const char* cons_expand_property = "Expand";
const int cons_msg_list_item_num = 100;

VirtualSMS::VirtualSMS(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VirtualSMS),
    m_dMsgDetail(NULL),
    m_dMsgManage(NULL),
    m_remindFlag(false)
{
    ui->setupUi(this);
    
    m_isInit = false;
    qApp->installTranslator(&m_tr);
    qApp->installTranslator(&m_trInner);
    //翻译
    slotChangeLanguage();
    //初始化
    init();    
    //翻译界面
    retranslateUi();
    //检查冲突
    g_udpbroadcast->checkConflict();
}

VirtualSMS::~VirtualSMS()
{
    delete m_dMsgDetail;
    delete m_dMsgManage;
    delete ui;
}

void VirtualSMS::showEvent(QShowEvent *se)
{
    QMainWindow::showEvent(se);
    updatePhoneBookList();
}

void VirtualSMS::closeEvent(QCloseEvent *ce)
{
    if (!g_setting->getIsCloseMin()) {
        slotQuit();
    }
    QMainWindow::closeEvent(ce);
}

//事件过滤
bool VirtualSMS::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Delete) {
            if (m_msgListWidgets.indexOf(qobject_cast<QListWidget*>(o)) != -1) {
                slotActionRemoveMsg();
            } else if (m_phonebookListWidgets.indexOf(qobject_cast<QListWidget*>(o)) != -1) {
                slotActionDeleteContact();
            }
        }
    } else if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if (me->button() == Qt::RightButton) {
            if (m_phonebookBtns.indexOf(qobject_cast<QToolButton*>(o)) != -1) {
                slotProcessPhoneBookBtnContextMenu();
            }
        }
    }
    
    //处理联系人搜索
    if (o == m_popup) {
        if (e->type() == QEvent::MouseButtonPress) {
            m_popup->hide();
            ui->lineEditSearch->setFocus();
            return true;
        }
        if (e->type() == QEvent::KeyPress) {
            
            bool consumed = false;
            int key = static_cast<QKeyEvent*>(e)->key();
            switch (key) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                slotDoneCompletion();
                consumed = true;
                
            case Qt::Key_Escape:
                ui->lineEditSearch->setFocus();
                m_popup->hide();
                consumed = true;
                
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Home:
            case Qt::Key_End:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
                break;                
            default:
                ui->lineEditSearch->setFocus();
                ui->lineEditSearch->event(e);
                m_popup->hide();
                break;
            }
            return consumed;
        }
        return false;
    }
    
    return QMainWindow::eventFilter(o, e);
}

//重写设置可见
void VirtualSMS::setVisible(bool visible)
{
    m_minAction->setEnabled(visible);              //主窗口可见时，最小化有效
    m_restoreAction->setEnabled(!visible);         //主窗口不可见时，还原有效

    QMainWindow::setVisible(visible);                //调用基类函数
}

//初始化
void VirtualSMS::init()
{   
    g_phoneBook->init();
    if (!g_phoneBook->getMyself().isValid()) {
        QMessageBox::information(this, tr("msg"), tr("No myself information in phonebook"));
    }
    g_PhoneNumberIdentification;
    g_msgbox->init();
    QApplication::setQuitOnLastWindowClosed(false);
    connect(g_setting, SIGNAL(signLanguageChanged()), this, SLOT(slotChangeLanguage()));
    connect(g_setting, SIGNAL(signRunModeChanged()), this, SLOT(slotChangeRunMode()));
    m_timerRemind.setInterval(500);
    connect(&m_timerRemind, SIGNAL(timeout()), this, SLOT(slotFlicker()));
    m_timerCheck.setInterval(3 * 1000);
    connect(&m_timerCheck, SIGNAL(timeout()), this, SLOT(slotCheck()));
    connect(g_udpbroadcast, SIGNAL(signPhoneNumConflict(QString,QHostAddress)),
            this, SLOT(slotPhoneNumConflict(QString,QHostAddress)));
    connect(g_udpbroadcast, SIGNAL(signContactLogin(QString)),
            this, SLOT(slotContactLogin(QString)));
    
    layout()->setMargin(0);
    //加载qss
    QFile file(":/qss/VirtualSMS.qss");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
    } else {
        setStyleSheet(file.readAll());
    }
    file.close();    
    
    //初始化托盘
    initTrayIcon();
    //初始化接收单例
    connect(g_receiver, SIGNAL(signNewMsgArrived(QString,QString)), 
            this, SLOT(slotProcessNewInMsg(QString,QString)));

    //初始化标签栏
    ui->tabWidget->setIconSize(QSize(cons_tab_size * 0.7, cons_tab_size * 0.7));
    QIcon iconPhoneBook(":/pic/PhoneBook.png");
    ui->tabWidget->setTabIcon(Tab_PhoneBook, iconPhoneBook);
    QIcon iconMessage(":/pic/Message.png");
    ui->tabWidget->setTabIcon(Tab_Message, iconMessage);
    
    //电话本按钮，添加联系人时需要用到，所以不能放到init函数最后
    m_phonebookBtns << ui->btnOther << ui->btnWhitelist << ui->btnBlacklist << ui->btnStranger;
    m_phonebookGroupBoxs << ui->groupBoxOther << ui->groupBoxWhitelist
                   << ui->groupBoxBlacklist << ui->groupBoxStranger;
    m_phonebookListWidgets << ui->listWidgetOther << ui->listWidgetWhitelist
                     << ui->listWidgetBlacklist << ui->listWidgetStranger;
    foreach (QToolButton *btn, m_phonebookBtns) {
        btn->setFocusPolicy(Qt::ClickFocus);
        btn->setProperty(cons_expand_property, false);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->installEventFilter(this);
        connect(btn, SIGNAL(clicked()), this, SLOT(slotPhonebookBtnClicked()));
    }
    ui->btnOther->click();

    //电话本按钮菜单
    m_actShowAll = new QAction(this);
    connect(m_actShowAll, SIGNAL(triggered()), this, SLOT(slotActionShowAll()));
    m_actShowOnline = new QAction(this);
    connect(m_actShowOnline, SIGNAL(triggered()), this, SLOT(slotActionShowOnline()));
    m_actShowAll->setVisible(false);
    m_menuPhoneBookBtn= new QMenu(this);
    m_menuPhoneBookBtn->addAction(m_actShowAll);
    m_menuPhoneBookBtn->addAction(m_actShowOnline);
    
    //初始化电话本列表
    QList<Contact> contacts = g_phoneBook->getAllContacts();
    foreach (Contact contact, contacts) {
        addContact(contact.id);
    }
    //需使用队列连接的方式，异步更新，否则会因写保护而不能读取，导致阻塞
    connect(g_phoneBook, SIGNAL(signContactChanged(int, int)), 
            this, SLOT(slotUpdatePhoneBook(int, int)), Qt::QueuedConnection);
    
    //暂时全部获取
    bool hasUnread = false;
    QList<Message> msgsInbox = g_msgbox->getLastMessages(cons_msg_list_item_num, Message::Box_Inbox);
    foreach (Message msg, msgsInbox) {
        if (msg.state==Message::State_Unread && !hasUnread) {
            hasUnread = true;
        }
        addMsg(msg);
    }
    if (hasUnread) {
        m_trayIcon->showMessage(tr("msg"), tr("Inbox has unread message"));
    }
    QList<Message> msgsOutbox = g_msgbox->getLastMessages(cons_msg_list_item_num, Message::Box_Outbox);
    foreach (Message msg, msgsOutbox) {
        addMsg(msg);
    }
    QList<Message> msgsDraftbox = g_msgbox->getLastMessages(cons_msg_list_item_num, Message::Box_Draftbox);
    foreach (Message msg, msgsDraftbox) {
        addMsg(msg);
    }
    QList<Message> msgsDustbin = g_msgbox->getLastMessages(cons_msg_list_item_num, Message::Box_Dustbin);
    foreach (Message msg, msgsDustbin) {
        addMsg(msg);
    }
    connect(g_msgbox, SIGNAL(signMsgChanged(int,int,int)),
            this, SLOT(slotUpdateMsg(int,int,int)), Qt::QueuedConnection);    

    //初始化电话本菜单
    m_actGoToChat = new QAction(this);
    connect(m_actGoToChat, SIGNAL(triggered()), this, SLOT(slotActionGotoChat()));
    m_actSendMsg = new QAction(this);
    connect(m_actSendMsg, SIGNAL(triggered()), this, SLOT(slotActionSendMsg()));
    m_actViewContactDetail = new QAction(this);
    connect(m_actViewContactDetail, SIGNAL(triggered()), this, SLOT(slotActionViewContactDetail()));
    m_actMoveToBlackList = new QAction(this);
    connect(m_actMoveToBlackList, SIGNAL(triggered()), this, SLOT(slotActionMoveToBlackList()));
    m_actMoveToWhiteList = new QAction(this);
    connect(m_actMoveToWhiteList, SIGNAL(triggered()), this, SLOT(slotActionMoveToWhiteList()));
    m_actMoveToOther = new QAction(this);
    connect(m_actMoveToOther, SIGNAL(triggered()), this, SLOT(slotActionMoveToOther()));
    m_actMoveToStranger = new QAction(this);
    connect(m_actMoveToStranger, SIGNAL(triggered()), this, SLOT(slotActionMoveToStranger())); 
    m_actDeleteContact = new QAction(this);
    connect(m_actDeleteContact, SIGNAL(triggered()), this, SLOT(slotActionDeleteContact())); 
    m_menuMoveTo = new QMenu(this);
    m_menuPhoneBook = new QMenu(this);
    m_menuMoveTo->addAction(m_actMoveToBlackList);
    m_menuMoveTo->addAction(m_actMoveToWhiteList);
    m_menuMoveTo->addAction(m_actMoveToOther);
    m_menuMoveTo->addAction(m_actMoveToStranger);
    m_menuPhoneBook->addAction(m_actGoToChat);
    m_menuPhoneBook->addAction(m_actSendMsg);
    m_menuPhoneBook->addAction(m_actViewContactDetail);
    m_menuPhoneBook->addMenu(m_menuMoveTo);
    m_menuPhoneBook->addAction(m_actDeleteContact);
    
    //信箱菜单
    m_actViewMsgDetail = new QAction(this);
    connect(m_actViewMsgDetail, SIGNAL(triggered()), this, SLOT(slotActionViewMsgDetail()));
    m_actEditUnsendMsg = new QAction(this);
    connect(m_actEditUnsendMsg, SIGNAL(triggered()), this, SLOT(slotActionEditUnsendMsg()));
    m_actGotoChatFromMsg = new QAction(this);
    connect(m_actGotoChatFromMsg, SIGNAL(triggered()), this, SLOT(slotActionGotoChatFromMsg()));
    m_actRead = new QAction(this);
    connect(m_actRead, SIGNAL(triggered()), this, SLOT(slotActionRead()));
    m_actStopRead = new QAction(this);
    connect(m_actStopRead, SIGNAL(triggered()), this, SLOT(slotActionStopRead()));
    m_actForwarding = new QAction(this);
    connect(m_actForwarding, SIGNAL(triggered()), this, SLOT(slotActionForwarding()));
    m_actReply = new QAction(this);
    connect(m_actReply, SIGNAL(triggered()), this, SLOT(slotActionReply()));
    m_actMoveToInbox = new QAction(this);
    connect(m_actMoveToInbox, SIGNAL(triggered()), this, SLOT(slotActionMoveToInbox()));
    m_actMoveToOutbox = new QAction(this);
    connect(m_actMoveToOutbox, SIGNAL(triggered()), this, SLOT(slotActionMoveToOutbox()));
    m_actMoveToDraftbox = new QAction(this);
    connect(m_actMoveToDraftbox, SIGNAL(triggered()), this, SLOT(slotActionMoveToDraftbox()));
//    m_actMoveToDustbin = new QAction(this);
//    connect(m_actMoveToDustbin, SIGNAL(triggered()), this, SLOT(slotActionMoveToDustbin()));
    m_actAddIntoPhoneBook = new QAction(this);
    connect(m_actAddIntoPhoneBook, SIGNAL(triggered()), this, SLOT(slotActionAddIntoPhoneBook()));
    m_actViewMoreMsg = new QAction(this);
    connect(m_actViewMoreMsg, SIGNAL(triggered()), this, SLOT(slotActionViewMoreMsg()));
    m_actRemoveMsg = new QAction(this);
    connect(m_actRemoveMsg, SIGNAL(triggered()), this, SLOT(slotActionRemoveMsg()));
    m_menuMsg = new QMenu(this);
    m_menuMsg->addAction(m_actGotoChatFromMsg);
    m_menuMsg->addSeparator();
    m_menuMsg->addAction(m_actViewMsgDetail);
    m_menuMsg->addAction(m_actViewMoreMsg);
    m_menuMsg->addSeparator();
    m_menuMsg->addAction(m_actRead);
    m_menuMsg->addAction(m_actStopRead);
    m_menuMsg->addAction(m_actEditUnsendMsg);
    m_menuMsg->addAction(m_actForwarding);
    m_menuMsg->addAction(m_actReply);
    m_menuMsg->addSeparator();
    m_menuMsg->addAction(m_actAddIntoPhoneBook);
    m_menuMsg->addSeparator();
    m_menuMsg->addAction(m_actMoveToInbox);
    m_menuMsg->addAction(m_actMoveToOutbox);
    m_menuMsg->addAction(m_actMoveToDraftbox);
//    m_menuMsg->addAction(m_actMoveToDustbin);
    m_menuMsg->addAction(m_actRemoveMsg);
    
    ui->tabWidget->setCurrentWidget(ui->tabPhoneBook);        
    CancelFocus(this);
    
    m_timerSearch.setSingleShot(true);
    m_timerSearch.setInterval(500);
    connect(&m_timerSearch, SIGNAL(timeout()), SLOT(slotAutoSuggest()));
    //此处需用textEdited，因为此信号响应的是人为的编辑，而不响应函数调用的改变，比如setText
    connect(ui->lineEditSearch, SIGNAL(textEdited(QString)), &m_timerSearch, SLOT(start()));
    connect(ui->lineEditSearch, SIGNAL(returnPressed()), this, SLOT(slotDoneCompletion()));
    //初始化搜索弹出框
    m_popup = new QListWidget;
    m_popup->setWindowFlags(Qt::Popup);
    m_popup->setFocusPolicy(Qt::NoFocus);
    m_popup->setFocusProxy(this);
    m_popup->setMouseTracking(true);
    m_popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    m_popup->setSelectionBehavior(QTreeWidget::SelectRows);
    m_popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    m_popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_popup->installEventFilter(this);        
    connect(m_popup, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotDoneCompletion()));
    
    ui->lineEditSearch->setFocusPolicy(Qt::ClickFocus);
   
    //电话本列表
    foreach (QListWidget *listWidget, m_phonebookListWidgets) {
        listWidget->setFocusPolicy(Qt::ClickFocus);
        listWidget->installEventFilter(this);
        listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)),
                this, SLOT(slotProcessPhoneBookClicked(QListWidgetItem*)));
        connect(listWidget, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(slotProcessPhoneBookDoubledClicked(QModelIndex)));
        connect(listWidget, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(slotProcessPhoneBookContextMenu()));
    }
    
    //初始化信息栏按钮
    ui->btnWriteMsg->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_msgBtns << ui->btnInbox << ui->btnOutbox << ui->btnDraftBox << ui->btnDustbin;
    m_msgGroupBoxs << ui->groupBoxInbox << ui->groupBoxOutbox
                   << ui->groupBoxDraftbox << ui->groupBoxDustbin;
    m_msgListWidgets << ui->listWidgetInbox << ui->listWidgetOutbox
                     << ui->listWidgetDraftbox << ui->listWidgetDustbin;
    foreach (QToolButton *btn, m_msgBtns) {
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        connect(btn, SIGNAL(clicked()), this, SLOT(slotMsgBtnClicked()));
    }
    ui->btnInbox->click();    

    //信息列表
    foreach (QListWidget *listWidget, m_msgListWidgets) {
        listWidget->setFocusPolicy(Qt::ClickFocus);
        listWidget->installEventFilter(this);
        listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
        listWidget->setItemDelegate(new ItemDelegate(this));
        connect(listWidget, SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(slotProcessMsgDoubledClicked(QModelIndex)));
        connect(listWidget, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(slotProcessMsgContextMenu()));
    }
    
    //电话本添加滚动条
    QScrollArea *view = new QScrollArea;    
    view->setFocusPolicy(Qt::NoFocus);    
    view->setFrameStyle(QFrame::NoFrame);    
    view->setWidgetResizable(true); 
    view->setWidget(ui->groupBoxPhoneBook_1);
    QVBoxLayout *vbLayout = new QVBoxLayout(ui->groupBoxPhoneBook);
    vbLayout->setMargin(0);    
    vbLayout->addWidget(view);
   
    m_isInit = true;
}

//翻译
void VirtualSMS::retranslateUi()
{
    //标签
    ui->tabWidget->setTabToolTip(Tab_PhoneBook, tr("PhoneBook"));
    ui->tabWidget->setTabToolTip(Tab_Message, tr("Message"));    
    
    //信息按钮
    m_strInbox = QT_TR_NOOP("Inbox(%1/%2)");
    m_strOutbox = QT_TR_NOOP("Outbox(%1)");
    m_strDraftBox = QT_TR_NOOP("DraftBox(%1)");
    m_strDustbin = QT_TR_NOOP("Dustbin(%1/%2)");
    m_strImportMsg = QT_TR_NOOP("Import success. Import message count:%1");    
    updateMsgBoxNum();
    
    //电话本按钮菜单
    m_actShowAll->setText(tr("ShowAll"));
    m_actShowOnline->setText(tr("ShowOnline"));
    
    //电话本菜单
    m_actGoToChat->setText(tr("GoToChat"));
    m_actSendMsg->setText(tr("SendMsg"));
    m_actViewContactDetail->setText(tr("ViewContactDetail"));
    m_actEditUnsendMsg->setText(tr("EditMsg"));
    m_actMoveToBlackList->setText(tr("MoveToBlackList"));
    m_actMoveToWhiteList->setText(tr("MoveToWhiteList"));
    m_actMoveToOther->setText(tr("MoveToOther"));
    m_actMoveToStranger->setText(tr("MoveToStranger"));
    m_actDeleteContact->setText(tr("DeleteContact"));
    m_menuMoveTo->setTitle(tr("MoveTo"));
    
    //信箱菜单
    m_actViewMsgDetail->setText(tr("ViewMsgDetail"));
    m_actGotoChatFromMsg->setText(tr("GoToChat"));
    m_actRead->setText(tr("Read"));
    m_actStopRead->setText(tr("StopRead"));
    m_actForwarding->setText(tr("Forwarding"));
    m_actReply->setText(tr("Reply"));
    m_actMoveToInbox->setText(tr("MoveToInbox"));
    m_actMoveToOutbox->setText(tr("MoveToOutbox"));
    m_actMoveToDraftbox->setText(tr("MoveToDraftbox"));
//    m_actMoveToDustbin->setText(tr("MoveToDustbin"));
    m_actAddIntoPhoneBook->setText(tr("AddIntoPhoneBook"));
    m_actViewMoreMsg->setText(tr("GoToMsgManage"));
    m_actRemoveMsg->setText(tr("RemoveMsg"));
    
    //托盘
    m_minAction->setText(tr("min"));
    m_restoreAction->setText(tr("restore"));
    m_quitAction->setText(tr("quit"));
    if (m_newInMsgPhoneNumToIds.isEmpty()) {
        m_trayIcon->setToolTip(tr("VirtualSMS"));
    }
}

//取消焦点
void VirtualSMS::CancelFocus(QWidget *w)
{
    if(!w) {
        return;
    }
    QObjectList list = w->children();
    foreach(QObject *obj, list) {
        if(!obj->isWidgetType()) {
            continue;
        }
        CancelFocus((QWidget*)obj);
    }
    w->setFocusPolicy(Qt::NoFocus);
}

//初始化托盘
void VirtualSMS::initTrayIcon()
{
    //创建并设置动作
    m_minAction = new QAction(this);
    connect(m_minAction,SIGNAL(triggered()), this, SLOT(hide()));

    m_restoreAction = new QAction(this);
    connect(m_restoreAction,SIGNAL(triggered()), this, SLOT(slotOpen()));

    m_quitAction = new QAction(this);
    connect(m_quitAction,SIGNAL(triggered()), this, SLOT(slotQuit()));

    //创建托盘图标菜单并添加动作
    m_trayIconMenu = new QMenu(QApplication::desktop()); // 不设置为this，这样点击菜单外的任何位置，菜单会消失
    m_trayIconMenu->addAction(m_restoreAction);
    m_trayIconMenu->addAction(m_minAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    //创建并设置托盘图标
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setIcon(QIcon(":/pic/Tray.png"));

    //显示系统托盘图标
    m_trayIcon->show();

    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(slotTrayIconActivated(QSystemTrayIcon::ActivationReason)));
}

//获取托盘位置
QRect VirtualSMS::getTrayIconRect()
{
    return m_trayIcon->geometry();
}

/**************************************************
根据当前运行模式和来信人信息响应点击系统托盘事件
运行模式	分类             效果
聊天	联系人来信	进入聊天
	陌生人来信	进入信息标签页，显示收件箱列表信息		
短信	所有来信          进入信息标签页，显示收件箱列表信息
****************************************************/
void VirtualSMS::slotTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        if (m_newInMsgPhoneNumToIds.isEmpty()) {
            if (isMinimized() || !isVisible() 
                    || ((windowState()&Qt::WindowActive) != Qt::WindowActive)) {
                slotOpen();
            }
        } else {
            if (g_setting->getRunMode() == RunMode_Chat) {
                foreach (QString phoneNum, m_newInMsgPhoneNumToIds.keys()) {
                    Contact contact = g_phoneBook->getContactOfPhoneNum(phoneNum);
                    if(contact.isValid()) {
                        //打开聊天界面，设置消息已读                        
                        openChatRoom(contact);
                        DialogChatRoom *dcRoom = qobject_cast<DialogChatRoom*>(m_idToChatRoom.value(contact.id));
                        foreach (int id, m_newInMsgPhoneNumToIds.values(phoneNum)) {
                            dcRoom->addMsg(g_msgbox->getMessageOfId(id));
                            g_msgbox->setRead(id, true);
                        }
                    } else {
                        slotOpen();
                        ui->tabWidget->setCurrentWidget(ui->tabMessage);
                        ui->btnInbox->click();
                    }
                }
                m_timerRemind.stop();
            } else {
                slotOpen();
                ui->tabWidget->setCurrentWidget(ui->tabMessage);
                ui->btnInbox->click();
            }
            m_newInMsgPhoneNumToIds.clear();
            m_trayIcon->setIcon(QIcon(":/pic/Tray.png"));
            m_trayIcon->setToolTip(tr("VirtualSMS"));
        }
        break;
    default:
        break;
    }
}

//显示主窗口
void VirtualSMS::slotOpen()
{
    activateWindow();
    showNormal();
}

//退出
void VirtualSMS::slotQuit()
{
    if (QMessageBox::Yes == QMessageBox::information(
                this, tr("msg"), tr("Are you sure to quit?")
                , QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        g_udpbroadcast->broadcastInformation(Command_Quit);
        g_msgbox->save();
        g_phoneBook->save();
        QApplication::quit();
    }
}

/************************************************
处理新消息，根据运行模式选择提示方式
当新消息到来时，如果新消息没有被过滤为垃圾短信，系统托盘图标
会因运行模式的不同而有不同的反应，当前运行模式为聊天模式时，
托盘图标会闪烁，当前运行模式是短信模式时，托盘图标会变为新短
信到来的图标
************************************************/
void VirtualSMS::slotProcessNewInMsg(const QString &phoneNum, const QString &content)
{
    //黑白名单过滤
    Filter filter;
    Message::Box box =  filter.messageFilter(phoneNum, content);

    //内容识别
    if (box == Message::Box_Inbox && g_setting->getIsIdentifyKeyword()) {
        MIInformation miinfo = g_MessageIdentification->messageIdentification(content);
        qDebug() << "msg identify" << miinfo.keyWord << miinfo.count << miinfo.state << __FILE__ << __LINE__;
        if(miinfo.state == Caution)
        {
            //提醒，存入垃圾箱
            identifyRemind(tr("Message content matches identify condition becauser it contains keyword \"%1\"")
                           .arg(miinfo.keyWord));
            box = Message::Box_Dustbin;
        }
        else if(miinfo.state == Refuse)
        {
            //直接存入垃圾箱
            box = Message::Box_Dustbin;
        }                        
    }
    
    if (box==Message::Box_Inbox && g_setting->getIsIdentifyNumber()) {
        //号码识别
        PNIInformation pNIInformation;
        pNIInformation = g_PhoneNumberIdentification->numberIdentification(phoneNum);
        if(pNIInformation.state == Caution)
        {
            //提醒，存入垃圾箱
            identifyRemind(tr("Phone number(%1) matches the phonenumber identify condition.").arg(phoneNum));
            box = Message::Box_Dustbin;
        }
        else if(pNIInformation.state == Refuse)
        {
            //直接存入垃圾箱
            box = Message::Box_Dustbin;
        }
    }
            
    if (box==Message::Box_Inbox && g_setting->getIsIdentifyNumberSegment()) {
        //号码段识别
        NumberSegmentInformation numseg = g_NumberSegmentIdentification->numberSegmentIdentification(phoneNum.toULongLong());
        if(numseg.state == Caution)
        {
            //提醒，存入垃圾箱
            identifyRemind(tr("Phone number(%1) matches number segment identify condition. Number segment:%2-%3")
                           .arg(phoneNum)
                           .arg(numseg.startNumber)
                           .arg(numseg.endNumber));
            box = Message::Box_Dustbin;
        }
        else if(numseg.state == Refuse)
        {
            //直接存入垃圾箱
            box = Message::Box_Dustbin;
        }
    }
    
    int id = g_msgbox->addMessage(phoneNum,
                                  QDateTime::currentDateTime(),
                                  content,
                                  Message::MsgType_InMsg,
                                  box,
                                  Message::State_Unread);
    qDebug() << id << phoneNum << content << box << __FILE__ << __LINE__;
    if (id == -1) {
        qDebug() << "add msg failed" << __FILE__ << __LINE__;
    } else {
        if (box == Message::Box_Inbox) {                        
            Contact contact = g_phoneBook->getContactOfPhoneNum(phoneNum);
            if (g_setting->getRunMode() == RunMode_Chat) {
                if (contact.isValid()) {
                    DialogChatRoom *dcRoom = qobject_cast<DialogChatRoom*>(m_idToChatRoom.value(contact.id));
                    if (dcRoom == NULL) {
                        m_newInMsgPhoneNumToIds.insertMulti(phoneNum, id);
                        QString strToolTip;
                        qDebug() << m_newInMsgPhoneNumToIds << __FILE__ << __LINE__;
                        QStringList phoneNums = m_newInMsgPhoneNumToIds.keys();
                        foreach (QString phoneNumT, phoneNums) {
                            Contact contactT = g_phoneBook->getContactOfPhoneNum(phoneNumT);
                            if (contactT.isValid()) {
                                strToolTip += QString("%1(%2)\n").arg(contactT.name).arg(contactT.phonenum);
                            } else {
                                strToolTip += QString("%1\n").arg(phoneNumT);
                            }
                        }
                        m_trayIcon->setToolTip(strToolTip);
                        m_timerRemind.start();
                    } else {
                        g_msgbox->setRead(id, true);
                        dcRoom->addMsg(g_msgbox->getMessageOfId(id));
                        if (!(dcRoom->windowState() & Qt::WindowActive)) {
                            QApplication::alert(dcRoom);
                        }
                    }
                } else {
                    m_timerRemind.start();
                }
            } else {
                QString strShowMsg;
                if (contact.isValid()) {
                    strShowMsg = QString("%1:%2").arg(contact.name).arg(content);
                } else {
                    strShowMsg = QString("%1:%2").arg(phoneNum).arg(content);
                }
                m_trayIcon->showMessage(tr("New Msg"), strShowMsg);        
                m_trayIcon->setIcon(QIcon(":/pic/Tray-NewMsg.png"));
            }
        } else if (box == Message::Box_Dustbin) {
            Contact contact = g_phoneBook->getContactOfPhoneNum(phoneNum);
            if (contact.isValid() && m_idToChatRoom.value(contact.id) != NULL) {
                m_trayIcon->showMessage(tr("Caution"), tr("%1(%2) send a new message to you. But it saved into dustbin")
                                        .arg(contact.name).arg(contact.phonenum));
            }
        }
    }
}

//智能识别提醒
IdentificationState VirtualSMS::identifyRemind(const QString &tip)
{
    QMessageBox msgBox;
    QPushButton *allow = msgBox.addButton(tr("Allow"), QMessageBox::ActionRole);
    QPushButton *refuse = msgBox.addButton(tr("Refuse"), QMessageBox::ActionRole);
    QPushButton *caution = msgBox.addButton(tr("Caution"), QMessageBox::ActionRole);
    msgBox.setText(tip + tr("\nAllow: message will save in inbox directly afterwards.\nRefuse: message will save in dustbin afterwards.\nCaution: message will save in dustbin next time and remind user."));
    msgBox.exec();
   
    if (msgBox.clickedButton() == allow) {
        return Allow;
    } else if (msgBox.clickedButton() == refuse) {
        return Refuse;
    } else if (msgBox.clickedButton() == caution) {
        return Caution;
    }
}

//托盘闪烁
void VirtualSMS::slotFlicker()
{
    if (m_remindFlag) {
        m_trayIcon->setIcon(QIcon(":/pic/Tray.png"));
        m_remindFlag = false;
    } else {
        m_trayIcon->setIcon(QIcon(""));
        m_remindFlag = true;
    }    
}

/************************************************
更新电话本，各事件说明如下
PhoneBook::ChangeEvent_Add:添加联系人
PhoneBook::ChangeEvent_Remove:删除联系人
PhoneBook::ChangeEvent_Modify:修改联系人信息
PhoneBook::ChangeEvent_ChangeType:修改联系人类型
PhoneBook::ChangeEvent_ChangeState:状态变更
************************************************/
void VirtualSMS::slotUpdatePhoneBook(int id, int changeEvent)
{
    switch (changeEvent) {
    case PhoneBook::ChangeEvent_Add:
        addContact(id);
        break;
    case PhoneBook::ChangeEvent_Remove:
        removeContact(id);
        break;
    case PhoneBook::ChangeEvent_ChangeName:
    case PhoneBook::ChangeEvent_ChangePhoneNum:
    case PhoneBook::ChangeEvent_ChangeState:
        modifyContact(id);
        break;
    case PhoneBook::ChangeEvent_ChangeType:
        changeContactBox(id);
        break;
    default:
        break;
    }
}

//添加联系人至电话本列表
void VirtualSMS::addContact(int id)
{
    Contact contact = g_phoneBook->getContactOfId(id);
    QListWidgetItem *lwItem = new QListWidgetItem(QString("%1<%2>")
                                                  .arg(contact.name).arg(contact.getCurrentPhoneNum()));
    lwItem->setSizeHint(QSize(0, cons_contact_item_height));
    lwItem->setData(Qt::UserRole, QVariant::fromValue<Contact>(contact));
    m_idToChatRoom.insert(contact.id, NULL);
    QListWidget *listWidget = getPhoneBookBox(contact.type);
    if (listWidget == NULL) {
        return;
    }
    if (contact.type == Contact::Type_Myself) {
        listWidget->insertItem(0, lwItem);
    } else {
        listWidget->addItem(lwItem);
    }
    setContactHead(contact, lwItem);
    updatePhoneBookList();
}

//在电话本列表中删除联系人
void VirtualSMS::removeContact(int id)
{
    bool flag = false;
    foreach (QListWidget *listWidget, m_phonebookListWidgets) {
        for (int i=listWidget->count()-1; i>=0; i--) {
            QListWidgetItem *lwItem = listWidget->item(i);
            Contact contact = lwItem->data(Qt::UserRole).value<Contact>();
            if (id == contact.id) {
                listWidget->removeItemWidget(lwItem);
                delete lwItem;
                flag = true;
                break;
            }
        }
        if (flag) {
            break;
        }
    }
    updatePhoneBookList();
}

//修改电话本列表中的联系人信息
void VirtualSMS::modifyContact(int id)
{
    bool flag = false;
    foreach (QListWidget *listWidget, m_phonebookListWidgets) {
        for (int i=listWidget->count()-1; i>=0; i--) {
            QListWidgetItem *lwItem = listWidget->item(i);
            Contact contact = lwItem->data(Qt::UserRole).value<Contact>();
            if (id == contact.id) {
                Contact contactT = g_phoneBook->getContactOfId(id);
                lwItem->setText(QString("%1<%2>").arg(contactT.name).arg(contactT.getCurrentPhoneNum()));
                if (contact.type!=contactT.type) {
                    qDebug() << "this is a bug" << __FILE__ << __LINE__;
                }
                if (contact.state!=contactT.state) {
                    setContactHead(contactT, lwItem);
                }
                lwItem->setData(Qt::UserRole, QVariant::fromValue<Contact>(contactT));
                flag = true;
                break;            
            }
        }
        if (flag) {
            break;
        }
    }
}

//改变联系人的列表
void VirtualSMS::changeContactBox(int id)
{
    removeContact(id);
    addContact(id);
}

//获取电话本列表控件
QToolButton* VirtualSMS::getPhoneBookBtn(int type)
{
    QToolButton *btn(NULL);
    switch (type) {
    case Contact::Type_Myself:
    case Contact::Type_Other:
        btn = ui->btnOther;
        break;
    case Contact::Type_WhiteList:
        btn = ui->btnWhitelist;
        break;
    case Contact::Type_BlackList:
        btn = ui->btnBlacklist;
        break;
    case Contact::Type_Stranger:
        btn = ui->btnStranger;
        break;
    default:
        btn = NULL;
        qDebug() << "this is a bug" << __FILE__ << __LINE__;
        break;        
    }
    return btn;
}

//获取电话本列表控件
QListWidget* VirtualSMS::getPhoneBookBox(int type)
{
    QListWidget *listWidget(NULL);
    switch (type) {
    case Contact::Type_Myself:
    case Contact::Type_Other:
        listWidget = ui->listWidgetOther;
        break;
    case Contact::Type_WhiteList:
        listWidget = ui->listWidgetWhitelist;
        break;
    case Contact::Type_BlackList:
        listWidget = ui->listWidgetBlacklist;
        break;
    case Contact::Type_Stranger:
        listWidget = ui->listWidgetStranger;
        break;
    default:
        listWidget = NULL;
        qDebug() << "this is a bug" << __FILE__ << __LINE__;
        break;        
    }
    return listWidget;
}

//设置联系人头像
void VirtualSMS::setContactHead(const Contact &contact, QListWidgetItem *lwItem)
{
    QString m_iconFileName;
    bool isOnline = contact.state==Contact::State_Online;
    switch (contact.type) {
    case Contact::Type_Myself:
        m_iconFileName = ":/pic/Head.png";
        break;
    case Contact::Type_BlackList:
        m_iconFileName = isOnline ? ":/pic/BlacklistHead.png" : ":/pic/BlacklistHead-Offline";
        break;
    case Contact::Type_WhiteList:
        m_iconFileName = isOnline ? ":/pic/WhitelistHead.png" : ":/pic/WhitelistHead-Offline";
        break;
    case Contact::Type_Other:
        m_iconFileName = isOnline ? ":/pic/OtherHead.png" : ":/pic/OtherHead-Offline";
        break;
    case Contact::Type_Stranger:
        m_iconFileName = ":/pic/Stranger.png";
        break;
    default:
        m_iconFileName = ":/pic/OtherHead.png";
        break;
    }
    QIcon iconContact(m_iconFileName);        
    lwItem->setIcon(iconContact);
}

//获取当前聚焦的电话本列表控件
QListWidget* VirtualSMS::getFocusedPhoneBookBox()
{
    QListWidget *listWidget(NULL);
    if (ui->listWidgetOther->hasFocus()) {
        listWidget = ui->listWidgetOther;
    } else if (ui->listWidgetWhitelist->hasFocus()) {
        listWidget = ui->listWidgetWhitelist;
    } else if (ui->listWidgetBlacklist->hasFocus()) {
        listWidget = ui->listWidgetBlacklist;
    } else if (ui->listWidgetStranger->hasFocus()) {
        listWidget = ui->listWidgetStranger;
    } else {
        qDebug() << "this is a bug" << __FILE__ << __LINE__;
    }
    return listWidget;
}

//获取选中的联系人
Contact VirtualSMS::getSelectedContact()
{
    return getFocusedPhoneBookBox()->currentIndex().data(Qt::UserRole).value<Contact>();
}

//更新电话本按钮图标
void VirtualSMS::updatePhoneBookList()
{
    for (int i=0; i<4; i++) {
        //更新列表高度
        QListWidget *listWidget = m_phonebookListWidgets[i];
        int visibleCount = 0;
        for (int j=listWidget->count()-1; j>=0; j--) {
            if (!listWidget->item(j)->isHidden()) {
                visibleCount++;
            }
        }
        if (visibleCount > 0) {
            //固定列表大小为行高*可见行数
            listWidget->setMaximumHeight(listWidget->item(0)->sizeHint().height() *
                                         visibleCount);
            listWidget->setMinimumHeight(listWidget->item(0)->sizeHint().height() *
                                         visibleCount);            
        } else {
            listWidget->setMaximumHeight(0);
        }
        //设置按钮图标
        bool isExpand = m_phonebookBtns[i]->property(cons_expand_property).toBool();
        QSize btnIconSize(cons_phonebook_btn_size, cons_phonebook_btn_size);
        QString strIcon = isExpand ? ":/pic/DownArrow.png" : ":/pic/RightArrow.png";
        m_phonebookBtns[i]->setIconSize(btnIconSize);
        m_phonebookBtns[i]->setIcon(QIcon(strIcon));
        //列表为空时隐藏
        if (visibleCount == 0) {
            m_phonebookGroupBoxs[i]->hide();
        } else {            
            m_phonebookGroupBoxs[i]->setVisible(isExpand);
        }
    }
}

//进入聊天
void VirtualSMS::slotActionGotoChat()
{
    openChatRoom(getSelectedContact());
}

//发送消息
void VirtualSMS::slotActionSendMsg()
{
    DialogWriteMsg *dwMsg = new DialogWriteMsg;
    dwMsg->initReply(getSelectedContact().phonenum);
    dwMsg->show();      
}

//查看联系人信息
void VirtualSMS::slotActionViewContactDetail()
{
    viewContactDetail(getSelectedContact());
}

//查看联系人信息
void VirtualSMS::viewContactDetail(Contact contact)
{
    DialogContact *dContact = new DialogContact;
    dContact->init(contact);
    dContact->showNormal();        
}

//移至黑名单
void VirtualSMS::slotActionMoveToBlackList()
{
    g_phoneBook->changeType(getSelectedContact().id, Contact::Type_BlackList);
//    g_phoneBook->save();        
}

//移至白名单
void VirtualSMS::slotActionMoveToWhiteList()
{
    g_phoneBook->changeType(getSelectedContact().id, Contact::Type_WhiteList);    
//    g_phoneBook->save();    
}

//移至其他
void VirtualSMS::slotActionMoveToOther()
{
    g_phoneBook->changeType(getSelectedContact().id, Contact::Type_Other);    
//    g_phoneBook->save();    
}

//移至陌生人
void VirtualSMS::slotActionMoveToStranger()
{
    g_phoneBook->changeType(getSelectedContact().id, Contact::Type_Stranger);    
//    g_phoneBook->save();    
}

//删除联系人
void VirtualSMS::slotActionDeleteContact()
{
    Contact contact = getSelectedContact();
    QString str = tr("Are you sure to delete contact:%1(%2)")
            .arg(contact.name).arg(contact.phonenum);
    if (QMessageBox::No == QMessageBox::information(
                this, tr("msg"), str, QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        return;
    }        
    g_phoneBook->removeContact(contact.id);    
//    g_phoneBook->save();
}

void VirtualSMS::slotActionShowAll()
{
    m_actShowAll->setVisible(false);
    m_actShowOnline->setVisible(true);
    setOfflineContactsVisible(true);
}

void VirtualSMS::slotActionShowOnline()
{
    m_actShowAll->setVisible(true);
    m_actShowOnline->setVisible(false);
    setOfflineContactsVisible(false);
}

//是否显示离线联系人
void VirtualSMS::setOfflineContactsVisible(bool isVisible)
{
    foreach (QListWidget *listWidget, m_phonebookListWidgets) {        
        for (int i=listWidget->count()-1; i>=0; i--) {
            QListWidgetItem *lwItem = listWidget->item(i);
            Contact contact = lwItem->data(Qt::UserRole).value<Contact>();
            if (contact.state == Contact::State_Offline) {
                listWidget->setItemHidden(lwItem, !isVisible);
            }
        }
    }
    updatePhoneBookList();
}

//获取当前聚焦的信箱
QListWidget* VirtualSMS::getFocusedMsgBox()
{
    QListWidget *listWidget(NULL);
    if (ui->listWidgetInbox->hasFocus()) {
        listWidget = ui->listWidgetInbox;
    } else if (ui->listWidgetOutbox->hasFocus()) {
        listWidget = ui->listWidgetOutbox;
    } else if (ui->listWidgetDraftbox->hasFocus()) {
        listWidget = ui->listWidgetDraftbox;
    } else if (ui->listWidgetDustbin->hasFocus()) {
        listWidget = ui->listWidgetDustbin;
    } else {
        qDebug() << "this is a bug" << __FILE__ << __LINE__;
    }
    return listWidget;
}

//获取选中的信息
Message VirtualSMS::getSelectedMesssage()
{
    return getFocusedMsgBox()->currentIndex().data(Qt::UserRole).value<Message>();
}

//收件箱、发件箱、垃圾箱共有
//查看信息详情
void VirtualSMS::slotActionViewMsgDetail()
{
    updateMsgBoxNum();
    viewMsgDetail(getSelectedMesssage());
}

//编辑未发送信息
void VirtualSMS::slotActionEditUnsendMsg()
{
    DialogWriteMsg *dWriteMsg = new DialogWriteMsg;
    Message msg = getSelectedMesssage();
    dWriteMsg->initSendUnsendMsg(msg);
    dWriteMsg->showNormal();
    g_msgbox->removeMessage(msg.id);
}

//收件箱、垃圾箱、发件箱共有
//信息朗读
void VirtualSMS::slotActionRead()
{
    Message msg = getSelectedMesssage();
    if (msg.state == Message::State_Unread) {
        g_msgbox->setRead(msg.id, true);
    }
    g_speech->setVoice(g_setting->getVoiceInfo());
    g_speech->tell(msg.content, this, SLOT(slotFinishRead()));
}

//停止朗读
void VirtualSMS::slotActionStopRead()
{
    g_speech->stop();
}

//从信息处进入聊天
void VirtualSMS::slotActionGotoChatFromMsg()
{
    Contact contact = g_phoneBook->getContactOfPhoneNum(getSelectedMesssage().phonenum);
    if (contact.isValid()) {
        openChatRoom(contact);
    } else {
        QMessageBox::information(this, tr("msg"), tr("Can't chat with unfamiliar phonenum!"));
    }
}

//收件箱、垃圾箱、发件箱共有
//信息转发
void VirtualSMS::slotActionForwarding()
{
    DialogWriteMsg *dwMsg = new DialogWriteMsg;
    dwMsg->initForwarding(getSelectedMesssage().content);
    dwMsg->show();
}

//收件箱、垃圾箱共有
//回复
void VirtualSMS::slotActionReply()
{    
    DialogWriteMsg *dwMsg = new DialogWriteMsg;
    dwMsg->initReply(getSelectedMesssage().phonenum);
    dwMsg->show();        
}

//移至收件箱
void VirtualSMS::slotActionMoveToInbox()
{    
    g_msgbox->changeBox(getSelectedMesssage().id, Message::Box_Inbox);
}

//移至发件箱
void VirtualSMS::slotActionMoveToOutbox()
{    
    g_msgbox->changeBox(getSelectedMesssage().id, Message::Box_Outbox);
}

//移至草稿箱
void VirtualSMS::slotActionMoveToDraftbox()
{
    g_msgbox->changeBox(getSelectedMesssage().id, Message::Box_Draftbox);
}

////移至垃圾箱
//void VirtualSMS::slotActionMoveToDustbin()
//{
//    Message msg = getSelectedMesssage();
//    g_msgbox->changeBox(msg.id, Message::Box_Dustbin);
//    g_MessageIdentification->MessageIdentificationProcessAndSave(msg.content);
//}

//删除消息
void VirtualSMS::slotActionRemoveMsg()
{
    if (QMessageBox::No == QMessageBox::information(
                this, tr("msg"), tr("Are you sure to delete?")
                , QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        return;
    }
    Message msg = getSelectedMesssage();
    if (msg.box == Message::Box_Dustbin) {
        g_msgbox->removeMessage(msg.id);
    } else {
        g_msgbox->changeBox(msg.id, Message::Box_Dustbin);
        g_MessageIdentification->MessageIdentificationProcessAndSave(msg.content);
        g_PhoneNumberIdentification->NumberIdentificationProcessAndSave(msg.phonenum);
        g_NumberSegmentIdentification->NSIProcessAndSave(msg.phonenum.toULongLong());
    }
}

//陌生号码存入电话本
void VirtualSMS::slotActionAddIntoPhoneBook()
{
    Contact contact;
    contact.phonenum = getSelectedMesssage().phonenum;
    viewContactDetail(contact);
}

//查看更多信息，进入信息管理
void VirtualSMS::slotActionViewMoreMsg()
{
    viewMsgManage();
}

//处理电话本按钮点击
void VirtualSMS::slotPhonebookBtnClicked()
{
    QToolButton *btn = static_cast<QToolButton*>(sender());
    btn->setProperty(cons_expand_property, !btn->property(cons_expand_property).toBool());
    updatePhoneBookList();
}

//处理消息按钮点击
void VirtualSMS::slotMsgBtnClicked()
{
    QToolButton *btn = static_cast<QToolButton*>(sender());
    int index = m_msgBtns.indexOf(btn);
    m_msgGroupBoxs[index]->show();
    m_msgListWidgets[index]->setFocus();
    m_msgListWidgets[index]->setCurrentItem(m_msgListWidgets[index]->item(0));
    for (int i=0; i<4; i++) {
        if (i != index) {
            m_msgGroupBoxs[i]->hide();
            m_msgListWidgets[i]->setCurrentItem(0);
        }
    }
}

//打开聊天界面
void VirtualSMS::openChatRoom(const Contact &contact)
{
    if (contact.type == Contact::Type_Myself) {
        QMessageBox::information(this, tr("msg"), tr("Can't chat with myself"));
        return;
    }
    DialogChatRoom *dcRoom = qobject_cast<DialogChatRoom*>(m_idToChatRoom.value(contact.id));
    if (dcRoom != NULL) {
        //激活窗口，显示至最前
        dcRoom->activateWindow();
        dcRoom->showNormal();     
    } else {
        dcRoom = new DialogChatRoom(contact);
        connect(dcRoom, SIGNAL(destroyed(QObject*)), this, SLOT(slotChatRoomClosed(QObject*)));
        dcRoom->showNormal();
        m_idToChatRoom.insert(contact.id, dcRoom);  //映射为一对一，插入重复键会直接替换旧值    
    }
}

//处理聊天界面关闭
void VirtualSMS::slotChatRoomClosed(QObject* obj)
{
    int id = m_idToChatRoom.key(obj);
    m_idToChatRoom.insert(id, NULL);
}

/************************************************
更新信息显示，信息变更事件说明如下
ChangeEvent_Add //添加信息
ChangeEvent_Remove //删除信息
ChangeEvent_ChangeBox //修改信息类型
ChangeEvent_ChangeState //修改信息状态
************************************************/
void VirtualSMS::slotUpdateMsg(int id, int changeEvent, int srcBox)
{
    //srcBox本来是想告诉界面该删哪个listWidget，不过由于后台线程的并发，
    //等到界面来删的时候，指定listWidget里可能并没有该信息，而是在另外一
    //个listWidget里。这是因为添加消息的时候是根据id添加的，当界面来添加
    //这个id的信息时，这id的属性和内容可能都已经变了。
    switch (changeEvent) {
    case MsgBox::ChangeEvent_Add:
        addMsg(id);
        break;
    case MsgBox::ChangeEvent_Remove:
        removeMsg(id);
        break;
    case MsgBox::ChangeEvent_ChangeBox:
        changeMsgBox(id);
        break;
    case MsgBox::ChangeEvent_ChangeState:
        changeMsgState(id);
        break;
    default:
        break;
    }
}

//添加信息
void VirtualSMS::addMsg(int id)
{
    Message msg = g_msgbox->getMessageOfId(id);
    addMsg(msg);
}

//添加信息
void VirtualSMS::addMsg(Message msg)
{
    if (msg.isEmpty()) {
        qDebug() << "msg is empty" << msg.id << msg.content << __FILE__ << __LINE__;
        return;
    }
    QListWidgetItem *lwItem = new QListWidgetItem;
    lwItem->setData(Qt::UserRole, QVariant::fromValue<Message>(msg));
    QListWidget *listWidget = getMsgBox(msg.box);
    if (listWidget == NULL) {
        return;
    }
    int count = listWidget->count();
    int i=0;
    while (i < count) {
        Message msgT = listWidget->item(i)->data(Qt::UserRole).value<Message>();
        if ((msgT.datetime<msg.datetime)
                || (msgT.datetime==msg.datetime && msgT.id<msg.id)) {
            break;
        }
        i++;
    }
    listWidget->insertItem(i, lwItem);
    //如果超过指定数目，则删除多余的项
    if (count >= cons_msg_list_item_num) {                
        QListWidgetItem *lwItemT = listWidget->item(count);
        listWidget->removeItemWidget(lwItemT);
        delete lwItemT;
    }
    
    updateMsgBoxNum();
    if (msg.state == Message::State_SendFail) {
        if (msg.msgtype == Message::MsgType_InMsg) {
            qDebug() << "this is a bug" << __FILE__ << __LINE__;
        }
        m_phoneNumToSendFailMsgs.insertMulti(msg.phonenum, msg.id);
    }
}

//删除信息
void VirtualSMS::removeMsg(int id)
{
    bool flag = false;
    foreach (QListWidget *listWidget, m_msgListWidgets) {
        for (int i=listWidget->count()-1; i>=0; i--) {
            QListWidgetItem *lwItem = listWidget->item(i);
            Message msg = lwItem->data(Qt::UserRole).value<Message>();
            if (id == msg.id) {
                listWidget->removeItemWidget(lwItem);
                delete lwItem;
                flag = true;
                if (msg.state == Message::State_SendFail) {
                    if (msg.msgtype == Message::MsgType_InMsg) {
                        qDebug() << "this is a bug" << __FILE__ << __LINE__;
                    }
                    m_phoneNumToSendFailMsgs.remove(msg.phonenum, msg.id);
                }
                QList<Message> msgs = g_msgbox->getLastMessages(cons_msg_list_item_num, (Message::Box)msg.box);
                if (msgs.count() == cons_msg_list_item_num) {
                    addMsg(msgs.first());
                }
                break;
            }
        }
        if (flag) {
            break;
        }
    }
    updateMsgBoxNum();

}

//修改信箱
void VirtualSMS::changeMsgBox(int id)
{
    removeMsg(id);
    addMsg(id);
}

//修改信息状态
void VirtualSMS::changeMsgState(int id)
{
    Message msg = g_msgbox->getMessageOfId(id);
    if (msg.isEmpty()) {
        qDebug() << "msg is empty" << msg.id << msg.content << __FILE__ << __LINE__;
        return;
    }
    QListWidget *listWidget = getMsgBox(msg.box);
    //接收到的信息不需要变更列表
    if (msg.msgtype == Message::MsgType_InMsg) {
        for (int i=listWidget->count()-1; i>=0; i--) {
            QListWidgetItem *lwItem = listWidget->item(i);
            Message msgT = lwItem->data(Qt::UserRole).value<Message>();
            if (msgT.id == msg.id) {
                lwItem->setData(Qt::UserRole, QVariant::fromValue<Message>(msg));
                break;
            }
        }
        updateMsgBoxNum();
    } else { //发送的信息需要变更列表
        removeMsg(msg.id);
        addMsg(msg.id);
        if (msg.state == Message::State_Sent) {
            if (msg.msgtype == Message::MsgType_InMsg) {
                qDebug() << "this is a bug" << __FILE__ << __LINE__;
            }
            m_phoneNumToSendFailMsgs.remove(msg.phonenum, msg.id);
        }                       
    }
}

//获取信箱列表控件
QListWidget* VirtualSMS::getMsgBox(int box) 
{
    QListWidget *listWidget(NULL);
    switch (box) {
    case Message::Box_Inbox:
        listWidget = ui->listWidgetInbox;
        break;
    case Message::Box_Outbox:
        listWidget = ui->listWidgetOutbox;
        break;
    case Message::Box_Draftbox:
        listWidget = ui->listWidgetDraftbox;
        break;
    case Message::Box_Dustbin:
        listWidget = ui->listWidgetDustbin;
        break;
    default:
        listWidget = NULL;
        qDebug() << "this is a bug" << __FILE__ << __LINE__;
        break;        
    }
    return listWidget;
}

//更新接收信息已读、未读数量
void VirtualSMS::updateMsgBoxNum()
{
    if (m_isInit) {
        ui->btnInbox->setText(tr(m_strInbox)
                              .arg(g_msgbox->getUnreadInboxNum())
                              .arg(g_msgbox->getBoxNum(Message::Box_Inbox)));
        ui->btnOutbox->setText(tr(m_strOutbox).arg(g_msgbox->getBoxNum(Message::Box_Outbox)));
        ui->btnDraftBox->setText(tr(m_strDraftBox)
                                 .arg(g_msgbox->getBoxNum(Message::Box_Draftbox)));
        ui->btnDustbin->setText(tr(m_strDustbin)
                                .arg(g_msgbox->getUnreadDustbinNum())
                                .arg(g_msgbox->getBoxNum(Message::Box_Dustbin)));
    }
}

//处理语言变更
void VirtualSMS::slotChangeLanguage()
{
    if (g_setting->getLanguage() == Language_Chinese) {
        m_tr.load(":/other/VirtualSMS_zh.qm");
        m_trInner.load(":/other/qt_zh_CN.qm");
    } else {
        m_tr.load("");
        m_trInner.load("");
    }
    ui->retranslateUi(this);
    if (m_isInit) {
        retranslateUi();
    }
}

//处理运行模式变更
void VirtualSMS::slotChangeRunMode()
{
    if (g_setting->getRunMode() == RunMode_Chat) {
        if (m_newInMsgPhoneNumToIds.isEmpty()) {
            m_trayIcon->setIcon(QIcon(":/pic/Tray.png"));
        } else {
            m_timerRemind.start();
        }
    } else {
        m_timerRemind.stop();
        if (m_newInMsgPhoneNumToIds.isEmpty()) {
            m_trayIcon->setIcon(QIcon(":/pic/Tray.png"));
        } else {
            m_trayIcon->setIcon(QIcon(":/pic/Tray-NewMsg.png"));
        }
    }
}

//查看信息详情
void VirtualSMS::viewMsgDetail(const Message &msg)
{
    if (m_dMsgDetail == NULL) {
        m_dMsgDetail = new DialogMsgDetail;
    }
    m_dMsgDetail->loadMsg(msg);
    m_dMsgDetail->showNormal(); 
    m_dMsgDetail->activateWindow();
}

//进入信息管理
void VirtualSMS::viewMsgManage()
{
    if (m_dMsgManage == NULL) {
        m_dMsgManage = new DialogMsgManage;
        connect(m_dMsgManage, SIGNAL(destroyed()), this, SLOT(slotMsgManageDestroyed()));
    }
    m_dMsgManage->showNormal();
    m_dMsgManage->activateWindow();    
}

void VirtualSMS::slotMsgManageDestroyed()
{
    m_dMsgManage = NULL;
}

//号码冲突
void VirtualSMS::slotPhoneNumConflict(const QString &name, const QHostAddress &ipAddr)
{
    QString str = QString("Name:%1, Ip:%2")
            .arg(name).arg(ipAddr.toString());
    m_trayIcon->showMessage(tr("Phonenumber conflict"), str);
    m_timerCheck.start();
}

//联系人登陆，是否有离线信息(发送失败的信息)需要发送
void VirtualSMS::slotContactLogin(const QString &phoneNum)
{
    qDebug() << "login" << m_phoneNumToSendFailMsgs << __FILE__ << __LINE__;
    if (m_phoneNumToSendFailMsgs.isEmpty()) {
        qDebug() << "empty failed msg" << __FILE__ << __LINE__;
        return;
    }
    QList<int> failedMsgs = m_phoneNumToSendFailMsgs.values(phoneNum);
    if (!failedMsgs.isEmpty()) {
        MessageSend send;
        foreach (int id, failedMsgs) {
            Message msg = g_msgbox->getMessageOfId(id);
            if (msg.isEmpty()) {
                qDebug() << "msg is empty" << msg.id << msg.content << __FILE__ << __LINE__;
                continue;
            }
            qDebug() << "send msg" << __FILE__ << __LINE__;
            if (send.sendMessage(msg.content, msg.phonenum)) {
                qDebug() << "send msg success" << __FILE__ << __LINE__;
                g_msgbox->changeState(msg.id, Message::State_Sent);
                g_msgbox->changeBox(msg.id, Message::Box_Outbox);
                m_phoneNumToSendFailMsgs.remove(msg.phonenum, msg.id);
                qDebug() << "change msg state to sent" << __FILE__ << __LINE__;
            }
        }
    }
}

//单击电话本列表项目
void VirtualSMS::slotProcessPhoneBookClicked(QListWidgetItem *item)
{
    Contact contact = item->data(Qt::UserRole).value<Contact>();
    for (int i=0; i<4; i++) {
        QListWidget *curListWidget = getPhoneBookBox(contact.type);
        foreach (QListWidget *listWidget, m_phonebookListWidgets) {
            if (listWidget != curListWidget) {
                QList<QListWidgetItem*> lwItems = listWidget->selectedItems();
                foreach (QListWidgetItem *lwItem, lwItems) {
                    lwItem->setSelected(false);
                }
            }
        }
    }    
}

/************************************************
双击电话本		
对象                     效果
自己                     查看自己信息
黑名单、白名单、其他        聊天模式：进入聊天;短信模式：写短信
************************************************/
void VirtualSMS::slotProcessPhoneBookDoubledClicked(QModelIndex index)
{
    Contact contact = index.data(Qt::UserRole).value<Contact>();
    if (contact.type == Contact::Type_Myself) {
        viewContactDetail(contact);
    } else {
        if (g_setting->getRunMode() == RunMode_Chat) {
            openChatRoom(contact);
        } else {
            DialogWriteMsg *dwMsg = new DialogWriteMsg();
            dwMsg->initReply(contact.phonenum);
            dwMsg->show();        
        }
    }
}

//电话本右键，设置各菜单是否可见
void VirtualSMS::slotProcessPhoneBookContextMenu()
{
    QListWidget *curPhoneBookBox = getFocusedPhoneBookBox();
    if (curPhoneBookBox!=NULL && curPhoneBookBox->currentIndex().isValid()) {        
        Contact contact = curPhoneBookBox->currentIndex().data(Qt::UserRole).value<Contact>();        
        if (g_setting->getRunMode()==RunMode_Chat && contact.type!=Contact::Type_Myself) {        
            m_actGoToChat->setVisible(true);       
        } else {
            m_actGoToChat->setVisible(false);
        }
        
        switch (contact.type) {
        case Contact::Type_Myself:
            m_actSendMsg->setVisible(false);
            m_actMoveToBlackList->setVisible(false);
            m_actMoveToWhiteList->setVisible(false);
            m_actMoveToOther->setVisible(false);
            m_actMoveToStranger->setVisible(false);
            m_actDeleteContact->setVisible(false);
            break;
        case Contact::Type_BlackList:
            m_actSendMsg->setVisible(true);
            m_actMoveToBlackList->setVisible(false);
            m_actMoveToWhiteList->setVisible(true);
            m_actMoveToOther->setVisible(true);
            m_actMoveToStranger->setVisible(true);
            m_actDeleteContact->setVisible(true);
            break;
        case Contact::Type_WhiteList:
            m_actSendMsg->setVisible(true);
            m_actMoveToBlackList->setVisible(true);
            m_actMoveToWhiteList->setVisible(false);
            m_actMoveToOther->setVisible(true);
            m_actMoveToStranger->setVisible(true);
            m_actDeleteContact->setVisible(true);
            break;
        case Contact::Type_Other:
            m_actSendMsg->setVisible(true);
            m_actMoveToBlackList->setVisible(true);
            m_actMoveToWhiteList->setVisible(true);
            m_actMoveToOther->setVisible(false);
            m_actMoveToStranger->setVisible(true);
            m_actDeleteContact->setVisible(true);
            break;
        case Contact::Type_Stranger:
            m_actSendMsg->setVisible(true);
            m_actMoveToBlackList->setVisible(true);
            m_actMoveToWhiteList->setVisible(true);
            m_actMoveToOther->setVisible(true);
            m_actMoveToStranger->setVisible(false);
            m_actDeleteContact->setVisible(false);
            break;            
        default:
            break;
        }
        m_menuPhoneBook->exec(QCursor::pos());
    }
}

//电话本按钮右键
void VirtualSMS::slotProcessPhoneBookBtnContextMenu()
{
    //暂不显示
    m_menuPhoneBookBtn->exec(QCursor::pos());
}

//处理消息双击事件
//如果当前运行模式为聊天模式且来信人为联系人，则进入聊天，否则查看详情
void VirtualSMS::slotProcessMsgDoubledClicked(QModelIndex index)
{
    Message msg = index.data(Qt::UserRole).value<Message>();
    if (msg.state == Message::State_Unsend) {
        DialogWriteMsg *dWriteMsg = new DialogWriteMsg;
        dWriteMsg->initSendUnsendMsg(msg);
        dWriteMsg->showNormal();
        g_msgbox->removeMessage(msg.id);        
        return;
    } else if (msg.state == Message::State_Unread) {
        g_msgbox->setRead(msg.id, true);
    }
    if (g_setting->getRunMode() == RunMode_Chat) {
        Contact contact = g_phoneBook->getContactOfPhoneNum(msg.phonenum);
        if (contact.isValid()) {
            openChatRoom(contact);
        } else {
            viewMsgDetail(msg);
        }
    } else {
        viewMsgDetail(msg);
    }
}

//信箱右键，设置各菜单是否可见
void VirtualSMS::slotProcessMsgContextMenu()
{
    QListWidget *curMsgBox = getFocusedMsgBox();
    if (curMsgBox->currentIndex().isValid()) {
        Message msg = curMsgBox->currentIndex().data(Qt::UserRole).value<Message>();
        
        m_actForwarding->setVisible(true);
        m_actReply->setVisible(true);
        
        if (msg.state == Message::State_Unsend) {
            m_actReply->setVisible(false);
            m_actEditUnsendMsg->setVisible(true);
        } else {
            m_actEditUnsendMsg->setVisible(false);
        }

        bool isSpeaking = g_speech->getStatus()==QtSpeech::State_Speaking;
        m_actRead->setVisible(!isSpeaking);
        m_actStopRead->setVisible(isSpeaking);
        
        m_actMoveToInbox->setVisible(false);
        m_actMoveToOutbox->setVisible(false);
        m_actMoveToDraftbox->setVisible(false);
//        m_actMoveToDustbin->setVisible(false);
        
        switch (msg.msgtype) {
        case Message::MsgType_InMsg:
            if (msg.box == Message::Box_Inbox) {
//                m_actMoveToDustbin->setVisible(true);            
            } else {
                m_actMoveToInbox->setVisible(true);                        
            }
            break;
        case Message::MsgType_OutMsg:
            m_actReply->setVisible(false);
            if (msg.box == Message::Box_Dustbin) {
                m_actMoveToOutbox->setVisible(true);
                m_actMoveToDraftbox->setVisible(true);
            } else {
//                m_actMoveToDustbin->setVisible(true);
            }
            break;
        default:
            break;
        }
        
        bool isValid = g_phoneBook->getContactOfPhoneNum(msg.phonenum).isValid();
        m_actAddIntoPhoneBook->setVisible(!isValid);
        if (g_setting->getRunMode() == RunMode_Chat) {
            m_actGotoChatFromMsg->setVisible(isValid);  //陌生号码则不可见
        } else {
            m_actGotoChatFromMsg->setVisible(false);
        }
        m_menuMsg->exec(QCursor::pos());
    }
}

//自动搜索
void VirtualSMS::slotAutoSuggest()
{
//    QStringList searchList; 
    QList<Contact> searchList;
    QString strSearch = ui->lineEditSearch->text();
    if (strSearch.isEmpty()) {
        return;
    }
//    for (int i=ui->listWidgetPhoneBook->count()-1; i>=0; i--) {
//        //暂时使用显示文本来搜
//        QString str = ui->listWidgetPhoneBook->item(i)->text();
//        if (str.contains(strSearch)) {
//            searchList.append(str);
//        }
//    }
    
    QList<Contact> contacts = g_phoneBook->getAllContacts();
    foreach (Contact contact, contacts) {
        if (contact.name.contains(strSearch) || contact.phonenum.contains(strSearch)) {
            searchList.append(contact);
        }
    }

//    searchList.sort();
    showCompletion(searchList);    
}

//搜索完成处理
void VirtualSMS::slotDoneCompletion()
{
    m_popup->hide();
    ui->lineEditSearch->setFocus();
    QListWidgetItem *item = m_popup->currentItem();
    if (item) {
        ui->lineEditSearch->setText(item->text());        
        Contact contact = item->data(Qt::UserRole).value<Contact>();
        getPhoneBookBtn(contact.type)->setProperty(cons_expand_property, true);
        QListWidget *listWidget = getPhoneBookBox(contact.type);        
        for (int i=listWidget->count()-1; i>=0; i--) {
            QListWidgetItem *lwItem = listWidget->item(i);
            if (lwItem->data(Qt::UserRole).value<Contact>() == contact) {
                listWidget->setCurrentItem(lwItem);
                slotProcessPhoneBookClicked(lwItem);
                break;
            }
        }        
    }
    updatePhoneBookList();
}

//显示搜索结果
void VirtualSMS::showCompletion(const QList<Contact> &choices)
{
    if (choices.isEmpty()) {
        return;
    }

    const QPalette &pal = ui->lineEditSearch->palette();
    QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);

    m_popup->setUpdatesEnabled(false);
    m_popup->clear();
    for (int i = 0; i < choices.count(); ++i) {        
        QListWidgetItem *item = new QListWidgetItem(m_popup);
        Contact contact = choices.at(i);
        QString strContact = QString("%1<%2>").arg(contact.name).arg(contact.getCurrentPhoneNum());
        item->setText(strContact);
        item->setData(Qt::UserRole, QVariant::fromValue<Contact>(contact));
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setTextColor(color);
        item->setSizeHint(QSize(0, cons_other_item_height));
    }
    m_popup->setCurrentItem(m_popup->item(0));
    m_popup->adjustSize();
    m_popup->setUpdatesEnabled(true);

    int h = m_popup->sizeHintForRow(0) * qMin(7, choices.count()) + 3;  
    m_popup->resize(ui->lineEditSearch->width(), h);

    m_popup->move(ui->lineEditSearch->mapToGlobal(QPoint(0, ui->lineEditSearch->height())));
    m_popup->setFocus();
    m_popup->show();
}

void VirtualSMS::slotFinishRead()
{
    qDebug() << "finish read" << __FILE__ << __LINE__;
    m_actRead->setVisible(true);
    m_actStopRead->setVisible(false);
}

void VirtualSMS::slotCheck()
{
    if (!g_udpbroadcast->checkConflict()) {
        m_timerCheck.stop();
    }
}

//添加联系人
void VirtualSMS::on_btnAddContact_clicked()
{
    viewContactDetail();
}

//写信息
void VirtualSMS::on_btnWriteMsg_clicked()
{
    DialogWriteMsg *dwMsg = new DialogWriteMsg();
    dwMsg->init();
    dwMsg->show();
}

//设置
void VirtualSMS::on_btnSet_clicked()
{
    DialogSetting *dSetting = new DialogSetting;
    dSetting->exec();
}

//信息管理
void VirtualSMS::on_btnMsgManage_clicked()
{
    viewMsgManage();
}

