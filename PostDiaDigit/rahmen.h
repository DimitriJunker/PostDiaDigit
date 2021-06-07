#ifndef RAHMEN_H
#define RAHMEN_H

#include <QDialog>
#include <qdir.h>
#include <qtextstream.h>
#include <QKeyEvent>

namespace Ui {
class Rahmen;
}

class Rahmen : public QDialog
{
    Q_OBJECT

public:
    explicit Rahmen(QWidget *parent = nullptr);
    ~Rahmen();
protected:
    void keyPressEvent(QKeyEvent *e);
private slots:
    void on_pb_autom_clicked();

    void on_sel_1st_clicked();

    void on_pb_go_clicked();

    void on_pb_quit_clicked();

    void on_pb_relaod_clicked();

    void on_pb_anzeigen_clicked();

    void on_pb_setK_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pb_hoch_clicked();

    void on_pb_runter_clicked();

    void on_pb_links_clicked();

    void on_pb_rechts_clicked();
    void changeEvent(QEvent *e);

private:
    Ui::Rahmen *ui;
    int m_TPkteX,m_TPkteY,m_TPkteW,m_TPkteH;  //Position Größe des Rechtecks das durch die Punkte aufgespannt wird, im transformierten Bild
    int m_TDiaX,m_TDiaY,m_TDiaW,m_TDiaH;  //Größe eines Dias im transformierten Bild und Position des ersten relativ zum roten Punkt oben links
    int m_anzX,m_anzY,m_distY;
    int m_TDeckelX,m_TDeckelY,m_TDeckelW,m_TDeckelH;

    QStringList m_fNamen;
    QImage m_ganz,m_40rahmen,m_deckel;
    QString m_prefix,m_suffix;
    int m_lenS,m_start,m_end,m_schiene;
    QString m_lstD;
    QDir m_qDir,m_zDir,m_dDir,m_2Dir,m_pDir;
    QColor m_rot,m_gruen;
    QPolygon m_quad;
    QRect m_posD;


    void runAuto(QString path);
    bool cutRahmen();
    void cutRahmen_m();
    void writeRahmen();
    QPoint findePkt(QImage &ganz,int pkt);
    void bildDrehen(QPoint &mpktGr);
    void bildDrehen(int winkel);
    void akt_pultF();
    QString mkFnameD(int nr);
    QString mkFnameP(int nr);
    QString mkFnameR(int nr);
    void aktSchiene();
    void aktfNamen();
    int col_dif(QColor col1,QColor col2);
    int col_dif(QColor col,int hue);
    int durchm_XY(QImage &img, QPoint &mpkt, QColor colS, bool inY);
    void showQuad();

    double adjColCircle(QImage &img, QPoint &mpkt, QColor colS, int sollD);


};

#endif // RAHMEN_H
