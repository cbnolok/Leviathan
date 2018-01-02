#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainTab_Items;
class MainTab_Chars;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_tabWidget_currentChanged(int /* UNUSED: index */);
    void onManual_actionEditScriptsProfiles_triggered();
    void onManual_actionLoadDefaultScriptsProfile_triggered();
    void onManual_actionEditClientProfiles_triggered();
    void onManual_actionLoadDefaultClientProfile_triggered();
    void onManual_actionSettings_triggered();
    void onManual_actionLoadClientProfile_mapped(int index);
    void onManual_actionLoadScriptsProfile_mapped(int index);
    void on_checkBox_onTop_toggled(bool checked);
    void on_checkBox_focus_toggled(bool checked);
    void loadDefaultProfiles();

private:
    Ui::MainWindow      *ui;
    MainTab_Items       *m_MainTab_Items_inst        = nullptr;
    MainTab_Chars       *m_MainTab_Chars_inst        = nullptr;

    int getDefaultClientProfile();
    int getDefaultScriptsProfile();
    void loadClientProfile(int index);
    void loadScriptProfile(int index);
};


#endif // MAINWINDOW_H
