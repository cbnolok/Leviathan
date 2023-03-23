#include "dlg_profileclient_options.h"
#include "ui_dlg_profileclient_options.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

#include "../globals.h"
#include "../cpputils/sysio.h"


Dlg_ProfileClient_Options::Dlg_ProfileClient_Options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_ProfileClient_Options)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);   // disable the '?' (what's this) in the title bar

    ui->setupUi(this);

    /* Profiles View */
    m_profiles_model = new QStandardItemModel();
    ui->listView_profiles->setModel(m_profiles_model);

    ui->pushButton_profileDelete->setEnabled(false);
    ui->pushButton_profileSave->setEnabled(false);

    updateProfilesView();
}

Dlg_ProfileClient_Options::~Dlg_ProfileClient_Options()
{
    delete m_profiles_model;

    delete ui;
}


// TODO: fai che ogni volta che segni un profilo come default controlli se ci sono altri profili default e li togli
// TODO: fai funzionare i pulsanti de-select all e clear all

void Dlg_ProfileClient_Options::updateProfilesView()
{
    // Empty the model (so, empty the list)
    m_profiles_model->removeRows(0, m_profiles_model->rowCount());

    // Load all the profiles.
    for (auto it = g_clientProfiles.begin(); it != g_clientProfiles.end(); ++it)
    {
        QStandardItem *newProfileItem = new QStandardItem(it->m_name.c_str());
        if (it->m_defaultProfile)        // set blue text color for the default profile
            newProfileItem->setForeground(QBrush(QColor("blue")));
        m_profiles_model->appendRow(newProfileItem);
    }
}


void Dlg_ProfileClient_Options::on_pushButton_pathBrowse_clicked()
{
    QFileDialog dlg;
    dlg.setOption(QFileDialog::ShowDirsOnly);
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setViewMode(QFileDialog::List);
    if (isValidDirectory(ui->lineEdit_editPath->text().toStdString()))
         dlg.setDirectory(ui->lineEdit_editPath->text());

    if (dlg.exec())
        ui->lineEdit_editPath->setText(dlg.directory().absolutePath());
}

void Dlg_ProfileClient_Options::on_lineEdit_editPath_textChanged(const QString &arg1)
{
    // Check if it's a valid path, if not, set the color of the text to red.
    if (!isValidDirectory(arg1.toStdString()))
    {
        ui->lineEdit_editPath->setStyleSheet("color: red");
        return;
    }

    ui->lineEdit_editPath->setStyleSheet("color: default");
}

void Dlg_ProfileClient_Options::on_pushButton_profileDelete_clicked()
{
    if (m_currentProfileIndex == -1)
        return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm action", "Are you sure you want to delete the selected profile?",
                                    QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::No)
        return;

    m_profiles_model->removeRow(m_currentProfileIndex);
    g_clientProfiles.erase(g_clientProfiles.begin() + m_currentProfileIndex);
    saveProfilesToJson();

    ui->pushButton_profileDelete->setDisabled(true);
    ui->pushButton_profileSave->setDisabled(true);
    m_currentProfileIndex = -1;
}

void Dlg_ProfileClient_Options::on_pushButton_profileAdd_clicked()
{
    QString errorMsg;
    if (ui->lineEdit_editName->text().isEmpty())
        errorMsg = "Invalid name!";
    else if (!isValidDirectory(ui->lineEdit_editPath->text().toStdString()))
        errorMsg = "Invalid path!";
    if (!errorMsg.isEmpty())
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(errorMsg);
        errorDlg.exec();
        return;
    }

    // Create a new profile.
    std::string profileDir(ui->lineEdit_editPath->text().toStdString());
    profileDir = standardizePath(profileDir);
    ClientProfile newProfile(profileDir);

    if (ui->checkBox_setDefaultProfile->checkState() == Qt::Checked)
    {
        newProfile.m_defaultProfile = true;
        for (size_t i = 0; i < g_clientProfiles.size(); ++i)
            g_clientProfiles[i].m_defaultProfile = false;
    }

    if (! ui->lineEdit_editName->text().isEmpty() )
        newProfile.m_name = ui->lineEdit_editName->text().toStdString();

    g_clientProfiles.emplace_back(std::move(newProfile));

    // Refresh the profiles list.
    updateProfilesView();

    // Since the ClientProfile instance and the newProfileItem are added sequentially, the index of ClientProfile instance
    //  inside the g_clientProfiles vector corresponds to the QStandardItem row.
    saveProfilesToJson();
}

