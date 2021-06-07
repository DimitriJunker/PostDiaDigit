#include "rahmen.h"
#include "ui_rahmen.h"
#include <qfileinfo.h>
#include <qpainter.h>
#include <qdebug.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <math.h>
#include <QDesktopServices>

#define MINSAT 80


Rahmen::Rahmen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Rahmen)
{
    ui->setupUi(this);
    m_lstD="";

    m_rot=QColor(200,44,44);
    m_gruen=QColor(40,75,50);
    m_TPkteW=2700;
    m_TPkteH=2050;
    m_anzX=8;
    m_anzY=5;
    m_distY=15;
    m_TDeckelX=2760;    //Position und Größe des Deckels relativ zum roten Punkt oben links
    m_TDeckelY=-175;
    m_TDeckelW=800;
    m_TDeckelH=560;
    m_TDiaX=105;
    m_TDiaY=-10;
    m_TDiaW=312;
    m_TDiaH=413;
 //   QStringList pars=QApplication::instance()->arguments();
//    if(pars.size()>1)
  //      runAuto(pars[1]);
}

Rahmen::~Rahmen()
{
    delete ui;
}

void Rahmen::on_pb_autom_clicked()
{

#ifdef QT_DEBUG
    QString path= "I:\\dias_neu\\Rahmen";
#else
    QString path= QFileDialog::getExistingDirectory(this,tr("Quellverzeichnis"),"");
#endif
    if(!path.isEmpty())
    {
        ui->la_dir->setText(path);
        m_qDir.setPath(path);

        // Done dir
        m_dDir=m_qDir;
        m_dDir.mkdir("Done");
        m_dDir.cd("Done");
        // 2do dir
        m_2Dir=m_qDir;
        m_2Dir.mkdir("2do");
        m_2Dir.cd("2do");
        akt_pultF();
        on_sel_1st_clicked();
    }
}

enum PUNKT {ROT1,ROT2,ROT3,GRUEN};

bool Rahmen::cutRahmen()
{
    bool ok=true;
    QFileInfo fiQ(m_qDir,m_fNamen[0]),fiZ;
    m_ganz=QImage(fiQ.filePath());
// grünsten Punkt suchen
    QPoint mpktGr=findePkt(m_ganz,GRUEN),mpktRot1,mpktRot2,mpktRot3;
    if(mpktGr.x()==-1)
        ok=false;   // kein Punkt gefunden
    else            // falls nicht unten rechts drehen
    {
        bildDrehen(mpktGr);
        QFileInfo fiZ(m_qDir,"gedreht.jpg");
        m_ganz.save(fiZ.filePath());
        // die 3roten Punkte suchen
        mpktRot1=findePkt(m_ganz,ROT1);
        mpktRot2=findePkt(m_ganz,ROT2);
        mpktRot3=findePkt(m_ganz,ROT3);
        if(mpktRot1.x()==-1||mpktRot2.x()==-1||mpktRot3.x()==-1)
            ok=false;   // kein Punkt gefunden
        m_quad.clear();
        m_quad <<mpktRot1 <<mpktRot2 << mpktRot3 << mpktGr;
        showQuad();
    }
    if(ok)
        cutRahmen_m();
    else    //wohl kein Pultfoto
    {
        ui->la_deckel->setPixmap(QPixmap::fromImage(m_ganz.scaled(ui->la_deckel->size(),Qt::KeepAspectRatio)));
        bool isDeckel=false;
        QMessageBox::StandardButton reply;
        if(m_lstD.isEmpty())
            reply=QMessageBox::warning(nullptr, tr("kein Pultfoto"),tr("Dies scheint kein Pultfoto zu sein, kopiere es in den 2do Ordner"),QMessageBox::Ok|QMessageBox::Abort);
        else
            reply=(QMessageBox::warning(nullptr,tr("kein Pultfoto"),tr("Dies scheint kein Pultfoto zu sein. Soll es das letzte Deckelbild ersetzen?"),QMessageBox::Yes|QMessageBox::No|QMessageBox::Abort));
        if(reply!=QMessageBox::Abort)
        {
            isDeckel=(reply==QMessageBox::Yes);
            QFileInfo fiZ;
            if(isDeckel)
            {
                m_ganz.scaled(800,560,Qt::KeepAspectRatio).save(m_lstD);
                fiZ.setFile(m_dDir,fiQ.fileName());
            }
            else
                fiZ.setFile(m_2Dir,fiQ.fileName());
            QFile::rename(fiQ.filePath(),fiZ.filePath());
            m_fNamen.removeFirst();
            if(!m_fNamen.isEmpty())
                cutRahmen();
        }
    }
    return ok;
}
//Transformiert das Bild so, daß die 4 farbigen Punkte ein Rechteck mit der Größe m_TPkteW*m_TPkteH aufspannen
//und speichert dies ab, Außerdem wird die Deckelposition ermittelt
void Rahmen::cutRahmen_m()
{
   QPolygonF quadQ, quadZ;
   quadQ=m_quad;
   quadZ << QPointF(0,0) << QPointF(m_TPkteW,0) << QPointF(0,m_TPkteH) << QPointF(m_TPkteW,m_TPkteH);
   QTransform q2q;
   QTransform::quadToQuad(quadQ,quadZ,q2q);
   m_40rahmen=m_ganz.transformed(q2q);
   QFileInfo fi(m_qDir,"q2q.jpg");
   m_40rahmen.save(fi.filePath());

   m_TPkteX=int(QImage::trueMatrix(q2q,m_ganz.width(),m_ganz.height()).dx()-q2q.dx()+.5);
   m_TPkteY=int(QImage::trueMatrix(q2q,m_ganz.width(),m_ganz.height()).dy()-q2q.dy()+.5);

   m_posD=QRect(m_TPkteX+m_TDeckelX,m_TPkteY+m_TDeckelY,m_TDeckelW,m_TDeckelH);
   m_deckel=m_40rahmen.copy(m_posD);
   ui->la_deckel->setPixmap(QPixmap::fromImage(m_deckel.scaled(ui->la_deckel->size(),Qt::KeepAspectRatio)));
}

