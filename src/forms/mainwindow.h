#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFutureWatcher>

class SubDlg_TaskProgress;
class MainTab_Items;
class MainTab_Chars;
class MainTab_Tools;
class MainTab_Log;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // menubar actions
    void onManual_actionEditScriptsProfiles_triggered();
    void onManual_actionLoadDefaultScriptsProfile_triggered();
    void onManual_actionEditClientProfiles_triggered();
    void onManual_actionLoadDefaultClientProfile_triggered();
    void onManual_actionSettings_triggered();
    void onManual_actionLoadClientProfile_mapped(int index);
    void onManual_actionLoadScriptsProfile_mapped(int index);
    // ui slots
    void on_tabWidget_currentChanged(int /* UNUSED: index */);
    void on_checkBox_onTop_toggled(bool checked);
    void on_checkBox_focus_toggled(bool checked);
    // other slots
    void loadDefaultProfiles_Async();
    void loadTaskDone();

public:
    int getDefaultClientProfile();
    int getDefaultScriptsProfile();
    void loadClientProfile_Async(int index);
    void loadScriptProfile_Async(int index);

private:
    void setupMenuBar();
    void loadDefaultProfiles_helper();
    void loadClientProfile_helper(int index);
    void loadScriptProfile_helper(int index);

    Ui::MainWindow      *ui;
    MainTab_Items       *m_MainTab_Items_inst;
    MainTab_Chars       *m_MainTab_Chars_inst;
    MainTab_Tools       *m_MainTab_Tools_inst;
    MainTab_Log         *m_MainTab_Log_inst;

    SubDlg_TaskProgress *m_loadProgressDlg;
    QFutureWatcher<void> m_futureWatcher;
    QFuture<void>        m_futureTask;
};


#endif // MAINWINDOW_H
