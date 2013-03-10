#ifndef MSGMANAGEITEMDELEGATE_H
#define MSGMANAGEITEMDELEGATE_H

#include <QItemDelegate>

class MsgManageItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit MsgManageItemDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    
signals:

public slots:

};

#endif // MSGMANAGEITEMDELEGATE_H
