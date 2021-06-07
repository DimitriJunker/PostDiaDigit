#ifndef DIASCUT_H
#define DIASCUT_H

#include <QDialog>
#include <qpixmap.h>
#include <qdir.h>
#include <qprogressdialog.h>
#include <diasopt.h>
#include <QKeyEvent>

typedef struct
{
    double  s_winkel;
    QRect   s_rectH;
    QRect   s_rectB;
    QDir    s_diG;
    QDir    s_diH;
    QProgressDialog *s_progD;
}CUTDATA;

typedef struct
{
    QString s_fPath;
    CUTDATA *s_data;
}CUTPICT;

namespace Ui {
class DiasCut;
}


class DiasCut : public QDialog
{
    Q_OBJECT

public:
    explicit DiasCut(QString ext1,QWidget *parent = nullptr);
    ~DiasCut();
    QTranslator m_language;
    void changeEvent(QEvent *e);
    QString m_maske;

protected:
    void keyPressEvent(QKeyEvent *e);
private slots:
    void on_pb_selPict_clicked();


    void on_pb_Load_cut_clicked();

    void on_pushButton_clicked();

    void on_pb_Abbruch_clicked();

private:
    Ui::DiasCut *ui;
    QPixmap m_pict;
    QList<QPoint> m_cont;
    int m_dpmm; //Punkte entlang des Umpfangs pro mm
    int m_ppmm; //Pixel pro mm
    QSize m_size;
    QDir m_qdir;
    QProgressDialog *m_prog;
    DiasOpt m_opt;

    void runAuto(QString path);
    int suchEckeVH(int anf, int step);
    int suchEckeHV(int anf, int step);
    double regressionWinkel(bool isVertikal,int ecke1, int ecke2, int ecke3=-1,int ecke4=-1);
    int regressionH(double arc, int ecke1, int ecke2, int ecke3=-1,int ecke4=-1);
    int regressionV(double arc,int ecke1, int ecke2, int ecke3=-1,int ecke4=-1);
    void saveCut(QDir di, double arc,QRect bk, QRect hk);
    void schneiden(QString path);
    void schneiden(double arc,QRect bk, QRect hk);
    void savePkt();
    void setDir(QString dir);
    void setDir(QDir dir);

};

#endif // DIASCUT_H
