/************************************************
类名: ItemDelegate
功能: 用于显示收件箱、发件箱、垃圾箱信息
说明: 收件箱、发件箱、垃圾箱列表控件代理
************************************************/

#include "itemdelegate.h"
#include "config.h"
#include "phonebook.h"

ItemDelegate::ItemDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

void ItemDelegate::paint(QPainter *painter, 
                         const QStyleOptionViewItem &option, 
                         const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else if (option.state & QStyle::State_MouseOver) {
        QBrush brush(QColor(0, 0, 255, 25));
        painter->fillRect(option.rect, brush);
    }
    Message msg = index.data(Qt::UserRole).value<Message>();
    QStyleOptionViewItem myOption = option;
    QFontMetrics fm(painter->font());
    int width = myOption.rect.width();
    int height = myOption.rect.height();
    int textMargin = fm.height();
    
    Contact contact = g_phoneBook->getContactOfPhoneNum(msg.phonenum);
    QString str;
    if (contact.isValid()) {
        str = contact.name;
    } else {
        str = msg.phonenum;
    }
    myOption.displayAlignment = Qt::AlignCenter;
    QPixmap pixmap;
    QString strStatePix;
    switch (msg.state) {
    case Message::State_Read:
        strStatePix = ":/pic/Read.png";
        break;
    case Message::State_Unread:
        strStatePix = ":/pic/Unread.png";
        break;
    case Message::State_Unsend:
        strStatePix = ":/pic/Unsend.png";
        break;
    case Message::State_Sent:
        strStatePix = ":/pic/Sent.png";
        break;
    case Message::State_SendFail:
        strStatePix = ":/pic/SendFail.png";
        break;
    default:
        qDebug() << "this is a bug" << __FILE__ << __LINE__;
        break;
    }    
    pixmap.load(strStatePix);

    QSize sizePixmap(height / 2 * 0.7, height / 2 * 0.7);
    int pixMargin = (height/2-sizePixmap.height())/2;
    painter->drawPixmap(QRect(myOption.rect.topLeft() + QPoint(pixMargin, pixMargin), sizePixmap), pixmap);
    
    myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    QSize sizeName(fm.width(msg.phonenum) + textMargin, height / 2);
    drawDisplay(painter, 
                myOption, 
                QRect(myOption.rect.topLeft() + QPoint(height/2, 0), sizeName),
                str);
    
    QSize sizeDateTime(width - sizePixmap.width() - sizeName.width(), height / 2);
    myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    drawDisplay(painter, 
                myOption, 
                QRect(myOption.rect.topRight() - QPoint(sizeDateTime.width(), 0), sizeDateTime),
                msg.datetime.toString("yyyy-MM-dd hh:mm:ss"));    
    
    myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    drawDisplay(painter, 
                myOption, 
                QRect(option.rect.topLeft() + QPoint(height / 2, height / 2), 
                      QSize(option.rect.width() - height / 2, height / 2)), 
                msg.content);     
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), cons_msg_item_height);
}
