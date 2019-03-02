#ifndef SUBDLG_SEARCHOBJ_H
#define SUBDLG_SEARCHOBJ_H

#include <QDialog>
#include "../spherescript/scriptsearch.h"


namespace Ui {
class SubDlg_SearchObj;
}

class SubDlg_SearchObj : public QDialog
{
    Q_OBJECT

public:
    explicit SubDlg_SearchObj(QWidget *parent = nullptr);
    ~SubDlg_SearchObj();

    ScriptSearch::SearchData getSearchData();
    void setSearchData(ScriptSearch::SearchData data);


private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::SubDlg_SearchObj *ui;
};

#endif // SUBDLG_SEARCHOBJ_H
