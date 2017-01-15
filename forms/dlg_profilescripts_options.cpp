#include "dlg_profilescripts_options.h"
#include "ui_dlg_profilescripts_options.h"
#include "../common.h"
#include "../globals.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDirModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include "../qtutils/checkableproxymodel.h"
#include "../qtutils/modelutils.h"

#include <qDebug>


Dlg_ProfileScripts_Options::Dlg_ProfileScripts_Options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_ProfileScripts_Options)
{
    ui->setupUi(this);

    /* Profiles View */
    m_profiles_model = new QStandardItemModel();
    ui->listView_profiles->setModel(m_profiles_model);

    /* Scripts View */
    // Since Qt doesn't give us a model which both shows a directory tree with subdirectories and files and a
    //  checkbox for them, i used a proxy model (CheckableProxyModel, by Andre Somers) which adds the checkbox
    //  to the source model (QDirModel).

    // Set up the QDirModel, which will be the source model for the CheckableProxyModel.
    // I use QDirModel (outdated) instead of QFileSystemModel because the latter is slow and gives some syncronization
    //  problems (with fetchMore())
    m_scripts_model_base = new QDirModel();
    m_scripts_model_base->setSorting(QDir::DirsFirst | QDir::Name);
    m_scripts_model_base->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    QStringList extensions;
    extensions << "*.scp";
    m_scripts_model_base->setNameFilters(extensions);   // filter out unneeded files (grays them)
    m_scripts_model_base->setReadOnly(true);            // can't do actions like rename, copy, cut, etc.

    // Set up the CheckableProxyModel.
    m_scripts_model = new CheckableProxyModel();
    //m_scripts_model->setDefaultCheckState(false);
    m_scripts_model->setSourceModel(m_scripts_model_base);
    ui->treeView_scripts->setModel(m_scripts_model);

    ui->treeView_scripts->hideColumn(1);        // hide columns showing size, file type, etc.
    ui->treeView_scripts->hideColumn(2);
    ui->treeView_scripts->hideColumn(3);
    ui->treeView_scripts->header()->setSectionResizeMode(0, QHeaderView::Stretch);  // stretch the column to fit the view width.

    // connect a checkbox to control the default state of the checkboxes
    //connect(ui->chkSetDefaultChecked, SIGNAL (toggled(bool)), m_checkProxy, SLOT (setDefaultCheckState(bool)));
    //m_scripts_model->setDefaultCheckState(ui->chkSetDefaultChecked->isChecked());

    // connect a reset button to reset the checkboxes
    //connect(ui->cmdReset, SIGNAL (clicked()), m_scripts_model, SLOT (resetToDefault()));

    // do something when the checked boxes changed
    //connect(m_scripts_model, SIGNAL (checkedNodesChanged()), this, SLOT (selectedItemsChanged()));

    ui->pushButton_profileDelete->setEnabled(false);
    ui->pushButton_profileSave->setEnabled(false);

    updateProfilesView();
}

Dlg_ProfileScripts_Options::~Dlg_ProfileScripts_Options()
{
    delete ui;

    delete m_scripts_model_base;
    delete m_scripts_model;
}

// TODO: fai che ogni volta che segni un profilo come default controlli se ci sono altri profili default e li togli
// TODO: fai funzionare i pulsanti de-select all e clear all

void Dlg_ProfileScripts_Options::updateProfilesView()
{
    // Empty the model (so, empty the list)
    m_profiles_model->removeRows(0, m_profiles_model->rowCount());

    // Load all the profiles.
    //g_scriptsProfiles = ScriptsProfile::readJsonData();
    for (auto it = g_scriptsProfiles.begin(); it != g_scriptsProfiles.end(); it++)
    {
        QStandardItem *newProfileItem = new QStandardItem(it->m_name.c_str());
        if ((*it).m_defaultProfile)        // set blue text color for the default profile
            newProfileItem->setForeground(QBrush(QColor("blue")));
        m_profiles_model->appendRow(newProfileItem);
    }
}

void Dlg_ProfileScripts_Options::updateScriptsView(QString path)
{
    // The QDirModel needs to have the Root Path, then the Tree View needs the Root Index (which is the shown directory of the model)
    QModelIndex rootIndexBase = m_scripts_model_base->index(path);
    // You can set the root index of the view only if the index is from the proxy model, you can't use an index of the source one,
    //  so you need to map the source index to a proxy model index, which you can then apply.
    QModelIndex rootIndex = m_scripts_model->mapFromSource(rootIndexBase);
    ui->treeView_scripts->setRootIndex(rootIndex);
}

