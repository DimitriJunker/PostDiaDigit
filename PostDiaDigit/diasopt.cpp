#include "diasopt.h"
#include "ui_diasopt.h"
#include <QDesktopServices>

DiasOpt::DiasOpt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiasOpt)
{
    ui->setupUi(this);
}

DiasOpt::~DiasOpt()
{
    delete ui;
}

void DiasOpt::changeEvent(QEvent *e)
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
void DiasOpt::initDialog()
{
    ui->le_maxBilder->setText(QString::number(m_maxBilder));
    ui->le_minW->setText(QString::number(m_minW));
    ui->le_minLen->setText(QString::number(m_minLen));

}
void DiasOpt::on_cb_lang_currentIndexChanged(const QString &lang)
{
    if(lang=="Deutsch")
        qApp->removeTranslator(m_pLanguage);
    else if(lang=="English")
    {
        m_pLanguage->load("dias_en");
        qApp->installTranslator(m_pLanguage);
    }
    else if(lang=="Français")
    {
        m_pLanguage->load("dias_fr");
        qApp->installTranslator(m_pLanguage);
    }
}

void DiasOpt::on_buttonBox_accepted()
{
    m_maxBilder=ui->le_maxBilder->text().toInt();
    m_minW=ui->le_minW->text().toDouble();
    m_minLen=ui->le_minLen->text().toInt();
    QSettings settings("Dimitri-Junker.de","PostDiaDigit");
    settings.setValue("NoS4M",m_maxBilder);
    settings.setValue("minArc",m_minW);
    settings.setValue("minLen",m_minLen);
    settings.setValue("lang",tr("de"));
}
void DiasOpt::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_F1)
    {
        QString url="file:./Docu/"+tr("liesmich.pdf");
        QDesktopServices::openUrl(QUrl(url,QUrl::TolerantMode));

    }
}
