#ifndef SUBDLG_SEARCHOBJ_H
#define SUBDLG_SEARCHOBJ_H

#include "../spherescript/scriptsearch.h"
#include <QDialog>


namespace Ui {
class SubDlg_SearchObj;
}

class SubDlg_SearchObj : public QDialog
{
    Q_OBJECT

public:
    explicit SubDlg_SearchObj(QWidget *parent = 0);
    ~SubDlg_SearchObj();
    void getSearchData(ScriptSearch::SearchBy_t &searchBy, bool &caseSensitive, std::string &key);

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();

private:
    Ui::SubDlg_SearchObj *ui;
};

#endif // SUBDLG_SEARCHOBJ_H
