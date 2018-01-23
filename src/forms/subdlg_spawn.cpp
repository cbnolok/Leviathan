#include "subdlg_spawn.h"
#include "ui_subdlg_spawn.h"
#include "../spherescript/scriptobjects.h"
#include "../keystrokesender/keystrokesender.h"
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

    ks::KeystrokeSender::sendStringFastAsync(".add 01ea7", true, g_sendKeystrokeAndFocusClient);
}

void SubDlg_Spawn::on_pushButton_init_clicked()
{
    if (!m_selectedScriptObj)
        return;

    std::string attr_cmd = ".act.attr 0b0";
    std::string defname_cmd = ".act.more "  + m_selectedScriptObj->m_defname;

    //std::string maxDist_cmd = ".act.morez " + ui->lineEdit_maxDist->text().toStdString();
    //std::string minTime_cmd = ".act.morex " + ui->lineEdit_minTime->text().toStdString();
    //std::string maxTime_cmd = ".act.morey " + ui->lineEdit_maxTime->text().toStdString();
    std::string morep_cmd = ".act.morep " + ui->lineEdit_minTime->text().toStdString() + "," +
            ui->lineEdit_maxTime->text().toStdString() + "," + ui->lineEdit_maxDist->text().toStdString();

    std::vector<std::string> stringsToSend = { attr_cmd, defname_cmd, morep_cmd };
    if (ui->lineEdit_amount->text().toInt())
    {
        std::string amount_cmd  = ".act.amount "+ ui->lineEdit_amount->text().toStdString();
        stringsToSend.push_back(amount_cmd.c_str());
    }
    stringsToSend.emplace_back(".act.timer 1");

    ks::KeystrokeSender::sendStringsFastAsync(stringsToSend, true, g_sendKeystrokeAndFocusClient);
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
    ks::KeystrokeSender::sendStringFastAsync(command.toStdString(), true, g_sendKeystrokeAndFocusClient);
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
