#include "postdiadigit.h"
#include "ui_postdiadigit.h"
#include <dias_rename.h>
#include <DiasCut.h>
#include <DiaDubl.h>
#include <rahmen.h>
#include <qsettings.h>
#include <cpddcopy.h>
#include <QDesktopServices>
#include <QUrl>

PostDiaDigit::PostDiaDigit(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PostDiaDigit)
{
    ui->setupUi(this);
    QSettings settings("Dimitri-Junker.de","PostDiaDigit");
    QString tmp=settings.value("Prim_Ext").toString();
    if(!tmp.isEmpty())
    {
        ui->le_ext1->setText(tmp);
        ui->le_ext2->setText(settings.value("Sec_Ext").toString());
        QString lang=settings.value("lang").toString();
        if(lang.isEmpty())
            lang=QLocale::system().name().left(2);
        if(lang=="de")
            qApp->removeTranslator(m_language);
        else
        {
            m_language->load("dias_rename_"+lang);
            qApp->installTranslator(m_language);
        }
    }
    m_progName=this->windowTitle();

}



PostDiaDigit::~PostDiaDigit()
{
    delete ui;
}

void PostDiaDigit::on_cb_lang_currentTextChanged(const QString &lang)
{
    if(lang=="Deutsch")
         qApp->removeTranslator(m_language);
     else if(lang=="English")
     {
         m_language->load("dias_en");
         qApp->installTranslator(m_language);
     }
     else if(lang=="Français")
     {
         m_language->load("dias_fr");
         qApp->installTranslator(m_language);
     }

}
void PostDiaDigit::changeEvent(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        //:name of the language
        ui->cb_lang->setCurrentText(tr("Deutsch"));
        break;
    default:
        break;
    }
}

void PostDiaDigit::on_actionUmbenennen_triggered()
{
    dias_rename dr;
    dr.setExt(ui->le_ext1->text(),ui->le_ext2->text());

    dr.exec();
}

void PostDiaDigit::on_actionSchneiden_triggered()
{
    DiasCut dc(ui->le_ext1->text());
    dc.exec();
}

void PostDiaDigit::on_actionDubletten_triggered()
{
    DiaDubl dd(ui->le_ext1->text());
    dd.exec();
}

void PostDiaDigit::on_actionRahmen_triggered()
{
    Rahmen dr;
    dr.exec();

}
/*** ------------------------------------------------------------------------ ***/
/*	PostDiaDigit::on_actionHilfe_triggered()                      			*/
/*	Zeigt Hilfefile an                                               	*/
/*	Parameter:	keine															*/
/*	Ergebnis:	keines															*/
/*	29.5.21 von Dimitri Junker                                                   */
/*** ------------------------------------------------------------------------ ***/

void PostDiaDigit::on_actionHilfe_triggered()
{
    QString url="file:./Docu/"+tr("liesmich.pdf");
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}
/*** ------------------------------------------------------------------------ ***/
/*	void PostDiaDigit::on_actionInfo_triggered()                        			*/
/*	Zeigt Copyright Dialog an                                               	*/
/*	Parameter:	keine															*/
/*	Ergebnis:	keines															*/
/*	29.5.21 von Dimitri Junker                                                   */
/*** ------------------------------------------------------------------------ ***/

void PostDiaDigit::on_actionInfo_triggered()
{
    QString vers=QString("%1 (%2)").arg(m_progName).arg(__DATE__);

    CPDDCopy to(vers);
    to.exec();

}

void PostDiaDigit::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_F1)
    {
        QString url="file:./Docu/"+tr("liesmich.pdf");
        QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

    }
}
