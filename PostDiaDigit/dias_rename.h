#ifndef DIAS_RENAME_H
#define DIAS_RENAME_H

#include <QDialog>
#include <qstring.h>
#include <qdir.h>
#include <QTranslator>
#include <QKeyEvent>

namespace Ui {
class dias_rename;
}

class dias_rename  : public QDialog
{
    Q_OBJECT

public:
    explicit dias_rename(QWidget *parent = nullptr);
    QTranslator m_language;
    void changeEvent(QEvent *e);
    void setExt(QString ext1, QString ext2);

    QString m_ext1,m_ext2;

    ~dias_rename();
protected:
    void keyPressEvent(QKeyEvent *e);
private slots:
    void on_pb_dir_clicked();

    void on_rb_1_clicked();

    void on_rb36_clicked();

    void on_rb_50_clicked();

    void on_rb_100_clicked();

    void on_rb_n_clicked();

    void on_rb_80_clicked();

    void on_sb_n_valueChanged(int arg1);

    void on_le_prefix_editingFinished();

    void on_sb_schiene_valueChanged(int arg1);

    void on_lineEdit_editingFinished();

    void on_sb_start_valueChanged(int arg1);

    void on_le_jpg_editingFinished();

    void on_sb_delStart_valueChanged(int arg1);

    void on_sb_delEnde_valueChanged(int arg1);

    void on_pB_go_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_rb_60_clicked();


//    void on_buttonBox_accepted();

    void on_pb_Abbruch_clicked();

private:
    Ui::dias_rename *ui;

    QStringList m_fNamen;
    int m_anzahl;

    void akt_fNamen();
    void akt_2do();
    QString mkFname(int i, bool nef=false);
    QString schiebe(QDir diQ,QDir diZ,QString fNameQ,QString fNameZ);
};

#endif // DIAS_RENAME_H
