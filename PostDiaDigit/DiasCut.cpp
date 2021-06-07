#include "DiasCut.h"
#include "ui_DiasCut.h"
#include <QtCore>
#include <QtGui>
#include <qfiledialog.h>
#include <qimage.h>
#include <QProcess>
#include <QMessageBox>
//#include <exif.h>
#include <qcommandlineparser.h>
#include <QtConcurrent/QtConcurrent>
#include <QDesktopServices>

DiasCut::DiasCut(QString ext1,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiasCut)
{
    ui->setupUi(this);
    QSettings settings("Dimitri-Junker.de","Dias_cut");
    m_maske="*."+ext1;
    m_opt.m_maxBilder=settings.value("NoS4M",100).toInt();
    m_opt.m_minLen=settings.value("minLen",-1).toInt();
    m_opt.m_minW=settings.value("minArc",100).toDouble();
    m_opt.m_pLanguage=&m_language;
    QString lang=settings.value("lang").toString();
    if(lang.isEmpty())
        lang=QLocale::system().name().left(2);
    if(lang=="de")
        qApp->removeTranslator(&m_language);
    else
    {
        m_language.load("dias_"+lang);
        qApp->installTranslator(&m_language);
    }
    QStringList pars=QApplication::instance()->arguments();
    m_prog=nullptr;
    if(pars.size()>1)
    {
       QFileInfo fi(pars[1]);
       QString fname=fi.fileName();
       if(fname.startsWith("cut",Qt::CaseInsensitive))
           schneiden(pars[1]);
       else
           runAuto(pars[1]);
    }
}

void DiasCut::changeEvent(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        //:name of the language
//        ui->cb_lang->setCurrentText(tr("Deutsch"));
        break;
    default:
        break;
    }
}

DiasCut::~DiasCut()
{
    delete ui;
}

void DiasCut::on_pb_selPict_clicked()
{
#ifdef QT_DEBUG
    QString path= "L:\\dias_neu\\87CP_2\\jpg";
#else
    QString path= QFileDialog::getExistingDirectory(this,tr("Quellverzeichnis"),"");

#endif
    if(!path.isEmpty())
        runAuto(path);
}


