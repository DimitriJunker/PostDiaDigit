#ifndef CPDDCOPY_H
#define CPDDCOPY_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class CPDDCopy;
}

class CPDDCopy : public QDialog
{
    Q_OBJECT

public:
    explicit CPDDCopy(QString &vers, QWidget *parent = nullptr);
//    CPDDCopy(QString &vers,QWidget *parent = 0);   // Standardkonstruktor

    ~CPDDCopy();
protected:
    void keyPressEvent(QKeyEvent *e);
public slots:
protected:
    void changeEvent(QEvent *e);


private slots:
    void on_pb_dyj_clicked();


    void on_pb_gpl_clicked();

private:
    Ui::CPDDCopy *ui;
};

#endif // CPDDCOPY_H
