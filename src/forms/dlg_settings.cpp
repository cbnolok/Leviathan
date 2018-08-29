#include "dlg_settings.h"
#include "ui_dlg_settings.h"

#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>

#include "../globals.h"
#include "../settings/appsettings.h"


Dlg_Settings::Dlg_Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_Settings)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);   // disable the '?' (what's this) in the title bar

    ui->setupUi(this);

    ui->checkBox_loadDefaultProfilesAtStartup->setChecked(g_settings.m_loadDefaultProfilesAtStartup);
    ui->lineEdit_spawn->setText( QString::fromStdString(g_settings.m_customSpawnCmd) );
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

    // Open the file in which we store the Settings.
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
