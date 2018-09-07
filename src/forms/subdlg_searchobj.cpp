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

ScriptSearch::SearchData SubDlg_SearchObj::getSearchData()
{
    ScriptSearch::SearchData ret;

    if (ui->radioButton_id->isChecked())
        ret.searchBy = ScriptSearch::SearchBy::ID;
    else if (ui->radioButton_defname->isChecked())
        ret.searchBy = ScriptSearch::SearchBy::Defname;
    else //if (ui->radioButton_description->isChecked())
        ret.searchBy = ScriptSearch::SearchBy::Description;

    ret.caseSensitive = ui->checkBox_casesensitive->isChecked();
    ret.key = ui->lineEdit_value->text().toStdString();

    return ret;
}

void SubDlg_SearchObj::setSearchData(ScriptSearch::SearchData data)
{
    ui->checkBox_casesensitive->setChecked(data.caseSensitive);
    ui->lineEdit_value->setText(data.key.c_str());

    switch (data.searchBy)
    {
        case ScriptSearch::SearchBy::ID:        ui->radioButton_id->setChecked(true);           break;
        case ScriptSearch::SearchBy::Defname:   ui->radioButton_defname->setChecked(true);      break;
        //case ScriptSearch::SearchBy::Description:
        default:                                ui->radioButton_description->setChecked(true);  break;
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
