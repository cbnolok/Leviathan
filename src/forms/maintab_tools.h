#ifndef MAINTAB_Tools_H
#define MAINTAB_Tools_H

#include <QWidget>

class Dlg_HuePicker;
class Dlg_WorldMap;

namespace Ui {
class MainTab_Tools;
}

class MainTab_Tools : public QWidget
{
    Q_OBJECT

public:
    explicit MainTab_Tools(QWidget *parent = nullptr);
    ~MainTab_Tools();

private slots:
    void on_pushButton_huePicker_clicked();
    void on_pushButton_worldMap_clicked();

private:
    Ui::MainTab_Tools *ui;
    Dlg_HuePicker *m_dlg_huepicker;
    Dlg_WorldMap *m_dlg_worldmap;
};

#endif // MAINTAB_Tools_H
