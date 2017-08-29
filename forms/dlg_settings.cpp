#include "dlg_settings.h"
#include "ui_dlg_settings.h"
#include "../globals.h"
#include "../settings/settings.h"
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>


Dlg_Settings::Dlg_Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_Settings)
{
    ui->setupUi(this);
    // dunno why, but this window cames with a '?' button in the title bar... i have to disable it manually
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->checkBox_loadDefaultProfilesAtStartup->setChecked(g_settings.m_loadDefaultProfilesAtStartup);
}

Dlg_Settings::~Dlg_Settings()
{
    delete ui;
}

void Dlg_Settings::on_checkBox_loadDefaultProfilesAtStartup_stateChanged(int /* arg1 UNUSED */)
{
    m_loadDefaultProfilesAtStartup = ui->checkBox_loadDefaultProfilesAtStartup->isChecked();
}

void Dlg_Settings::on_buttonBox_accepted()
{
    g_settings.m_loadDefaultProfilesAtStartup = m_loadDefaultProfilesAtStartup;

    // Open the file in which we store the Scripts Profiles.
    QFile jsonFile;
    jsonFile.setFileName("Settings.json");
    if (!jsonFile.open(QIODevice::WriteOnly))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText("Error: can't open Settings.json! Can't save the settings!");
        errorDlg.exec();
        return;
    }

    // Convert the Settings data to the Json format.
    QJsonObject mainJsonField;
    mainJsonField["Settings"] = g_settings.generateJsonObject();

    // Retrieve the formatted json data and write the file.
    QByteArray textData = QJsonDocument(mainJsonField).toJson(QJsonDocument::Indented);
    jsonFile.write(textData);
    jsonFile.close();
}

void Dlg_Settings::on_buttonBox_rejected()
{
    close();
}
