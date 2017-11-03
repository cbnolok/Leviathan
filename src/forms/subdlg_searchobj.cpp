#include "subdlg_searchobj.h"
#include "ui_subdlg_searchobj.h"

SubDlg_SearchObj::SubDlg_SearchObj(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubDlg_SearchObj)
{
    ui->setupUi(this);
}

void SubDlg_SearchObj::getSearchData(ScriptSearch::SearchBy_t &searchBy, bool &caseSensitive, std::string &key)
{
    using s_t = ScriptSearch::SearchBy_t;
    if (ui->radioButton_id->isChecked())
        searchBy = s_t::ID;
    else if (ui->radioButton_defname->isChecked())
        searchBy = s_t::Defname;
    else //if (ui->radioButton_description->isChecked())
        searchBy = s_t::Description;
    caseSensitive = ui->checkBox_casesensitive->isChecked();
    key = ui->lineEdit_value->text().toStdString();
}

SubDlg_SearchObj::~SubDlg_SearchObj()
{
    delete ui;
}

void SubDlg_SearchObj::on_pushButton_ok_clicked()
{
    QDialog::done(QDialog::Accepted);
}

void SubDlg_SearchObj::on_pushButton_cancel_clicked()
{
    QDialog::done(QDialog::Rejected);
}