void Dlg_ProfileClient_Options::on_pushButton_profileSave_clicked()
{
    if (m_currentProfileIndex == -1)
        return;

    QString errorMsg;
    if (ui->lineEdit_editName->text().isEmpty())
        errorMsg = "Invalid name!";
    else if (!isValidDirectory(ui->lineEdit_editPath->text().toStdString()))
        errorMsg = "Invalid path!";
    if (!errorMsg.isEmpty())
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(errorMsg);
        errorDlg.exec();
        return;
    }

    // Update the profile name shown on the list
    m_profiles_model->setData(m_profiles_model->index(m_currentProfileIndex,0), ui->lineEdit_editName->text(), Qt::DisplayRole);

    // Update profile data
    ClientProfile *cp = &g_clientProfiles[m_currentProfileIndex];
    cp->m_name = ui->lineEdit_editName->text().toStdString();

    cp->m_defaultProfile = (ui->checkBox_setDefaultProfile->checkState() == Qt::Unchecked) ? false: true;
    if (cp->m_defaultProfile)
    {
        for (int i = 0, sz = (int)g_clientProfiles.size(); i < sz; ++i)
        {
            if (i != m_currentProfileIndex)
                g_clientProfiles[i].m_defaultProfile = false;
        }
    }

    cp->m_clientPath = ui->lineEdit_editPath->text().toStdString();
    cp->m_clientPath = standardizePath(cp->m_clientPath);

    cp->m_clientWindowName = ui->lineEdit_editWindowTitle->text().toStdString();

    saveProfilesToJson();
}

void Dlg_ProfileClient_Options::on_listView_profiles_clicked(const QModelIndex &index)
{
    m_currentProfileIndex = index.row();
    ClientProfile *cp = &g_clientProfiles[m_currentProfileIndex];

    ui->lineEdit_editName->setText(QString::fromStdString(cp->m_name));
    ui->checkBox_setDefaultProfile->setCheckState(cp->m_defaultProfile ? Qt::Checked : Qt::Unchecked);
    ui->lineEdit_editPath->setText(QString::fromStdString(cp->m_clientPath));
    ui->lineEdit_editWindowTitle->setText(QString::fromStdString(cp->m_clientWindowName));

    ui->pushButton_profileDelete->setDisabled(false);
    ui->pushButton_profileSave->setDisabled(false);
}

void Dlg_ProfileClient_Options::saveProfilesToJson()
{
    // Open the file in which we store the Client Profiles.
    QFile jsonFile;
    jsonFile.setFileName("ClientProfiles.json");
    if (!jsonFile.open(QIODevice::WriteOnly))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText("Error: can't open ClientProfiles.json! Can't save the Client Profiles!");
        errorDlg.exec();
        return;
    }

    // Convert all profiles (plus the new one) in Json format.
    QJsonObject mainJsonField;
    QJsonObject clientProfileJsonField;
    for (size_t i = 0; i < g_clientProfiles.size(); ++i)
        clientProfileJsonField[QString::number((long)i)] = g_clientProfiles[i].generateJsonObject();
    mainJsonField["ClientProfiles"] = clientProfileJsonField;

    // Retrieve the formatted json data and write the file.
    QByteArray textData = QJsonDocument(mainJsonField).toJson(QJsonDocument::Indented);
    jsonFile.write(textData);
    jsonFile.close();
}


void Dlg_ProfileClient_Options::on_checkBox_setDefaultProfile_stateChanged(int arg1)
{
    if (arg1 != Qt::Checked)
        return;

    for (int idx = 0; ClientProfile &cp : g_clientProfiles)
    {
        if ((cp.m_defaultProfile) && (idx != m_currentProfileIndex))
            cp.m_defaultProfile = false;
        ++idx;
    }
}

