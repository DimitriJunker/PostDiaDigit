#include "dias_rename.h"
#include "ui_dias_rename.h"
#include <qfileinfo.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <QDesktopServices>



dias_rename::dias_rename(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dias_rename)
{
    ui->setupUi(this);
    QSettings settings("Dimitri-Junker.de","PostDiaDigit");
    ui->sb_delStart->setValue(settings.value("Del_1st",2).toInt());
    ui->sb_delEnde->setValue(settings.value("Del_lst",5).toInt());
    m_anzahl=settings.value("Size",50).toInt();

    ui->rb_1->setChecked(m_anzahl==1);
    ui->rb36->setChecked(m_anzahl==36);
    ui->rb_50->setChecked(m_anzahl==50);
    ui->rb_60->setChecked(m_anzahl==60);
    ui->rb_80->setChecked(m_anzahl==80);
    ui->rb_100->setChecked(m_anzahl==100);
    if(m_anzahl!=1 && m_anzahl!=36 && m_anzahl!=50 && m_anzahl!=60 && m_anzahl!=80 && m_anzahl!=100)
    {
        ui->rb_n->setChecked(true);
        ui->sb_n->setValue(m_anzahl);
    }
    else
        ui->rb_n->setChecked(false);
    ui->sb_schiene->setValue(1);
    /*
    QString lang=settings.value("lang").toString();
    if(lang.isEmpty())
        lang=QLocale::system().name().left(2);
    if(lang=="de")
        qApp->removeTranslator(&m_language);
    else
    {
        m_language.load("dias_rename_"+lang);
        qApp->installTranslator(&m_language);
    }
*/
}

void dias_rename::setExt(QString ext1, QString ext2)
{
    m_ext1=ext1;
    m_ext2=ext2;
}

void dias_rename::changeEvent(QEvent *e)
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

dias_rename::~dias_rename()
{
    delete ui;
}

void dias_rename::on_pb_dir_clicked()
{
#ifdef QT_DEBUG
     QString path= "I:\\dias_neu\\Tst_";
#else
     QString path= QFileDialog::getExistingDirectory(this,tr("Quellverzeichnis"),"");
#endif
    ui->la_dir->setText(path);
    akt_fNamen();

}
void dias_rename::akt_fNamen()
{
    QDir di(ui->la_dir->text());
    QString sFilter = "*."+m_ext1;
    QStringList slFilter;
    slFilter << sFilter;
    m_fNamen=di.entryList(slFilter);
    m_fNamen.sort(Qt::CaseInsensitive);
    akt_2do();
}

void dias_rename::akt_2do()
{
    //was soll gelöscht werden
    if(!m_fNamen.isEmpty())
    {
        ui->la_rest->setNum(m_fNamen.size());
        int delS=ui->sb_delStart->value()-1,delE=ui->sb_delEnde->value()-1,anzDel=0;
        if(delS>-1 && delS<m_fNamen.size() && delS < m_anzahl)
        {
            ui->la_delStart->setText(m_fNamen[delS]);
            if(delE<m_fNamen.size())
            {
                ui->la_delEnd->setText(m_fNamen[delE]);
                anzDel=delE-delS+1;
            }
            else
            {
                ui->la_delEnd->setText(m_fNamen[m_fNamen.size()-1]);
                anzDel=m_fNamen.size()-delS;
            }
        }
        else
        {
            ui->la_delStart->clear();
            ui->la_delEnd->clear();
        }
        //was soll umbenannt werden
        int renS=0,renE=qMin(m_anzahl-1+anzDel,m_fNamen.size()-1);
        if(!delS)   //dann wird das erste gelöscht und nicht umbenannt
            renS=anzDel;
        if(renS<m_fNamen.size())
        {
            ui->la_ren1q->setText(m_fNamen[renS]);
            ui->la_renxq->setText(m_fNamen[renE]);
            ui->la_ren1z->setText(mkFname(renS));
            ui->la_renxz->setText(mkFname(renE-anzDel));
        }
    }
}


QString dias_rename::schiebe(QDir diQ,QDir diZ,QString fNameQ,QString fNameZ)
{
    QFileInfo fiQ(diQ,fNameQ);
    QFileInfo fiZ(diZ,fNameZ);
    QFile::rename(fiQ.filePath(),fiZ.filePath());
    return(fiQ.completeBaseName());
}

QString dias_rename::mkFname(int i,bool nef)
{
    QString endung="";
    if(nef)
        endung=m_ext2;
    else
        endung=m_ext1;
    int ziffern=2;
    if(m_anzahl>99)
        ziffern=3;
    QString name=QString("%1%2%3%4.%5").arg(ui->le_prefix->text()).arg(ui->sb_schiene->value(),ziffern,10,QLatin1Char('0')).arg(ui->le_us_2->text()).arg(i+ui->sb_start->value(),2,10,QLatin1Char('0')).arg(endung);
    return name;
}