void Rahmen::writeRahmen()
{

    QString zfname=mkFnameD(m_start);
    QFileInfo fi(m_zDir,zfname);
    m_lstD=fi.filePath();
    m_deckel.save(m_lstD);
    int i=m_start;

   for(int y=0;y<m_anzY;y++)
   {
       for(int x=0;x<m_anzX;x++)
       {
           QString zfname=mkFnameR(i++);
           fi.setFile(m_zDir,zfname);
           QRect cut(m_TPkteX+(x*m_TDiaW)+m_TDiaX-m_TDiaW/20,m_TPkteY+(y*m_TDiaH)+m_TDiaH,(m_TDiaW*11)/10,m_TDiaH);
           m_40rahmen.copy(cut).save(fi.filePath());
           if(i>m_end)
               return;
       }
   }
}

void Rahmen::bildDrehen(QPoint &mpktGr)
{
    int winkel=0,x=mpktGr.x(),y=mpktGr.y();
     if(x>(m_ganz.width())/2)    //grüner Punkt in rechter Hälfte
     {
         if(y<(m_ganz.height()/2))   //oben rechts
         {
             winkel=90;
             mpktGr.setY(x);
             mpktGr.setX(m_ganz.height()-y);
         }                         // sonst unten rechts, also OK
     }
     else  //grüner Punkt in linker Hälfte
     {
         if(y<(m_ganz.height()/2))   //oben links
         {
             winkel =180;
             mpktGr.setX(m_ganz.width()-x);
             mpktGr.setY(m_ganz.height()-y);
         }
         else  //unten links
         {
             winkel=270;
             mpktGr.setX(y);
             mpktGr.setY(m_ganz.width()-x);
         }
     }
     if(winkel>0)
         bildDrehen(winkel);
}
void Rahmen::bildDrehen(int winkel)
{
    int tx=0,ty=0;
    switch (winkel)
    {
    case 90:
        ty=-m_ganz.height();
        break;
    case 180:   // statt um 180 Grad drehn einfach spiegeln um beide Achsen
    {
        QImage rot=m_ganz.mirrored(true,true);
        rot.swap(m_ganz);
        winkel=0;
        break;
    }
    case 270:
        tx=-m_ganz.width();
        break;
    default:
        winkel=0;
        break;
    }
    if(winkel)
    {
        QImage bildR(m_ganz.height(),m_ganz.width(),m_ganz.format());
        QPainter painter(&bildR);
        painter.rotate(winkel);
        painter.translate(tx,ty);
        painter.drawImage(0,0,m_ganz);
        bildR.swap(m_ganz);
    }

}

