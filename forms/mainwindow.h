#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


// i don't need the whole structure, because i use only a pointer to the class instance.
class MainTab_Items;
class MainTab_Chars;
class Dlg_ProfileScripts_Options;

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
    void on_actionOptions_triggered();
    void on_actionLoadDefaultProfile_triggered();

private:
    Ui::MainWindow      *ui;
    MainTab_Items       *m_MainTab_Items_inst        = nullptr;
    MainTab_Chars       *m_MainTab_Chars_inst        = nullptr;
    Dlg_ProfileScripts_Options    *m_Profile_Options_inst     = nullptr;
};


#endif // MAINWINDOW_H
