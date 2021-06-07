#include "cpddcopy.h"
#include "ui_cpddcopy.h"
#include <QDesktopServices>
#include <QUrl>
/*
* 7.8.16 Dimitri Junker (DYJ) gekennzeichnet mit 'DYJ DYJTrack 2.04a'
    Copyright Dialog hinzugefüggt
*/
CPDDCopy::CPDDCopy(QString &vers,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CPDDCopy)
{
    ui->setupUi(this);
    ui->la_vers->setText(vers);
}

CPDDCopy::~CPDDCopy()
{
    delete ui;
}

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CPDDCopy


/////////////////////////////////////////////////////////////////////////////


void CPDDCopy::changeEvent(QEvent *e)
{
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
void CPDDCopy::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_F1)
    {
        QString url="file:./Docu/"+tr("liesmich.pdf");
        QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

    }
}

void CPDDCopy::on_pb_dyj_clicked()
{
    QDesktopServices::openUrl(QUrl("http://www.dimitri-junker.de/html/software__pc.html",QUrl::TolerantMode));

}

void CPDDCopy::on_pb_gpl_clicked()
{
    //:put here the abreviation of the new Language (for ex. en for english, fr for frensh, es for spanish,...
    QString sprache=tr("de"),url;
    if(sprache=="de")	//deutsch
        url="http://www.gnu.de/documents/gpl.de.html";
    else if(sprache=="fr")//französisch
        url="http://www.april.org/sites/default/files/groupes/trad-gpl/doc/GPLv3/www.rodage.org/gpl-3.0.fr.html?q=groupes/trad-gpl/doc/GPLv3/www.rodage.org/gpl-3.0.fr.html";
    else if(sprache=="en") //englisch
        url="http://www.gnu.org/licenses/gpl.html";
    else
        url="http://www.gnu.org/licenses/translations.html";
    QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

}