QPoint Rahmen::findePkt(QImage &ganz,int pkt)
{
    int step=ganz.width()/200,x,y,xMin=0,xMax=ganz.width(),yMin=0,yMax=ganz.height();
//    int bestCol=40;
    double bestQ=0;
    QPoint retPkt(-1,-1);
    QColor colS=m_rot;

    switch(pkt)
    {
        case ROT1:      // oben links
            xMax=ganz.width()/2;
            yMax=ganz.height()/4;
            break;
        case ROT2:      // oben rechts
            xMin=ganz.width()/2;
            yMax=ganz.height()/4;
            break;
        case ROT3:      // unten links
            xMax=ganz.width()/2;
            yMin=ganz.height()*3/4;
            break;
        case GRUEN:
            colS=m_gruen;
            break;
    }

    for(x=xMin; x<xMax;x+=step)
    {
        for(y=yMin;y<yMax;y+=step)
        {
            if(col_dif(ganz.pixelColor(x,y),colS)<25)
            {
                QPoint mpkt(x,y);
                double quali=adjColCircle(ganz,mpkt,colS,90);
                if( quali>bestQ)
                {
                    bestQ=quali;
                    retPkt=mpkt;
                }
            }
        }
    }
    return retPkt;
 }
int Rahmen::col_dif(QColor col1,QColor col2)
{
 //   qDebug() << "Col 1 h=" << col1.hue() << "s=" << col1.saturation()<<"v="<<col1.value()<<"\n";
 //   qDebug() << "Col 2 h=" << col2.hue() << "s=" << col2.saturation()<<"v="<<col2.value()<<"\n";
    int dColH=abs(col1.hue()-col2.hue());
    if(dColH>180)
        dColH=360-dColH;

    int dColV=abs(col1.value()-col2.value());
    int dColS=abs(col1.saturation()-col2.saturation());
    return (dColH +dColS/2 +dColV/2)/3;

}
int Rahmen::col_dif(QColor col,int hue)
{
    int dCol=abs(col.hue()-hue);
    if(dCol>180)
        dCol=360-dCol;
    return dCol;
}
int Rahmen::durchm_XY(QImage &img, QPoint &mpkt,QColor colS,bool inY)
{

    QPoint aktPt=mpkt,dPkt(0,0),maxPkt;
    QRect rImg=img.rect();
    if(inY)
        dPkt.setY(1);
    else
        dPkt.setX(1);
    aktPt+=dPkt;
    int durchm=0,err=0;
    while(err<5 && rImg.contains(aktPt))
    {
 //       qDebug()<<"Col="<<img.pixelColor(aktPt)<<"sat="<<img.pixelColor(aktPt).saturation()<<"hue="<<img.pixelColor(aktPt).hue()<<"\n";
 //       if(img.pixelColor(aktPt).saturation()<MINSAT || col_dif(img.pixelColor(aktPt),hue)>40)

        if(col_dif(img.pixelColor(aktPt),colS)>25 )
          err++;
 //       qDebug()<<aktPt<<"\n";
        aktPt+=dPkt;
    }
    maxPkt=aktPt;
    aktPt=mpkt-dPkt;
    err=0;
    while(err<5 && rImg.contains(aktPt))
    {
//        qDebug()<<"Col="<<img.pixelColor(aktPt)<<"sat="<<img.pixelColor(aktPt).saturation()<<"hue="<<img.pixelColor(aktPt).hue()<<"val="<<img.pixelColor(aktPt).value()<<"dif="<< col_dif(img.pixelColor(aktPt),m_rot) <<"\n";
//        if(img.pixelColor(aktPt).saturation()<MINSAT || col_dif(img.pixelColor(aktPt),hue)>40)
        if(col_dif(img.pixelColor(aktPt),colS)>25)
            err++;
        aktPt-=dPkt;
    }
    if(inY)
        durchm=maxPkt.y()-aktPt.y();
    else
        durchm=maxPkt.x()-aktPt.x();
    aktPt+=maxPkt;
    aktPt/=2;
    mpkt=aktPt;
    return durchm;
}
double Rahmen::adjColCircle(QImage &img, QPoint &mpkt, QColor colS, int sollD)
{
    int durchm=0;
    durchm_XY(img, mpkt,colS,false);             //verschiebt die x Koordinate von mpkt zum Mittelpunkt
    durchm=durchm_XY(img, mpkt,colS,true);       //verschiebt die y Koordinate von mpkt zum Mittelpunkt und ermittelt den y-Durchmesser
    durchm=qMin(durchm,durchm_XY(img, mpkt,colS,false))-10;     //nochmal x
    double quali1=double(durchm)/sollD, quali2=0,quali3=0;
    if(quali1>1.)
        quali1=1/quali1;
    if(quali1>0.4)
    {

        int qadratH=(durchm*6)/10;
        double richtF=0,weiss=0;
        int minX=qMax(0,mpkt.x()-qadratH),maxX=qMin (img.width(),mpkt.x()+qadratH);
        int minY=qMax(0,mpkt.y()-qadratH),maxY=qMin (img.height(),mpkt.y()+qadratH);


        for(int x=minX;x<maxX;x++)
        {
            for(int y=minY;y<maxY;y++)
            {
                QColor colA=img.pixelColor(x,y);
  //             qDebug()<<"Val="<<colA.value()<<"sat="<<colA.saturation()<<"hue="<<colA.hue();
                if(colA.saturation()>MINSAT && colA.value()>50) //farbig & hell
                {
                    if(col_dif(img.pixelColor(x,y),colS)<20)
                    {
//                        qDebug()<<" farbig";
                        richtF++;
                    }
                }
                else if(img.pixelColor(x,y).value()>100) //weiss bzw hellgrau
                {
//                    qDebug()<<" weiss";
                    weiss++;
                }
    //            qDebug()<<"\n";

            }
        }
        //im Kreis sollten Pi/4 * D^2 Punkte der richigen Farbe sein, der Rest der (maxX-minX)(maxY-minY) Punkte sollten weiss sein

        quali2=richtF/M_PI_4/durchm/durchm;
        quali3=weiss/((maxX-minX)/(maxY-minY)-M_PI_4*durchm*durchm);


        quali3=(weiss+richtF)/(maxX-minX)/(maxY-minY);
        if(quali1>1.)
            quali1=1/quali1;
        if(quali2>1.)
            quali2=1/quali2;
        if(quali3>1.)
            quali3=1/quali3;
    }

    return(quali1+quali2+quali3);

}

