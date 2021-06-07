#ifndef POSTDIADIGIT_H
#define POSTDIADIGIT_H

#include <QMainWindow>
#include <QTranslator>
#include <QKeyEvent>

namespace Ui {
class PostDiaDigit;
}

class PostDiaDigit : public QMainWindow
{
    Q_OBJECT

public:
    explicit PostDiaDigit(QWidget *parent = nullptr);
    ~PostDiaDigit();
    QTranslator *m_language;
    void changeEvent(QEvent *e);
    QString m_progName;

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void on_cb_lang_currentTextChanged(const QString &lang);

    void on_actionUmbenennen_triggered();

    void on_actionSchneiden_triggered();

    void on_actionDubletten_triggered();

    void on_actionRahmen_triggered();

    void on_actionInfo_triggered();

    void on_actionHilfe_triggered();

private:
    Ui::PostDiaDigit *ui;
};

#endif // POSTDIADIGIT_H