void DiasCut::runAuto(QString path)
{
    qDebug() << QTime::currentTime().toString() << "runAuto \n";

//Dir und Filetyp ermitteln

    setDir(path);
//    QString sFilter = "*."+m_ext1;
    QStringList slFilter;
    slFilter << m_maske;
    ui->la_maske->setText(m_maske);
    QStringList flist=m_qdir.entryList(slFilter);
    if(flist.isEmpty())
        return;

// Alle Bilder so überlagern, daß ein weißes Kreuz gebildet werden sollte
    QFileInfo fi_Bilder(m_qdir,flist.at(1));
    QImage im_KreuzG8=QImage(fi_Bilder.filePath()).convertToFormat(QImage::Format_Grayscale8);

    //Anzahl Bilder die dazu berücksichtigt werden sollen
    int maxBilder=qMin(m_opt.m_maxBilder+2,flist.size());
    QPainter painter(&im_KreuzG8);
    painter.setCompositionMode(QPainter::CompositionMode_Lighten);

    m_prog= new QProgressDialog("", tr("Abbruch"), 0, maxBilder+11, nullptr);
    m_prog->setWindowModality(Qt::WindowModal);
    m_prog->setValue(0);
    m_prog->setWindowTitle(tr("Berechne Schnittmaske"));
    for(int i=2;i<maxBilder;i++)
    {
        if(!flist.at(i).contains("_01."))           // 1.Bild einer Schiene ggf etwas verschoben
        {
            fi_Bilder.setFile(m_qdir,flist.at(i));
            painter.drawImage(0, 0, QImage(fi_Bilder.filePath()));
        }
        m_prog->setValue(i);
    }

// Kreuz sichern
    QFileInfo fi_Kreuz(m_qdir,"Kreuz_Grau.bmp");
    im_KreuzG8.save(fi_Kreuz.filePath());

//Maske Berechnen
    qDebug() << QTime::currentTime().toString() << "maskeBerechnen1 \n";

    double bestWinkel=0;
    QRect bestRectB,bestRectH;
    int progval=0;
    if(m_prog==nullptr)
    {
        m_prog= new QProgressDialog("", tr("Abbruch"), 0, 11, nullptr);
        m_prog->setWindowModality(Qt::WindowModal);
        m_prog->setValue(0);
        m_prog->setWindowTitle(tr("Berechne Schnittmaske"));
    }
    else
        progval=m_prog->value();

    m_prog->setMaximumWidth(300);
    //Umriss bilden
    //Falls Kreuz noch nicht Monochrome, dann wandeln
    QImage kreuzMon1;
    QDir di(fi_Kreuz.absoluteDir());

   // nach SW wandeln
    kreuzMon1=QImage(im_KreuzG8.width(),im_KreuzG8.height(),QImage::Format_Mono);
    kreuzMon1.fill(0);
    for(int x=0;x<im_KreuzG8.width();x++)
    {
        for(int y=0;y<im_KreuzG8.height();y++)
        {
            if(im_KreuzG8.pixelColor(x,y).lightness()>40)
                kreuzMon1.setPixel(x,y,1);
        }
    }

    qDebug() << QTime::currentTime().toString() << "maskeBerechnen2 \n";
    m_prog->setValue(progval+2);

    if(!kreuzMon1.isNull())
    {

//um einen sauberen Umriss zu erhalten Bild vergrößern und die unteren Ecken abrunden damit es keine übereck Kontakte gibt

        QImage kreuzMon2(kreuzMon1.width()*2,kreuzMon1.height()*2,QImage::Format_Mono);
        m_size=kreuzMon1.size();
        QPainter dopp(&kreuzMon2);
        dopp.drawImage(kreuzMon2.rect(),kreuzMon1);

        for(int y=2; y<(kreuzMon2.height());y+=2)
        {
            for(int x=2; x<(kreuzMon2.width()-2);x+=2)
            {
                if(kreuzMon2.pixelIndex(x,y) && !kreuzMon2.pixelIndex(x,y-1))   //Obere Kannte
                {
                    if(!kreuzMon2.pixelIndex(x-1,y))   // linke Ecke der obere Kannte
                        kreuzMon2.setPixel(x,y,0);
                    if(!kreuzMon2.pixelIndex(x+2,y))   // rechte Ecke der obere Kannte
                        kreuzMon2.setPixel(x+1,y,0);
                }
            }
        }
        for(int y=1; y<(kreuzMon2.height()-1);y+=2)
        {
            for(int x=2; x<(kreuzMon2.width()-2);x+=2)
            {
                if(kreuzMon2.pixelIndex(x,y) && !kreuzMon2.pixelIndex(x,y+1))   //untere Kannte
                {
                    if(!kreuzMon2.pixelIndex(x-1,y))   // linke Ecke der unteren Kannte
                        kreuzMon2.setPixel(x,y,0);
                    if(!kreuzMon2.pixelIndex(x+2,y))   // rechte Ecke der unteren Kannte
                        kreuzMon2.setPixel(x+1,y,0);
                }
            }
        }
        qDebug() << QTime::currentTime().toString() << "maskeBerechnen 3\n";
        m_prog->setValue(progval+4);

        //zum Umriss bilden per XOR mit dem um ein Pixel verschobebnen Bild überlagern, einmal horizontal, einmal vertikal, diese dann per OR überlagern
        //horizontaler Anteil
        QImage kreuzH=kreuzMon2;
        QPainter painterH(&kreuzH);
        painterH.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painterH.drawImage(0, 1, kreuzMon2);

        //vertikaler Anteil
        QImage kreuzV=kreuzMon2;
        QPainter painterV(&kreuzV);
        painterV.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painterV.drawImage(1, 0, kreuzMon2);
        m_prog->setValue(progval+7);


        //beide überlagern
        painterV.setCompositionMode(QPainter::RasterOp_SourceOrDestination);
        painterV.drawImage(0, 0, kreuzH);
        painterV.drawImage(1, 0, kreuzH);       //sonst fehlt unten rechts

        fi_Kreuz.setFile(di,"kreuz_m2.bmp");
        kreuzMon2.save(fi_Kreuz.filePath());
        fi_Kreuz.setFile(di,"kreuz_u2.bmp");
        kreuzV.save(fi_Kreuz.filePath());

    // Umriss suchen Bildmitte sollte teil der Dias sein, von dort nach oben gehen bis eine Linie gefunden wird
        double minAbweichung = 1000000;        // Bewertung ob ein Umriss der gesuchte sein könnte

        int suchX=kreuzV.width()/2,suchY=kreuzV.height()/2;
        qDebug() << QTime::currentTime().toString() << "maskeBerechnen 4\n";
        m_prog->setValue(progval+10);

        while(suchY>-1)
        {
            if(kreuzV.pixelIndex(suchX,suchY))
            {
                int aktX=suchX,aktY=suchY;
                m_cont.clear();      //evtuell ja vorher schon Müll gefunden;
                while(1)
                {
                   kreuzV.setPixel(aktX,aktY,0);
                   m_cont.append(QPoint(aktX,aktY));

                   if(kreuzV.pixelIndex(aktX+1,aktY))         //rechts gesetzt?
                       aktX++;
                   else if(kreuzV.pixelIndex(aktX,aktY-1))      //oben gesetzt?
                       aktY--;
                   else if(kreuzV.pixelIndex(aktX-1,aktY))      //links gesetzt?
                       aktX--;
                   else if(kreuzV.pixelIndex(aktX,aktY+1))      //unten gesetzt?
                       aktY++;
                   else     //Ende oder Sackgasse
                   {
                       int dist=qAbs(m_cont[0].x()-aktX);
                       dist+=qAbs(m_cont[0].y()-aktY);
                       if(dist>1 && m_cont.size()>2)       //Sackgasse
                       {
                           aktX=m_cont[m_cont.size()-2].x();
                           aktY=m_cont[m_cont.size()-2].y();
                           m_cont.removeLast();
                           m_cont.removeLast();
                       }
                       else
                           break;
                   }
                }

//kurze Umrisse ignorieren, sind wahrscheinlich Müll
                int min_len=m_opt.m_minLen*2;
                if(min_len<0)
                    min_len=kreuzV.width();
                int xMax=kreuzV.width()/4,xMaxI=-1,xMin=kreuzV.width()/4,xMinI=-1,yMax=kreuzV.height()/4,yMaxI=-1,yMin=kreuzV.height()/4;
                qDebug() << QTime::currentTime().toString() << "maskeBerechnen5 \n";

                if(m_cont.size()>min_len)
                {
                    //Die Verdopplung rückgängig machen und die Extreme suchen
                   int i=1;
                   m_cont[0]/=2;
                   while(i<m_cont.size())
                   {
                       m_cont[i]/=2;
                       if(m_cont[i]==m_cont[i-1])
                           m_cont.removeAt(i);
                       else
                       {
                           if(xMax<m_cont[i].x())
                           {
                               xMax=m_cont[i].x();
                               xMaxI=i;
                           }
                           if(xMin>m_cont[i].x())
                           {
                               xMin=m_cont[i].x();
                               xMinI=i;
                           }
                           if(yMax<m_cont[i].y())
                           {
                               yMax=m_cont[i].y();
                               yMaxI=i;
                           }
                           if(yMin>m_cont[i].y())
                               yMin=m_cont[i].y();  //yMinI wird nicht benötigt, statt dessen Anfang/Ende
                           i++;
                       }
                   }
// Das Kreuz besteht aus 4 Geraden von 24mm und 8 Geraden von 6mm, die Ecken sind ggf abgerundet, deshalb suche ich 18mm bzw 4mm lange Geraden, gespeichert wird der Index ihrer Mitte
//Da durch den Suchstart in der Bildmitte der Umriss wahrscheinlich in der Mitte der oberen Seite beginnt und im Uhrzeigersinn geht ist die erste komplette 24mm Seite die rechte

//                   savePkt();        //Pkt Liste zu debug Zwecken sichern
                   qDebug() << QTime::currentTime().toString() << "maskeBerechnen 6\n";

                   int len=m_cont.size();
                    m_dpmm=len/144;                                     // UmrissPixel pro mm

                    m_ppmm=(xMax-xMin+yMax-yMin)/72; // Pixel pro mm, von oben bis unten und von links bis rechts sind es je 36mm
                    int ecke[12];
                    ecke[0]=suchEckeHV(0,m_ppmm/2);        //portrait or
                    ecke[2]=suchEckeVH(xMaxI,-m_ppmm/2);   //Breitk or
                    ecke[3]=suchEckeVH(xMaxI,m_ppmm/2);    //Breitk ur
                    ecke[5]=suchEckeHV(yMaxI,-m_ppmm/2);   //portrait ur
                    ecke[6]=suchEckeHV(yMaxI,m_ppmm/2);    //portrait ul
                    ecke[8]=suchEckeVH(xMinI,-m_ppmm/2);   //Breitk ul
                    ecke[9]=suchEckeVH(xMinI,m_ppmm/2);    //Breitk ol
                    ecke[11]=suchEckeHV(len-1,-m_ppmm/2);   //portrait ol  nicht yMinI da die obere Seite geteilt ist
//jetzt noch die Innenecken
                    ecke[1]=suchEckeVH(ecke[0]+(ecke[2]-ecke[0])/4,m_ppmm/2);  //Innenecke or
                    ecke[4]=suchEckeHV(ecke[3]+(ecke[5]-ecke[3])/4,m_ppmm/2);  //Innenecke ur
                    ecke[7]=suchEckeVH(ecke[6]+(ecke[8]-ecke[6])/4,m_ppmm/2);  //Innenecke ul
                    ecke[10]=suchEckeHV(ecke[9]+(ecke[11]-ecke[9])/4,m_ppmm/2);  //Innenecke ol



                    //Regressionsgeraden berechnen
                    double winkelI[8];      // je 4 für Portrait / Portrait
                    winkelI[0]=regressionWinkel(false,ecke[9],ecke[10],ecke[1],ecke[2]);    // Breitkannt oben
                    winkelI[1]=regressionWinkel(true,ecke[2],ecke[3]);                      // Breitkannt rechts
                    winkelI[2]=regressionWinkel(false,ecke[3],ecke[4],ecke[7],ecke[8]);     // Breitkannt unten
                    winkelI[3]=regressionWinkel(true,ecke[8],ecke[9]);                      // Breitkannt links

                    winkelI[4]=regressionWinkel(false,ecke[11],m_cont.size()-1,0,ecke[0]);  // Portrait oben von Ecke 11 bis zum Ende und der Anfang
                    winkelI[5]=regressionWinkel(true,ecke[0],ecke[1],ecke[4],ecke[5]);      // Portrait rechts
                    winkelI[6]=regressionWinkel(false,ecke[5],ecke[6]);                     // Portrait unten
                    winkelI[7]=regressionWinkel(true,ecke[6],ecke[7],ecke[10],ecke[11]);    // Portrait links

                    //Drehwinkel bestimmen
                    double winkel=0;
                    for(int i=0;i<8;i++)
                       winkel+=fmod(winkelI[i]+135,90)-45;
                    winkel/=8;
                    //Trapezwinkel bestimmen 1.: Neigung oben unten
                    double winkelOU=fmod(winkelI[1]+180,180)-fmod(winkelI[3]+180,180);  //Unterschied Winkel rechts links Breitkannt
                    winkelOU+=fmod(winkelI[5]+180,180)-fmod(winkelI[7]+180,180);  //Unterschied Winkel rechts links Portrait
                    winkelOU/=2;  //Durchschnitt
                    //Trapezwinkel bestimmen:2.: Neigung rechts links
                    double winkelRL=fmod(winkelI[0]+180,180)-fmod(winkelI[2]+180,180);  //Unterschied Winkel oben unten Breitkannt
                    winkelRL+=fmod(winkelI[4]+180,180)-fmod(winkelI[6]+180,180);  //Unterschied Winkel oben unten Portrait
                    winkelRL/=2;  //Durchschnitt

                    ui->la_qw->setText(QString::number(winkel));
                    ui->la_qou->setText(QString::number(winkelOU));
                    ui->la_qrl->setText(QString::number(winkelRL));
                    // Falls gewünscht Winkel ignorieren
                    if(fabs(winkel)<m_opt.m_minW)
                        winkel=0;
                    else
                        winkel*=-1;

                    QRect breitk,portrait;
                    breitk.setX(regressionV(winkel,ecke[8],ecke[9]));                           // Breitkannt links
                    breitk.setY(regressionH(winkel,ecke[9],ecke[10],ecke[1],ecke[2]));            // Breitkannt oben
                    breitk.setWidth(regressionV(winkel,ecke[2],ecke[3])-breitk.x());            // Breitkannt Breite
                    breitk.setHeight(regressionH(winkel,ecke[3],ecke[4],ecke[7],ecke[8])-breitk.y());// Breitkannt Höhe

                    portrait.setY(regressionH(winkel,ecke[11],m_cont.size()-1,0,ecke[0]));                            // Portrait oben
                    portrait.setX(regressionV(winkel,ecke[6],ecke[7],ecke[10],ecke[11]));                 // Portrait links
                    portrait.setHeight(regressionH(winkel,ecke[5],ecke[6])-portrait.y());            // Portrait Höhe
                    portrait.setWidth(regressionV(winkel,ecke[0],ecke[1],ecke[4],ecke[5])-portrait.x()); // Portrait Breite
//Qualität prüfen und ggf kleine Korrekturen
// wenn man die langen Seiten durch 1.5 teilt sollten alle 4 gleich lang sein
                    double bw=breitk.width()/1.5;
                    double bh=breitk.height();
                    double hw=portrait.width();
                    double hh=portrait.height()/1.5;

                    double max12mm=qMax(qMax(bw,bh),qMax(hw,hh))/2;
                    double min12mm=qMin(qMin(bw,bh),qMin(hw,hh))/2;
                    double abweichung=(max12mm/min12mm)-1.;
                    if(abweichung<0.01)     // Wahrscheinlich stimmt das größte
                    {
                        QPoint mpkt;
                        // Mittelpunkt bestimmen
                        //x-Koordinate
                        if(bw>hw)
                            mpkt.setX(breitk.x()+breitk.width()/2);
                        else
                            mpkt.setX(portrait.x()+portrait.width()/2);
                        //y-Koordinate
                        if(bh>hh)
                            mpkt.setY(breitk.y()+breitk.height()/2);
                        else
                            mpkt.setY(portrait.y()+portrait.height()/2);
                        //obere linke Ecken bestimmen
                        breitk.setLeft(int(mpkt.x()-(max12mm*1.5)+.5));
                        breitk.setRight(int(mpkt.x()+(max12mm*1.5)+.5));
                        breitk.setTop(int(mpkt.y()-max12mm+.5));
                        breitk.setBottom(int(mpkt.y()+max12mm+.5));

                        portrait.setLeft(int(mpkt.x()-max12mm+.5));
                        portrait.setRight(int(mpkt.x()+max12mm+.5));
                        portrait.setTop(int(mpkt.y()-(max12mm*1.5)+.5));
                        portrait.setBottom(int(mpkt.y()+(max12mm*1.5)+.5));

                        suchY=-1;       //dann muß man nicht weiter suchen
                    }
                    if(abweichung<minAbweichung)    //dann ist es die bisher beste Kurve
                    {
                        minAbweichung=abweichung;
                        saveCut(di,winkel , breitk,portrait);
                        bestWinkel=winkel;
                        bestRectB=breitk;
                        bestRectH=portrait;
                    }
                    break;
                }
           }
           suchY--;
       }
        qDebug() << QTime::currentTime().toString() << "maskeBerechnen7 \n";
       BOOL weiter=TRUE;
       if(minAbweichung>0.1)
       {
           QMessageBox::StandardButton reply;
           reply=QMessageBox::warning(nullptr, tr("schlechte Schnittmaske"), tr("Die Schnittmaske ist nicht gut! Trotdem schneiden?"),
                                         QMessageBox::Yes|QMessageBox::Abort);
           if(reply==QMessageBox::Abort)
               weiter=FALSE;
       }
       m_prog->close();
       delete m_prog;
       m_prog=nullptr;
       if(weiter)
           schneiden(bestWinkel , bestRectB,bestRectH);

    }
}
//sucht ein horizontales Stück des Umrisses mit Länge len um den geschätzten Punkt gesch herum

