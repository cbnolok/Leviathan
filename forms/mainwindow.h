#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


// i don't need the whole structure, because i use only a pointer to the class instance.
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
    void onCustom_actionEditScriptsProfiles_triggered();
    void onCustom_actionLoadDefaultScriptsProfile_triggered();
    void onCustom_actionEditClientProfiles_triggered();
    void onCustom_actionLoadDefaultClientProfile_triggered();
    void onCustom_actionSettings_triggered();
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
