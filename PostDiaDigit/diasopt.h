#ifndef DIASOPT_H
#define DIASOPT_H

#include <QDialog>
#include <qsettings.h>
#include <QTranslator>
#include <QKeyEvent>

namespace Ui {
class DiasOpt;
}

class DiasOpt : public QDialog
{
    Q_OBJECT

public:
    int m_maxBilder;
    int m_minLen;
    QTranslator *m_pLanguage;
    double m_minW;
    explicit DiasOpt(QWidget *parent = nullptr);
    ~DiasOpt();
    void changeEvent(QEvent *e);
    void initDialog();

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:

    void on_cb_lang_currentIndexChanged(const QString &arg1);

    void on_buttonBox_accepted();

private:
    Ui::DiasOpt *ui;
};

#endif // DIASOPT_H