int DiasCut::suchEckeVH(int anf, int step)
{
    int len=m_ppmm*10;
    if(step<0)
        len*=-1;
    while(qAbs(len)>20)
    {
        while(qAbs(m_cont[anf].y()-m_cont[anf+len].y())>qAbs(m_cont[anf].x()-m_cont[anf+len].x()))
              anf+=step;
        len*=.8;
    }
    return anf;
}
int DiasCut::suchEckeHV(int anf, int step)
{
    int len=m_ppmm*10;
    if(step<0)
        len*=-1;
    while(qAbs(len)>20)
    {
        while(qAbs(m_cont[anf].y()-m_cont[anf+len].y())<qAbs(m_cont[anf].x()-m_cont[anf+len].x()))
              anf+=step;
        len*=.8;
    }
    return anf;
}

// besimmt eine Regressionsgerade entweder durch ein 24mm Stück oder durch 2 6mm/12mm Stücke.  x=ay+b zurückgegeben wird aber nur der Winkel in Grad zwischen -90 und 90
// da es bei einer fast senkrechten zu großen Fehlern kommt vertausche ich x und y im Vergleich zur normalen
double DiasCut::regressionWinkel(bool isVertikal, int ecke1, int ecke2, int ecke3, int ecke4)
{
//Mittelwerte bestimmen
    double mx,my,sx=0,sy=0 , winkel;
    //Die Ecken ignorieren
    ecke1+=m_ppmm;
    if(ecke3==-1)        // eine zusammenhängende 24mm Seite bestimmen
        ecke2-=m_ppmm;
    else if(ecke3>0)    // eine 36mm Seite aus 2 6mm Stücken bestimmen
    {
        ecke2-=m_ppmm;
        ecke3+=m_ppmm;
        ecke4-=m_ppmm;
    }
    else             // die obere 24mm Seite aus 2 12mm Stücken bestimmen
        ecke4-=m_ppmm;

    for(int pos=ecke1;pos<ecke2;pos++)
    {
        sx+=m_cont[pos].x();
        sy+=m_cont[pos].y();
    }
    int len=ecke2-ecke1;
    if(ecke3>-1)
    {
        len+=ecke4-ecke3;
        for(int pos=ecke3;pos<ecke4;pos++)
        {
            sx+=m_cont[pos].x();
            sy+=m_cont[pos].y();
        }
    }
    mx=sx/len;
    my=sy/len;
    // Parameter a bestimmen
    double n=0,z=0;     // Nenner und Zähler
    if(isVertikal)
    {
        for(int pos=ecke1;pos<ecke2;pos++)
        {
            double dy=(m_cont[pos].y()-my);
            n+=dy*(m_cont[pos].x()-mx);
            z+=dy*dy;
        }
        if(ecke3>-1)
        {
            for(int pos=ecke3;pos<ecke4;pos++)
            {
                double dy=(m_cont[pos].y()-my);
                n+=dy*(m_cont[pos].x()-mx);
                z+=dy*dy;
            }
        }
        double a=n/z;

        winkel=atan(a)*180/M_PI+90;
        if(winkel>90.)
            winkel-=180;

    }
    else
    {
        for(int pos=ecke1;pos<ecke2;pos++)
        {
            double dx=(m_cont[pos].x()-mx);
            n+=dx*(m_cont[pos].y()-my);
            z+=dx*dx;
        }
        if(ecke3>-1)
        {
            for(int pos=ecke3;pos<ecke4;pos++)
            {
                double dx=(m_cont[pos].x()-mx);
                n+=dx*(m_cont[pos].y()-my);
                z+=dx*dx;
            }
        }
        double a=n/z;

        winkel=atan(a)*180/M_PI;

    }

    return winkel;

}

