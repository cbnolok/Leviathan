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
    explicit Dlg_Settings(QWidget *parent = 0);
    ~Dlg_Settings();

private slots:
    void on_checkBox_loadDefaultProfilesAtStartup_stateChanged(int /* arg1 UNUSED */);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::Dlg_Settings *ui;

    bool m_loadDefaultProfilesAtStartup;
};

#endif // DLG_SETTINGS_H
