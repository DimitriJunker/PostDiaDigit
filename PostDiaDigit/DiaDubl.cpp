#include "DiaDubl.h"
#include "ui_DiaDubl.h"
#include <qfileinfo.h>
#include <qdebug.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <QDesktopServices>

DiaDubl::DiaDubl(QString ext, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiaDubl)
{
    m_ext=ext;
    ui->setupUi(this);
}

DiaDubl::~DiaDubl()
{
    delete ui;
}

void DiaDubl::on_pb_Dir_clicked()
{
#ifdef QT_DEBUG
    QString path= "I:\\dias_D";
#else
    QString path= QFileDialog::getExistingDirectory(this,tr("Quellverzeichnis"),"");
#endif
    if(!path.isEmpty())
    {
        ui->la_dir->setText(path);
        QDir dir;
        dir.setPath(path);
        chkDir(dir);
        qDebug() <<m_dubletten.size()/2;
        filter();
        on_pb_Nxt_clicked();
    }

}
void DiaDubl::chkDir(QDir dir)
{
    if (!dir.exists())
        return;

    QStringList const dirs = dir.entryList(QDir::NoDot | QDir::NoDotDot | QDir::Dirs);
    for (QString const& name: dirs)
    {
        if(name.compare("doku",Qt::CaseInsensitive))
        {
            QDir sub=dir;
            sub.cd(name);
            chkDir(sub);
        }
    }
    QString filter=QString("*.%1").arg(m_ext);
    qDebug() <<filter;
    QStringList const bilder = dir.entryList(QStringList() << filter, QDir::Files);

    double lstLen=1.;
    QString lstPath="";
    for (QString const& name: bilder)
    {
        QFileInfo fi(dir,name);
        double aktLen=fi.size();
        double untersch=unterschied(aktLen,lstLen);
        if(untersch<.02)
        {
            m_dubletten.append(lstPath);
            m_dubletten.append(fi.filePath());
        }
        lstPath=fi.filePath();
        lstLen=aktLen;

    }
}
double DiaDubl::unterschied(double a, double b)
{
    if(a>b)
        return(a/b)-1;
    else
        return(b/a)-1;
}

void DiaDubl::on_pb_Nxt_clicked()
{
    if(!m_dubletten.isEmpty())
    {
        ui->la_verd->setText(QString::number(m_dubletten.size()));
        ui->la_B_Pfad->setText(m_dubletten.first());
        QPixmap pix1(m_dubletten.first());
        m_dubletten.removeFirst();
        QPixmap pix2(m_dubletten.first());
        m_dubletten.removeFirst();
        ui->la_Bild1->setPixmap(pix1.scaled(700,700,Qt::KeepAspectRatio));
        ui->la_Bild2->setPixmap(pix2.scaled(700,700,Qt::KeepAspectRatio));
        qApp->processEvents();
        filter();
    }

}
void DiaDubl::filter()
{
    bool behalten=false;
    ui->pb_Nxt->setEnabled(false);
    while(!behalten && !m_dubletten.isEmpty())
    {
        QImage im1(m_dubletten[0]);
        QImage im2(m_dubletten[1]);
        if(unterschied(im1.width(),im2.width())<.1)
        {
            QImage pix1=im1.scaled(3,3);
            QImage pix2=im2.scaled(3,3);
            double divCol=0;
            for(int x=0;x<3;x++)
            {
                for(int y=0;y<3;y++)
                {
                    divCol+=unterschied(pix1.pixelColor(0,0).red(),pix2.pixelColor(0,0).red());
                    divCol+=unterschied(pix1.pixelColor(0,0).green(),pix2.pixelColor(0,0).green());
                    divCol+=unterschied(pix1.pixelColor(0,0).blue(),pix2.pixelColor(0,0).blue());
                }
            }
            behalten=(divCol<1);
        }
        if(!behalten)
        {
            m_dubletten.removeFirst();
            m_dubletten.removeFirst();
        }
    }
    ui->la_verd->setText(QString::number(m_dubletten.size()));
    ui->pb_Nxt->setEnabled(true);
}

void DiaDubl::on_pb_Abbruch_clicked()
{
    reject();
}
void DiaDubl::changeEvent(QEvent *e)
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
void DiaDubl::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_F1)
    {
        QString url="file:./Docu/"+tr("liesmich.pdf");
        QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

    }
}