// Berechnet eine horizontale Regressionsgerade, also nur den Mittwert der Y-Koordinaten, ggf nach Drehung um arc
int DiasCut::regressionH(double arc,int ecke1, int ecke2, int ecke3, int ecke4)
{

    double my,sy=0;
    //Die Ecken ignorieren
    ecke1+=m_ppmm;
    if(ecke3==-1)        // eine zusammenhängende 24mm Seite bestimmen
        ecke2-=m_ppmm;
    else if(ecke3>0)    // eine 36mm Seite aus 2 6mm Stücken bestimmen
    {
        ecke2-=m_ppmm;
        ecke3+=m_ppmm;
        ecke4-=m_ppmm;
    }
    else             // die obere 24mm Seite aus 2 12mm Stücken bestimmen
        ecke4-=m_ppmm;

    if(arc==0.0)
    {
        for(int pos=ecke1;pos<ecke2;pos++)
            sy+=m_cont[pos].y();
        int len=ecke2-ecke1;
        if(ecke3>-1)
        {
            len+=ecke4-ecke3;
            for(int pos=ecke3;pos<ecke4;pos++)
                sy+=m_cont[pos].y();
        }
        my=sy/len;
    }
    else
    {
        QTransform trRot;
        trRot.translate(m_size.width()/2,m_size.height()/2);
        trRot.rotate(arc);
        trRot.translate(-m_size.width()/2,-m_size.height()/2);
        for(int pos=ecke1;pos<ecke2;pos++)
            sy+=trRot.map(m_cont[pos]).y();
        int len=ecke2-ecke1;
        if(ecke3>-1)
        {
            len+=ecke4-ecke3;
            for(int pos=ecke3;pos<ecke4;pos++)
                sy+=trRot.map(m_cont[pos]).y();
        }
        my=sy/len;
    }

    return int(my+.5);
}
//wie das vorige nur vertikal
int DiasCut::regressionV(double arc,int ecke1, int ecke2, int ecke3, int ecke4)
{

    double mx,sx=0;
    //Die Ecken ignorieren
    ecke1+=m_ppmm;
    if(ecke3==-1)        // eine zusammenhängende 24mm Seite bestimmen
        ecke2-=m_ppmm;
    else if(ecke3>0)    // eine 36mm Seite aus 2 6mm Stücken bestimmen
    {
        ecke2-=m_ppmm;
        ecke3+=m_ppmm;
        ecke4-=m_ppmm;
    }
    else             // die obere 24mm Seite aus 2 12mm Stücken bestimmen
        ecke4-=m_ppmm;
    if(arc==0.0)
    {
        for(int pos=ecke1;pos<ecke2;pos++)
            sx+=m_cont[pos].x();
        int len=ecke2-ecke1;
        if(ecke3>-1)
        {
            len+=ecke4-ecke3;
            for(int pos=ecke3;pos<ecke4;pos++)
                sx+=m_cont[pos].x();
        }
        mx=sx/len;
    }
    else
    {
        QTransform trRot;
        trRot.translate(m_size.width()/2,m_size.height()/2);
        trRot.rotate(arc);
        trRot.translate(-m_size.width()/2,-m_size.height()/2);
        for(int pos=ecke1;pos<ecke2;pos++)
            sx+=trRot.map(m_cont[pos]).x();
        int len=ecke2-ecke1;
        if(ecke3>-1)
        {
            len+=ecke4-ecke3;
            for(int pos=ecke3;pos<ecke4;pos++)
                sx+=trRot.map(m_cont[pos]).x();
        }
        mx=sx/len;
    }

    return int(mx+.5);
}