bool Dlg_ProfileScripts_Options::checkScriptsFromProfile_loop(const std::string scriptFromProfile, const QModelIndex &proxyParent)
{
    for (int modelRow = 0; modelRow < m_scripts_model->rowCount(proxyParent); modelRow++)
    {
        const QModelIndex proxyIdx = m_scripts_model->index(modelRow, 0, proxyParent);
        if(!proxyIdx.isValid())
            continue;
        const QModelIndex sourceIdx = m_scripts_model->mapToSource(proxyIdx);

        if (m_scripts_model_base->isDir(sourceIdx))
        {
            if (checkScriptsFromProfile_loop(scriptFromProfile, proxyIdx) == true)
                return true;
        }

        // get the absolute path of the current file in the view
        //std::string curFileStr = idx.data(Qt::DisplayRole).toString().toStdString();    // filename
        std::string curViewFileStr = m_scripts_model_base->filePath(sourceIdx).toStdString();
        if ( !scriptFromProfile.compare(curViewFileStr) )
        {
            m_scripts_model->setData(proxyIdx, Qt::Checked, Qt::CheckStateRole);
            return true;
        }
    }
    return false;
}

void Dlg_ProfileScripts_Options::checkScriptsFromProfile(const ScriptsProfile *sp, const QModelIndex &proxyParent)
{
    // set the check state of the scripts in the treeview

    for (auto it = sp->m_scriptsToLoad.begin(); it != sp->m_scriptsToLoad.end(); it++)
    {
        if (!checkScriptsFromProfile_loop(*it, proxyParent))
            appendToLog("Profile \"" + sp->m_name + "\": File \"" + *it + "\" saved in the profile doesn't exist anymore!");
    }
}

void Dlg_ProfileScripts_Options::on_listView_profiles_clicked(const QModelIndex &index)
{
    /* // For multiple selections:
    QItemSelectionModel selection = ui->listView_profiles->selectionModel()->selectedIndexes();
    if(selection.indexes().isEmpty())
    {
        // empty the scripts tree view
        updateScriptsView("");
        return;
    }

    if (selection.indexes().first().row() > (int)g_scriptsProfiles.size())
        return;
    */

    m_currentProfileIndex = index.row();
    ScriptsProfile *sp = &g_scriptsProfiles[m_currentProfileIndex];

    m_scripts_model->resetToDefault();  // TODO: or another method?

    ui->lineEdit_editName->setText(QString(sp->m_name.c_str()));
    ui->checkBox_setDefaultProfile->setCheckState(sp->m_defaultProfile ? Qt::Checked : Qt::Unchecked);
    ui->lineEdit_editPath->setText(QString(sp->m_scriptsPath.c_str()));
    if (sp->m_useSpheretables)
        ui->checkBox_spheretables->setCheckState(Qt::Checked);
    else
    {
        updateScriptsView(QString(sp->m_scriptsPath.c_str()));
        checkScriptsFromProfile(sp, ui->treeView_scripts->rootIndex());
    }

    ui->pushButton_profileDelete->setDisabled(false);
    ui->pushButton_profileSave->setDisabled(false);
}

void Dlg_ProfileScripts_Options::on_lineEdit_editPath_textChanged(const QString &arg1)
{
    // Check if it's a valid path, if not, set the color of the text to red.
    if (!isValidDirectory(arg1.toStdString()))
    {
        ui->lineEdit_editPath->setStyleSheet("color: red");
        return;
    }

    ui->lineEdit_editPath->setStyleSheet("color: default");
    updateScriptsView(ui->lineEdit_editPath->text());
}

void Dlg_ProfileScripts_Options::on_pushButton_pathBrowse_clicked()
{
    QFileDialog *dlg = new QFileDialog();
    dlg->setOption(QFileDialog::ShowDirsOnly);
    dlg->setFileMode(QFileDialog::Directory);
    dlg->setAcceptMode(QFileDialog::AcceptOpen);
    dlg->setViewMode(QFileDialog::List);
    if (isValidDirectory(ui->lineEdit_editPath->text().toStdString()))
         dlg->setDirectory(ui->lineEdit_editPath->text());

    if (dlg->exec())
    {
        ui->lineEdit_editPath->setText(dlg->directory().absolutePath());
        updateScriptsView(ui->lineEdit_editPath->text());
    }

    delete dlg;
}

void Dlg_ProfileScripts_Options::saveProfilesToJson()
{
    // Open the file in which we store the Scripts Profiles.
    QFile jsonFile;
    jsonFile.setFileName("ScriptsProfiles.json");
    if (!jsonFile.open(QIODevice::WriteOnly))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText("Error: can't open ScriptsProfiles.json! Can't save the profiles!");
        errorDlg.exec();
        return;
    }

    // Parse back all profiles (plus the new one) in json format.
    QJsonObject mainJsonField;
    QJsonObject scriptsProfileJsonField;
    for (size_t i = 0; i < g_scriptsProfiles.size(); i++)
        scriptsProfileJsonField[QString::number((long)i)] = g_scriptsProfiles[i].generateJsonObject();
    mainJsonField["ScriptsProfiles"] = scriptsProfileJsonField;

    // Retrieve the formatted json data and write the file.
    QByteArray textData = QJsonDocument(mainJsonField).toJson(QJsonDocument::Indented);
    jsonFile.write(textData);
    jsonFile.close();
}

