#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QItemDelegate>

class ItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ItemDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    
signals:

public slots:

};

#endif // ITEMDELEGATE_H