void DiasCut::savePkt()
{
    QFileInfo fi(m_qdir,"pkt.csv");
    QFile fCut(fi.filePath());
    if(fCut.open(QIODevice::WriteOnly))
    {
        QTextStream oCut(&fCut);
        int len=m_cont.size();
        for(int i=0; i<len;i++)
            oCut << i << " , "<< m_cont[i].x() << " , "<< m_cont[i].y()<< "\n";

        fCut.close();

    }
}


void DiasCut::saveCut(QDir di,double arc, QRect bk, QRect hk)
{
    QFileInfo fi(di,"cut.csv");
    ui->la_cut->setText(fi.filePath());
    QFile fCut(fi.filePath());
    if(fCut.open(QIODevice::WriteOnly))
    {
        QTextStream oCut(&fCut);
        oCut << tr("Infos zum Schneiden von Bildern\n");
        oCut << tr("Winkel , ") << arc << "\n";
        oCut << tr("Bildtyp, x , y, Breite , Höhe>\n");

        oCut << "Portrait , " << bk.x() << " , " << bk.y() <<  " , "  << bk.width() <<  " , "  << bk.height() << "\n";
        oCut << "partrait , " << hk.x() << " , " << hk.y() <<  " , "  << hk.width() <<  " , "  << hk.height() << "\n";
        fCut.close();
    }
}

