#ifndef DIALOGADDIDENTIFICATION_H
#define DIALOGADDIDENTIFICATION_H

#include <QDialog>
#include <QtGui>

namespace Ui {
    class DialogAddIdentification;
}

class DialogAddIdentification : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddIdentification(QWidget *parent = 0);
    ~DialogAddIdentification();
    
    enum Type {
        Type_AddKeyword,
        Type_AddNumber,
        Type_AddNumberSegment
    };

    void init(const QList<QString> &keys, Type type);

signals:
    void signAdd(const QMap<QString, QString> &, int);
    
private slots:
    void on_btnOK_clicked();
    
    void on_btnCancel_clicked();
    
private:
    Ui::DialogAddIdentification *ui;
    QList<QLabel*> m_labelKeys;
    QList<QLineEdit*> m_lineEditKeys;
    QList<QString> m_keys;
    Type m_type;
    QRegExpValidator m_regValid;
};

#endif // DIALOGADDIDENTIFICATION_H
