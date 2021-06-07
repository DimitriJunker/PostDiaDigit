#ifndef DIADUBL_H
#define DIADUBL_H
#include <qdir.h>

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class DiaDubl;
}

class DiaDubl : public QDialog
{
    Q_OBJECT

public:
    explicit DiaDubl(QString ext, QWidget *parent = nullptr);
    QString m_ext;
    ~DiaDubl();
    void chkDir(QDir dir);
    double unterschied(double a, double b);
    void filter();

    void changeEvent(QEvent *e);

    QStringList m_dubletten;

protected:
    void keyPressEvent(QKeyEvent *e);
private slots:
    void on_pb_Dir_clicked();

    void on_pb_Nxt_clicked();

    void on_pb_Abbruch_clicked();

private:
    Ui::DiaDubl *ui;
};

#endif // DIADUBL_H
