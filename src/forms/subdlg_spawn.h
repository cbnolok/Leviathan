#ifndef SUBDLG_SPAWN_H
#define SUBDLG_SPAWN_H

#include <QDialog>

class ScriptObj;


namespace Ui {
class SubDlg_Spawn;
}

class SubDlg_Spawn : public QDialog
{
    Q_OBJECT

public:
    explicit SubDlg_Spawn(QWidget *parent = nullptr);
    ~SubDlg_Spawn();

private slots:
    void on_pushButton_place_clicked();
    void on_pushButton_init_clicked();
    void on_pushButton_customCmd_clicked();
    void on_checkBox_top_toggled(bool checked);

    void onCust_selectedObj_changed(ScriptObj* obj);

private:
    Ui::SubDlg_Spawn *ui;

    const ScriptObj* m_selectedScriptObj;
};

#endif // SUBDLG_SPAWN_H