void Rahmen::akt_pultF()
{
    QString sFilter = "*.jpg";
    QStringList slFilter;
    slFilter << sFilter;
    m_fNamen=m_qDir.entryList(slFilter);
    m_fNamen.sort(Qt::CaseInsensitive);
    if(!m_fNamen.isEmpty())
        cutRahmen();
}


void Rahmen::on_sel_1st_clicked()
{
#ifdef QT_DEBUG
    QString path= "I:\\Dias_H\\1989_Türkei_11\\T89_01_01.png";
#else
    QString path= QFileDialog::getOpenFileName(this,tr("das erste Bild"),"",tr("png(*.png);jpeg(*.jpg);Alle(*.*)"));
#endif
    if(!path.isEmpty())
    {
        ui->la_1stP->setText(path);
        QFileInfo fi(path);

        // Ziel dir
        m_pDir=m_zDir=fi.dir();
        m_zDir.mkdir("Doku");
        m_zDir.cd("Doku");
        m_lstD="";
        QStringList namensteile=fi.baseName().split(QLatin1Char('_'));
        m_lenS=0;
        if(namensteile.size()==2)       // ab01_01.png
        {
            m_prefix=namensteile[0];
            while(m_prefix.back().isDigit())
            {
                m_prefix.chop(1);
                m_lenS++;
            }
            m_schiene=namensteile[0].right(m_lenS).toInt();
            m_start=(namensteile[1].toInt()-1)/40;
        }
        else {                           // ab99_01_01.png
            m_prefix=namensteile[0]+"_";
            m_lenS=namensteile[1].size();
            m_schiene=namensteile[1].toInt();
            m_start=(namensteile[2].toInt()-1)/40;
        }
        m_start*=40;
        m_start++;
        m_suffix=fi.suffix();
        aktSchiene();

    }
}