void DiasCut::on_pb_Load_cut_clicked()
{

    QString path= QFileDialog::getOpenFileName(this,tr("eine 'cut' Datei"),"","cut.csv(cut.csv)");
    if(!path.isEmpty())
        schneiden(path);
}


void DiasCut::schneiden(QString path)
{

    ui->la_cut->setText(path);
    QFile fCut(path);
    QFileInfo fi(path);
    setDir(fi.absoluteDir());
    if(fCut.open(QIODevice::ReadOnly))
    {
        QTextStream iCut(&fCut);
        iCut.readLine();   //ignorieren
        QStringList data;
        data=iCut.readLine().split(",");
        double winkel=data[1].toDouble();
        iCut.readLine();   //ignorieren
        data=iCut.readLine().split(",");

        QRect bk(data[1].toInt(),data[2].toInt(),data[3].toInt(),data[4].toInt());
        data=iCut.readLine().split(",");
        QRect hk(data[1].toInt(),data[2].toInt(),data[3].toInt(),data[4].toInt());
        fCut.close();
        schneiden(winkel,bk,hk);
    }
}

enum H_B {HUB,HOCH,BREIT};
void cut1inThread(CUTPICT *aktPict)
{
    QFileInfo file(aktPict->s_fPath);
    CUTDATA *cd=aktPict->s_data;
    if(cd->s_progD)
        cd->s_progD->setValue(cd->s_progD->value()+1);

    //Bild Laden
    QImage bildO(file.filePath()),*bild=&bildO,bildR;
//Bild rotieren
    if(cd->s_winkel !=0.0)
    {
        bildR=QImage(bildO.size(),bildO.format());
        QPainter painter(&bildR);
        int xc=bildO.width()/2,yc=bildO.height()/2;
        painter.translate(xc,yc);
        painter.rotate(cd->s_winkel);
        painter.translate(-xc,-yc);
        painter.drawImage(0,0,bildO);
        bild=&bildR;
    }
//  Breit- oder Portrait?
//  Mitte der 4 Rechtecke ermitteln:
    QPoint mitten[4];

    mitten[0].setX(cd->s_rectH.x()+cd->s_rectH.width()/2);  //oben
    mitten[0].setY((cd->s_rectH.y()+cd->s_rectB.y())/2);

    mitten[1].setX(mitten[0].x());              //unten
    mitten[1].setY((cd->s_rectH.y()+cd->s_rectB.y()+cd->s_rectH.height()+cd->s_rectB.height())/2);

    mitten[2].setX((cd->s_rectB.x()+cd->s_rectH.x())/2);   //links
    mitten[2].setY(cd->s_rectB.y()+cd->s_rectB.height()/2);

    mitten[3].setX((cd->s_rectH.x()+cd->s_rectB.x()+cd->s_rectH.width()+cd->s_rectB.width())/2);  //rechts
    mitten[3].setY(mitten[2].y());

    int abst=+cd->s_rectH.width()/20,hell[4];     //1.2mm
    for(int r=0;r<4;r++)
    {
        hell[r]=0;
        for(int x=mitten[r].x()-(abst*2);x<=(mitten[r].x()+(abst*2));x+=abst)
        {
            for(int y=mitten[r].y()-(abst*2);y<=(mitten[r].y()+(abst*2));y+=abst)
                hell[r]+=bild->pixelColor(x,y).lightness();

        }
    }
    int hellHoch=hell[0]+hell[1],hellBreit=hell[2]+hell[3];
    H_B h_oder_b=HUB;                           //unklar
    if(hellHoch<60 && hellBreit>100)
        h_oder_b=BREIT;
    else if(hellHoch>100 && hellBreit<60)
        h_oder_b=HOCH;
    QString filenamePng=file.completeBaseName()+".png";

    if(h_oder_b!=HOCH)  //breitkannt speichern
    {
        QFileInfo fiB(cd->s_diG,filenamePng);
        bild->copy(cd->s_rectB).mirrored(FALSE,TRUE).save(fiB.filePath());

    }
    if(h_oder_b!=BREIT)  //Portrait speichern
    {
        QFileInfo fiH;
        if(h_oder_b==HUB)   //dann wurde schon breitkannt gespeichert -> in anderen Ordner speichern
            fiH.setFile(cd->s_diH,filenamePng);
        else
            fiH.setFile(cd->s_diG,filenamePng);
        bild->copy(cd->s_rectH).mirrored(FALSE,TRUE).save(fiH.filePath());
    }
    delete aktPict;
    aktPict=nullptr;
}


