#ifndef DIALOGDELETEMSG_H
#define DIALOGDELETEMSG_H

#include <QDialog>
#include <QDateTime>
#include <QIntValidator>

namespace Ui {
    class DialogDeleteMsg;
}

class DialogDeleteMsg : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDeleteMsg(QWidget *parent = 0);
    ~DialogDeleteMsg();

signals:
    void signDeleteMsgsBefore(const QDateTime &);
    
private slots:
    void on_btnDelete_clicked();
    
    void on_btnCancel_clicked();
    
    void on_radioButtonDeleteAll_clicked();
    
    void on_radioButtonDeleteBefore_clicked();
    
private:
    Ui::DialogDeleteMsg *ui;
    QIntValidator m_valid;
};

#endif // DIALOGDELETEMSG_H