void Dlg_ProfileScripts_Options::on_pushButton_profileAdd_clicked()
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
    ScriptsProfile newProfile(ui->lineEdit_editPath->text().toStdString());
    if (! ui->lineEdit_editName->text().isEmpty() )
        newProfile.m_name = ui->lineEdit_editName->text().toStdString();
    if (ui->checkBox_setDefaultProfile->checkState() == Qt::Checked)
    {
        newProfile.m_defaultProfile = true;
        for (size_t i = 0; i < g_scriptsProfiles.size(); i++)
        {
            ScriptsProfile *spTemp = &g_scriptsProfiles[i];
            spTemp->m_defaultProfile = false;
        }
    }

    // Populate the scripts file list in the profile.
    //fetchSyncCheckableProxyModelSourcedQFileSystemModelCheckedDirRecursive(m_scripts_model, ui->treeView_scripts->rootIndex());
    QStringList selectedScripts = ModelUtils::extractPathsFromCheckableProxyModelSourcedQDirModel(m_scripts_model, ui->treeView_scripts->rootIndex());
    for (int i = 0; i < selectedScripts.count(); i++)
        newProfile.m_scriptsToLoad.push_back(selectedScripts.at(i).toStdString());
    g_scriptsProfiles.push_back(newProfile);

    // Add item in the profiles list.
    QStandardItem *newProfileItem = new QStandardItem(newProfile.m_name.c_str());
    if (newProfile.m_defaultProfile)
    {
        newProfileItem->setForeground(QBrush(QColor("blue")));  // set blue text color for the default profile
        for (int i = 0; i < m_profiles_model->rowCount(); i++ )
        {
            QModelIndex iIndex = m_profiles_model->index(i,0);
            QStandardItem *iItem = m_profiles_model->itemFromIndex(iIndex);
            iItem->setForeground(QBrush(QColor("black")));    // set black color for non default profiles
        }
    }
    m_profiles_model->appendRow(newProfileItem);
    // Since the ScriptsProfile instance and the newProfileItem are added sequentially, the index of ScriptsProfile instance
    //  inside the g_scriptsProfiles vector corresponds to the QStandardItem row.

    saveProfilesToJson();
}

void Dlg_ProfileScripts_Options::on_pushButton_profileDelete_clicked()
{
    if (m_currentProfileIndex == -1)
        return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm action", "Are you sure to delete the current profile?",
                                    QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::No)
        return;

    m_profiles_model->removeRow(m_currentProfileIndex);
    g_scriptsProfiles.erase(g_scriptsProfiles.begin() + m_currentProfileIndex);
    saveProfilesToJson();

    ui->pushButton_profileDelete->setDisabled(true);
    ui->pushButton_profileSave->setDisabled(true);
    m_currentProfileIndex = -1;
}

void Dlg_ProfileScripts_Options::on_pushButton_profileSave_clicked()
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
    ScriptsProfile *sp = &g_scriptsProfiles[m_currentProfileIndex];
    sp->m_name = ui->lineEdit_editName->text().toStdString();
    sp->m_scriptsPath = ui->lineEdit_editPath->text().toStdString();

    sp->m_defaultProfile = (ui->checkBox_setDefaultProfile->checkState() == Qt::Unchecked) ? false: true;
    if (sp->m_defaultProfile)
    {
        for (size_t i = 0; i < g_scriptsProfiles.size(); i++)
        {
            QModelIndex iIndex = m_profiles_model->index(i,0);
            QStandardItem *iItem = m_profiles_model->itemFromIndex(iIndex);

            if ((int)i != m_currentProfileIndex)
            {
                ScriptsProfile *spTemp = &g_scriptsProfiles[i];
                spTemp->m_defaultProfile = false;
                iItem->setForeground(QBrush(QColor("black")));    // set black color for non default profiles
            }
            else
                iItem->setForeground(QBrush(QColor("blue")));    // set blue color for the default profile
        }
    }

    sp->m_useSpheretables = (ui->checkBox_spheretables->checkState() == Qt::Unchecked) ? false: true;
    if (!sp->m_useSpheretables)
    {
        QStringList selectedScripts = ModelUtils::extractPathsFromCheckableProxyModelSourcedQDirModel(m_scripts_model, ui->treeView_scripts->rootIndex());
        sp->m_scriptsToLoad.clear();
        for (int i = 0; i < selectedScripts.count(); i++)
            sp->m_scriptsToLoad.push_back(selectedScripts.at(i).toStdString());
    }

    saveProfilesToJson();
}


void Dlg_ProfileScripts_Options::on_pushButton_SelectAllScripts_clicked()
{
    ModelUtils::resetCheckedStateCheckableProxyModel(m_scripts_model, true);
}

void Dlg_ProfileScripts_Options::on_pushButton_clearSelection_clicked()
{
    ModelUtils::resetCheckedStateCheckableProxyModel(m_scripts_model, false);
}