void DiasCut::schneiden(double winkel,QRect rectB, QRect rectH)
{
    qDebug() << QTime::currentTime().toString() << "schneiden1 \n";

    QFileInfo file;
    QString sFilter=ui->la_maske->text();
    if(m_qdir.exists() && !sFilter.isEmpty())
    {
        QStringList slFilter;
        slFilter << sFilter;

        QStringList flist=m_qdir.entryList(slFilter);

        m_qdir.mkdir(tr("geschnitten"));
        QDir diG=m_qdir;
        diG.cd(tr("geschnitten"));
        m_qdir.mkdir("portrait");
        QDir diH=m_qdir;
        diH.cd("portrait");

//Files sicherheitshalber sortieren, um die Dianummer in der Schiene zu ermitteln
        flist.sort(Qt::CaseInsensitive);
        QFileInfo fiDias(diG,"dias.csv");
        QFile fDias(fiDias.filePath());
        if(fDias.open(QIODevice::WriteOnly))
        {
            QTextStream oDias(&fDias);
            oDias << "Infos zu den Dias,Version,1\n";
            oDias << "Schiene Nr ,  Bild Nr  , Scan Zeit , File name , Hell H, Hell B\n";

            int bildNr=0,schieneNr=0;
            qint64 lst_dte=0;
            QList<CUTPICT *> mmList;
            CUTDATA *cd=new CUTDATA;
            cd->s_winkel=winkel;
            cd->s_rectB=rectB;
            cd->s_rectH=rectH;
            cd->s_diG=diG;
            cd->s_diH=diH;

            for(int i=0;i<flist.size();i++)
            {
                CUTPICT *aktPict=new CUTPICT;
                aktPict->s_data=cd;

                file.setFile(m_qdir,flist.at(i));
                // Bild und Schienennummer bestimmen und in csv Datei speichern
                QDateTime dt=file.lastModified();
                aktPict->s_fPath=file.filePath();
                bildNr++;
                qint64 akt_dte=dt.toSecsSinceEpoch();
                if((akt_dte-lst_dte)>10)        //neue Schiene
                {
                    schieneNr++;
                    bildNr=1;
                }
                lst_dte=akt_dte;

                oDias << schieneNr << " , " << bildNr <<  " , " << dt.toString("dd.MM.yyyy hh:mm:ss") << " , " << flist.at(i) << "\n";
                mmList.append(aktPict);
            }
            fDias.close();
            int anzahlBilder=mmList.size();
            QProgressDialog progress("", tr("Abbruch"), 0, anzahlBilder, nullptr);
            progress.setWindowModality(Qt::WindowModal);
            progress.setValue(0);
            progress.setWindowTitle(tr("Schneide %n Bilder",nullptr,anzahlBilder));
            qDebug() << QTime::currentTime().toString() << "schneiden2 \n";
    #ifdef QT_DEBUG
            qDebug() << "Kein Multitasking";
            cd->s_progD=&progress;
            for(int pNr=0;pNr<anzahlBilder;pNr++)
                cut1inThread(mmList[pNr]);	// kein Multitasking zum Debugging
    #else

            cd->s_progD=nullptr;
//            QThreadPool::globalInstance()->setMaxThreadCount(maxThreads);
            QFutureWatcher<void> fW;
            QObject::connect(&fW, SIGNAL(finished()), &progress, SLOT(reset()));
            QObject::connect(&progress, SIGNAL(canceled()), &fW, SLOT(cancel()));
            QObject::connect(&fW, SIGNAL(progressRangeChanged(int,int)), &progress, SLOT(setRange(int,int)));
            QObject::connect(&fW, SIGNAL(progressValueChanged(int)), &progress, SLOT(setValue(int)));

            fW.setFuture(QtConcurrent::map(mmList,cut1inThread));
            progress.exec();
            fW.waitForFinished();
    #endif
            delete cd;
        }
    }
    qDebug() << QTime::currentTime().toString() << "schneiden3 \n";
}

void DiasCut::setDir(QString dirPath)
{
    ui->la_dir->setText(dirPath);
    m_qdir.setPath(dirPath);
}

void DiasCut::setDir(QDir dir)
{
    ui->la_dir->setText(dir.absolutePath());
    m_qdir=dir;
}

void DiasCut::on_pushButton_clicked()
{
    m_opt.exec();

}

void DiasCut::on_pb_Abbruch_clicked()
{
    reject();
}
void DiasCut::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_F1)
    {
        QString url="file:./Docu/"+tr("liesmich.pdf");
        QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

    }
}
