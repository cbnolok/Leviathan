#ifndef DLG_SETTINGS_H
#define DLG_SETTINGS_H

#include <QDialog>


namespace Ui {
class Dlg_Settings;
}

class Dlg_Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_Settings(QWidget *parent = nullptr);
    ~Dlg_Settings();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();    

private:
    Ui::Dlg_Settings *ui;

    bool m_loadDefaultProfilesAtStartup;
    bool m_caseSensitiveScriptParsing;
};

#endif // DLG_SETTINGS_H
