/************************************************
类名: MsgManageItemDelegate
功能: 用于显示信息管理里的信息
说明: 无
************************************************/

#include "msgmanageitemdelegate.h"
#include "config.h"
#include "phonebook.h"

const int cons_text_margin = 6;

enum DataIndex {
    DataIndex_Type,
    DataIndex_Value
};
enum DataType {
    DataType_MessageBox,
    DataType_PhoneBook
};

MsgManageItemDelegate::MsgManageItemDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

void MsgManageItemDelegate::paint(QPainter *painter, 
                         const QStyleOptionViewItem &option, 
                         const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) {
        QBrush brush(QColor(0, 0, 255, 50));
        painter->fillRect(option.rect, brush);
    } else if (option.state & QStyle::State_MouseOver) {
        QBrush brush(QColor(0, 0, 255, 25));
        painter->fillRect(option.rect, brush);
    }
    DataType dataType = (DataType)index.data(Qt::UserRole + DataIndex_Type).toInt();
    Message msg = index.data(Qt::UserRole + DataIndex_Value).value<Message>();
    int fontHeight = option.fontMetrics.height();
    QPoint pos = option.rect.topLeft();
    QString strDateTime;
    Contact contact;
    Contact myself;
    QString strSender;
    QString strReceiver;
    QString strContactInfo;
    QString strPersonalInfo;
    if (msg.datetime.date().daysTo(QDate::currentDate()) == 0) {
        strDateTime = msg.datetime.toString("hh:mm:ss");
    } else {
        strDateTime = msg.datetime.toString("yyyy-MM-dd hh:mm:ss");
    }
    contact = g_phoneBook->getContactOfPhoneNum(msg.phonenum);
    if (contact.isValid()) {
        strContactInfo = QString("%1<%2>").arg(contact.name).arg(contact.getCurrentPhoneNum());
    } else {
        strContactInfo = msg.phonenum;
    }
    myself = g_phoneBook->getMyself();
    strPersonalInfo = QString("%1<%2>").arg(myself.name).arg(myself.getCurrentPhoneNum());
    
    painter->save();
    pos += QPoint(0, fontHeight/3);    
    
    QPixmap pixmap(QString(":/pic/%1.png")
                   .arg(Message::msgStateToString((Message::State)msg.state)));   
    float ratio = pixmap.width() * 1.0 / pixmap.height();
    QSize sizePixmap(fontHeight * ratio, fontHeight);
    int textWidth = option.rect.width() - sizePixmap.width();
    if (dataType == DataType_MessageBox) {
        painter->drawPixmap(QRect(pos, sizePixmap), pixmap);          
        
        pos += QPoint(sizePixmap.width() + fontHeight/3, 0);
        painter->setPen(Qt::blue);
        switch (msg.box) {
        case Message::Box_Inbox:
        case Message::Box_Outbox:
        case Message::Box_Draftbox:
            painter->drawText(QRect(pos, QSize(textWidth, fontHeight)),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              QString("%1 %2").arg(strContactInfo).arg(strDateTime));
            break;
        case Message::Box_Dustbin:        
            if (msg.msgtype == Message::MsgType_InMsg) {
                strSender = strContactInfo;
                strReceiver = strPersonalInfo;
            } else {
                strSender = strPersonalInfo;
                strReceiver = strContactInfo;
            }        
            //发件人
            painter->drawText(QRect(pos, QSize(textWidth, fontHeight)), 
                              Qt::AlignLeft | Qt::AlignVCenter,
                              tr("Sender:    %1").arg(strSender));
            //时间
            pos += QPoint(0, fontHeight);    
            painter->drawText(QRect(pos, QSize(textWidth, fontHeight)), 
                              Qt::AlignLeft | Qt::AlignVCenter,
                              tr("DateTime:  %1").arg(strDateTime));
            //收件人
            pos += QPoint(0, fontHeight);    
            painter->drawText(QRect(pos, QSize(textWidth, fontHeight)), 
                              Qt::AlignLeft | Qt::AlignVCenter,
                              tr("Receiver:  %1").arg(strReceiver));
            break;
        default:
            break;
        }        
    } else if (dataType == DataType_PhoneBook) {
        switch (msg.msgtype) {
        case Message::MsgType_InMsg:
            painter->setPen(Qt::blue);
            painter->drawText(QRect(pos, QSize(textWidth, fontHeight)),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              QString("%1 %2").arg(strContactInfo).arg(strDateTime));            
            break;
        case Message::MsgType_OutMsg:
            painter->setPen(Qt::darkGreen);
            painter->drawText(QRect(pos, QSize(textWidth, fontHeight)),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              QString("%1 %2").arg(strPersonalInfo).arg(strDateTime));            
            break;
        default:
            break;
        }
    } else {
        qDebug() << "this is a bug" << __FILE__ << __LINE__;
    }
    painter->restore();     
    
    //内容
    painter->save();
    pos += QPoint(0, fontHeight + fontHeight/3);
    painter->setPen(QPen(Qt::black));
    QRect rectContent = QRect(pos, QSize(textWidth, option.rect.bottomLeft().y() - pos.y()));
    painter->drawText(rectContent, 
                      msg.content + "\n"); 
    painter->restore();
    
}

QSize MsgManageItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //计算高度
    DataType dataType = (DataType)index.data(Qt::UserRole + DataIndex_Type).toInt();
    Message msg = index.data(Qt::UserRole + DataIndex_Value).value<Message>();
    QString head;
    if (dataType==DataType_MessageBox && msg.box == Message::Box_Dustbin) {
        head = "idle line\nidle line\nidle line\n";
    } else {
        head = "idle line\n";
    }
    int width = option.rect.width();
    int height = 0;
    if (width > 0) {
        QTextDocument textDoc(head + msg.content);
        textDoc.setDefaultFont(option.font);
        textDoc.setTextWidth(width - cons_text_margin);
        height = textDoc.size().height();
    } else {
        height = option.fontMetrics.height();
    }
    return QSize(option.rect.width(), height);
}