void dias_rename::on_rb_1_clicked()
{
    m_anzahl=1;
    akt_2do();
}

void dias_rename::on_rb36_clicked()
{
    m_anzahl=36;
    akt_2do();
}

void dias_rename::on_rb_50_clicked()
{
    m_anzahl=50;
    akt_2do();
}

void dias_rename::on_rb_100_clicked()
{
    m_anzahl=100;
    akt_2do();
}

void dias_rename::on_rb_n_clicked()
{
    m_anzahl=ui->sb_n->value();
    akt_2do();
}

void dias_rename::on_rb_80_clicked()
{
    m_anzahl=80;
    akt_2do();
}

void dias_rename::on_sb_n_valueChanged(int arg1)
{
    if(ui->rb_n->isChecked())
    {
        m_anzahl=arg1;
        akt_2do();
    }
}

void dias_rename::on_le_prefix_editingFinished()
{
    ui->sb_start->setValue(1);
    akt_2do();
}

void dias_rename::on_sb_schiene_valueChanged(int arg1)
{
    ui->sb_start->setValue(1);
    akt_2do();
}

void dias_rename::on_lineEdit_editingFinished()
{
    akt_2do();
}

void dias_rename::on_sb_start_valueChanged(int arg1)
{
    akt_2do();
}

void dias_rename::on_le_jpg_editingFinished()
{
    akt_fNamen();
}

void dias_rename::on_sb_delStart_valueChanged(int arg1)
{
    ui->sb_delEnde->setMinimum(arg1);
    if(ui->sb_delEnde->value()<arg1)
        ui->sb_delEnde->setValue(arg1);
    akt_2do();
}

void dias_rename::on_sb_delEnde_valueChanged(int arg1)
{
    akt_2do();
}

void dias_rename::on_pB_go_clicked()
{
    if(!m_fNamen.isEmpty())
    {
        // neue dir
        QDir dirQ(ui->la_dir->text());
        dirQ.mkdir("del");
        QDir dirD=dirQ;
        dirD.cd("del");

        dirQ.mkdir(m_ext1);
        QDir dirJ=dirQ;
        dirJ.cd(m_ext1);

        QDir dirN;
        bool auchNef=false;
        QString nefExt="";
        if(!m_ext2.isEmpty())
        {
            nefExt=m_ext2;
            dirQ.mkdir(nefExt);
            dirN=dirQ;
            dirN.cd(nefExt);
            auchNef=true;
        }

        //löschen (bzw in Dir del verschieben)
        int delS=ui->sb_delStart->value()-1,delE=ui->sb_delEnde->value()-1;
        if(delS>-1 && delS<m_fNamen.size() && delS < (m_anzahl-1))
        {
            if(delE>=m_fNamen.size())
                delE=m_fNamen.size()-1;
            for(int del=delS;del<=delE;del++)
            {
                QString baseN=schiebe(dirQ,dirD,m_fNamen[delS],m_fNamen[delS]); // ja immer delS, da es aus der Liste gelöscht wird
                if(auchNef)
                {
                    QString fnameNef=baseN+"."+nefExt;
                    schiebe(dirQ,dirD,fnameNef,fnameNef);
                }
                m_fNamen.removeAt(delS);
            }
        }
        //umbenennen
        int anz=qMin(m_anzahl,m_fNamen.size());
        for(int nr=0; nr<anz;nr++)
        {
            QString baseN=schiebe(dirQ,dirJ,m_fNamen[0],mkFname(nr)); // ja immer 0, da es aus der Liste gelöscht wird
            if(auchNef)
            {
                QString fnameNef=baseN+"."+nefExt;
                schiebe(dirQ,dirN,fnameNef,mkFname(nr,true)); // ja immer 0, da es aus der Liste gelöscht wird
            }
            m_fNamen.removeAt(0);
        }
        if(anz>1)
            ui->sb_schiene->setValue((ui->sb_schiene->value()+1));
        else
            ui->sb_start->setValue((ui->sb_start->value()+1));
        akt_2do();
    }
}

void dias_rename::on_pushButton_clicked()
{
    akt_fNamen();
}

void dias_rename::on_pushButton_3_clicked()
{
    QSettings settings("Dimitri-Junker.de","PostDiaDigit");
    settings.setValue("Prim_Ext",m_ext1);
    settings.setValue("Sec_Ext",m_ext2);
    settings.setValue("Del_1st",ui->sb_delStart->value());
    settings.setValue("Del_lst",ui->sb_delEnde->value());
    settings.setValue("Size",m_anzahl);
    settings.setValue("lang",tr("de"));

}

void dias_rename::on_rb_60_clicked()
{
    m_anzahl=60;
    akt_2do();

}



void dias_rename::on_pb_Abbruch_clicked()
{
    reject();
}
void dias_rename::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_F1)
    {
        QString url="file:./Docu/"+tr("liesmich.pdf");
        QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

    }
}
