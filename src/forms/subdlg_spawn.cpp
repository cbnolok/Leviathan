#include "subdlg_spawn.h"
#include "ui_subdlg_spawn.h"
#include "../spherescript/scriptobjects.h"
#include "../globals.h"


SubDlg_Spawn::SubDlg_Spawn(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SubDlg_Spawn)
{
    m_selectedScriptObj = nullptr;
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->setupUi(this);
}

SubDlg_Spawn::~SubDlg_Spawn()
{
    delete ui;
}

void SubDlg_Spawn::on_pushButton_place_clicked()
{
    if (!m_selectedScriptObj)
        return;

    g_keystrokeSender.sendString(".add 01ea7");
}

void SubDlg_Spawn::on_pushButton_init_clicked()
{
    if (!m_selectedScriptObj)
        return;

    std::string defname_cmd = ".act.more "  + m_selectedScriptObj->m_defname;

    std::string amount_cmd  = ".act.amount "+ ui->lineEdit_amount->text().toStdString();

    //std::string maxDist_cmd = ".act.morez " + ui->lineEdit_maxDist->text().toStdString();
    //std::string minTime_cmd = ".act.morex " + ui->lineEdit_minTime->text().toStdString();
    //std::string maxTime_cmd = ".act.morey " + ui->lineEdit_maxTime->text().toStdString();
    std::string morep_cmd = ".act.morep " + ui->lineEdit_minTime->text().toStdString() + "," +
            ui->lineEdit_maxTime->text().toStdString() + "," + ui->lineEdit_maxDist->text().toStdString();

    g_keystrokeSender.sendString(".act.attr 0b0");

    g_keystrokeSender.sendString(defname_cmd.c_str());

    if (ui->lineEdit_amount->text().toInt())
        g_keystrokeSender.sendString(amount_cmd.c_str());

    //g_keystrokeSender.sendString(maxDist_cmd.c_str());
    //g_keystrokeSender.sendString(minTime_cmd.c_str());
    //g_keystrokeSender.sendString(maxTime_cmd.c_str());
    g_keystrokeSender.sendString(morep_cmd.c_str());

    g_keystrokeSender.sendString(".act.timer 1");
}

void SubDlg_Spawn::on_pushButton_customCmd_clicked()
{
    if (!m_selectedScriptObj)
        return;

    QString objDefname = QString::fromStdString(m_selectedScriptObj->m_defname);
    QString amount  = ui->lineEdit_amount->text();
    QString maxDist = ui->lineEdit_maxDist->text();
    QString minTime = ui->lineEdit_minTime->text();
    QString maxTime = ui->lineEdit_maxTime->text();

    QString command = QString::fromStdString(g_settings.m_customSpawnCmd).arg(objDefname, amount, maxDist, minTime, maxTime);
    g_keystrokeSender.sendString(command.toStdString().c_str());
}

void SubDlg_Spawn::on_checkBox_top_toggled(bool checked)
{
    if (checked)
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    else
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);

    // From Qt Wiki:
    // Note: This function calls setParent() when changing the flags for a window, causing the widget
    // to be hidden. You must call show() to make the widget visible again..
    show();
}

void SubDlg_Spawn::onCust_selectedObj_changed(ScriptObj* obj)
{
    m_selectedScriptObj = obj;
    ui->label_objDesc->setText(obj->m_description.c_str());
}
