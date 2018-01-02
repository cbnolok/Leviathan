#include "subdlg_searchobj.h"
#include "ui_subdlg_searchobj.h"
#include <QTimer>


SubDlg_SearchObj::SubDlg_SearchObj(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubDlg_SearchObj)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);
    ui->radioButton_description->setChecked(true);  // default
    QTimer::singleShot(50, ui->lineEdit_value, SLOT(setFocus()));
}

ScriptSearch::SearchData_t SubDlg_SearchObj::getSearchData()
{
    ScriptSearch::SearchData_t ret;

    using s_t = ScriptSearch::SearchBy_t;
    if (ui->radioButton_id->isChecked())
        ret.searchBy = s_t::ID;
    else if (ui->radioButton_defname->isChecked())
        ret.searchBy = s_t::Defname;
    else //if (ui->radioButton_description->isChecked())
        ret.searchBy = s_t::Description;

    ret.caseSensitive = ui->checkBox_casesensitive->isChecked();
    ret.key = ui->lineEdit_value->text().toStdString();

    ret.initialized = true;
    return ret;
}

void SubDlg_SearchObj::setSearchData(ScriptSearch::SearchData_t data)
{
    if (!data.initialized)
        return;

    ui->checkBox_casesensitive->setChecked(data.caseSensitive);
    ui->lineEdit_value->setText(data.key.c_str());

    using s_t = ScriptSearch::SearchBy_t;
    switch (data.searchBy)
    {
        case s_t::ID:           ui->radioButton_id->setChecked(true);           break;
        case s_t::Defname:      ui->radioButton_defname->setChecked(true);      break;
        //case s_t::Description:
        default:                ui->radioButton_description->setChecked(true);  break;
    }
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