void Rahmen::aktSchiene()
{
    QString sFilter;
    if(m_lenS>0)
        sFilter=QString("%1%2_*.%3").arg(m_prefix).arg(m_schiene,m_lenS,10,QLatin1Char('0')).arg(m_suffix);
    else
        sFilter=QString("%1_*.%2").arg(m_prefix).arg(m_suffix);
    QStringList slFilter;
    slFilter << sFilter;
    QStringList fNamen=m_pDir.entryList(slFilter);
    m_end=0;
    QRegExp sep("[(_|.)]");
    if(!fNamen.isEmpty())
    {
        for(int i=0; i<fNamen.size();i++)
        {
            QStringList namensteile=fNamen[i].split(sep);
            m_end=qMax(m_end,namensteile[namensteile.size()-2].toInt());
        }
    }
    else
        m_end=0;
    aktfNamen();
}


void Rahmen::aktfNamen()
{
    if(m_end)
    {
        int lst=qMin(m_start+39,m_end);
        QString fstName=mkFnameR(m_start);
        ui->la_1stR->setText(fstName);
        QString lstName=mkFnameR(lst);
        ui->la_lstR->setText(lstName);
        QFileInfo fi(m_pDir,mkFnameP(m_start));
        ui->la_1stP->setText(fi.filePath());
        ui->pb_go->setDisabled(false);
    }
    else
    {
        ui->la_1stR->clear();
        ui->la_lstR->clear();
        ui->la_1stP->clear();
        ui->pb_go->setDisabled(true);
    }

}

QString Rahmen::mkFnameR(int nr)
{
    QString fName;
    if(m_lenS>0)
        fName=QString("%1%2_R%3.jpg").arg(m_prefix).arg(m_schiene,m_lenS,10,QLatin1Char('0')).arg(nr,2,10,QLatin1Char('0'));
    else
        fName=QString("%1_R%2.jpg").arg(m_prefix).arg(nr,2,10,QLatin1Char('0'));
    return fName;

}
QString Rahmen::mkFnameP(int nr)
{
    QString fName;
    if(m_lenS>0)
        fName=QString("%1%2_%3.jpg").arg(m_prefix).arg(m_schiene,m_lenS,10,QLatin1Char('0')).arg(nr,2,10,QLatin1Char('0'));
    else
        fName=QString("%1_%2.jpg").arg(m_prefix).arg(nr,2,10,QLatin1Char('0'));
    return fName;

}

QString Rahmen::mkFnameD(int nr)
{
    nr=(nr+39)/40;
    QString fName;
    if(m_lenS>0)
        fName=QString("%1%2_D%3.jpg").arg(m_prefix).arg(m_schiene,m_lenS,10,QLatin1Char('0')).arg(nr,2,10,QLatin1Char('0'));
    else
        fName=QString("%1_D%2.jpg").arg(m_prefix).arg(nr,2,10,QLatin1Char('0'));
    return fName;

}


void Rahmen::on_pb_go_clicked()
{
    writeRahmen();

    //    Pultfoto in Done-Ordner verschieben
    QFileInfo fiQ(m_qDir,m_fNamen[0]);
    QFileInfo fiD(m_dDir,m_fNamen[0]);
    QFile::rename(fiQ.filePath(),fiD.filePath());
    if(m_lenS>0)
    {
        m_start+=40;
        if(m_start>m_end)
        {
            m_start=1;
            m_schiene++;
            aktSchiene();
        }
        else
            aktfNamen();
    }
    m_fNamen.removeFirst();
    if(!m_fNamen.isEmpty())
        cutRahmen();
}

void Rahmen::on_pb_quit_clicked()
{
    qApp->quit();
}

void Rahmen::on_pb_relaod_clicked()
{
    akt_pultF();

}
void Rahmen::showQuad()
{

    ui->sb_r1x->setMaximum(m_ganz.width()/2);
    ui->sb_r1y->setMaximum(m_ganz.height()/2);

    ui->sb_r2x->setMinimum(m_ganz.width()/2);
    ui->sb_r2x->setMaximum(m_ganz.width()-1);
    ui->sb_r2y->setMaximum(m_ganz.height()/2);

    ui->sb_r3x->setMaximum(m_ganz.width()/2);
    ui->sb_r3y->setMinimum(m_ganz.height()/2);
    ui->sb_r3y->setMaximum(m_ganz.height()-1);

    ui->sb_gx->setMinimum(m_ganz.width()/2);
    ui->sb_gx->setMaximum(m_ganz.width()-1);
    ui->sb_gy->setMinimum(m_ganz.height()/2);
    ui->sb_gy->setMaximum(m_ganz.height()-1);


    ui->sb_r1x->setValue(m_quad.point(ROT1).x());
    ui->sb_r1y->setValue(m_quad.point(ROT1).y());
    ui->sb_r2x->setValue(m_quad.point(ROT2).x());
    ui->sb_r2y->setValue(m_quad.point(ROT2).y());
    ui->sb_r3x->setValue(m_quad.point(ROT3).x());
    ui->sb_r3y->setValue(m_quad.point(ROT3).y());
    ui->sb_gx->setValue(m_quad.point(GRUEN).x());
    ui->sb_gy->setValue(m_quad.point(GRUEN).y());

}

void Rahmen::on_pb_anzeigen_clicked()
{
    ui->la_deckel->setPixmap(QPixmap::fromImage(m_ganz.scaled(ui->la_deckel->size(),Qt::KeepAspectRatio)));

}

void Rahmen::on_pb_setK_clicked()
{
    m_quad.setPoint(ROT1,ui->sb_r1x->value(),ui->sb_r1y->value());
    m_quad.setPoint(ROT2,ui->sb_r2x->value(),ui->sb_r2y->value());
    m_quad.setPoint(ROT3,ui->sb_r3x->value(),ui->sb_r3y->value());
    m_quad.setPoint(GRUEN,ui->sb_gx->value(),ui->sb_gy->value());
    cutRahmen_m();

}

void Rahmen::on_pushButton_clicked()
{
    bildDrehen(90);
    on_pb_anzeigen_clicked();
}

void Rahmen::on_pushButton_2_clicked()
{
    bildDrehen(180);
    on_pb_anzeigen_clicked();
}

void Rahmen::on_pushButton_3_clicked()
{
    bildDrehen(270);
    on_pb_anzeigen_clicked();
}

void Rahmen::on_pb_hoch_clicked()
{
    m_posD.moveTop(qMin(m_40rahmen.height()-m_posD.height(),m_posD.y()+25));
    m_deckel=m_40rahmen.copy(m_posD);
    ui->la_deckel->setPixmap(QPixmap::fromImage(m_deckel.scaled(ui->la_deckel->size(),Qt::KeepAspectRatio)));

}

void Rahmen::on_pb_runter_clicked()
{
    m_posD.moveTop(qMax(0,m_posD.y()-25));
    m_deckel=m_40rahmen.copy(m_posD);
    ui->la_deckel->setPixmap(QPixmap::fromImage(m_deckel.scaled(ui->la_deckel->size(),Qt::KeepAspectRatio)));

}

void Rahmen::on_pb_links_clicked()
{
    m_posD.moveLeft(qMin(m_40rahmen.width()-m_posD.width(),m_posD.x()+25));
    m_deckel=m_40rahmen.copy(m_posD);
    ui->la_deckel->setPixmap(QPixmap::fromImage(m_deckel.scaled(ui->la_deckel->size(),Qt::KeepAspectRatio)));
}

void Rahmen::on_pb_rechts_clicked()
{
    m_posD.moveLeft(qMax(0,m_posD.x()-25));
    m_deckel=m_40rahmen.copy(m_posD);
    ui->la_deckel->setPixmap(QPixmap::fromImage(m_deckel.scaled(ui->la_deckel->size(),Qt::KeepAspectRatio)));
}
void Rahmen::changeEvent(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        //:name of the language
        break;
    default:
        break;
    }
}
void Rahmen::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_F1)
    {
        QString url="file:./Docu/"+tr("liesmich.pdf");
        QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

    }
}
