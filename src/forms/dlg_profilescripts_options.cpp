#include "dlg_profilescripts_options.h"
#include "ui_dlg_profilescripts_options.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

#include "../globals.h"
#include "../qtutils/checkableproxymodel.h"
#include "../qtutils/modelutils.h"
#include "../cpputils/collections.h"
#include "../cpputils/sysio.h"


void Dlg_ProfileScripts_Options::setEnabledScriptsGroup(bool val)
{
    ui->lineEdit_editName->setEnabled(val);
    ui->lineEdit_editPath->setEnabled(val);
    ui->pushButton_pathBrowse->setEnabled(val);
    ui->checkBox_setDefaultProfile->setEnabled(val);
}


Dlg_ProfileScripts_Options::Dlg_ProfileScripts_Options(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_ProfileScripts_Options),
    m_currentProfileIndex(-1), m_scriptsProfileLoaded(nullptr)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);   // disable the '?' (what's this) in the title bar
    ui->setupUi(this);

    /* Profiles View */
    m_profiles_model = new QStandardItemModel(this);
    ui->listView_profiles->setModel(m_profiles_model);

    /* Scripts View */
    // Since Qt doesn't give us a model which both shows a directory tree with subdirectories and files and a
    //  checkbox for them, i used a proxy model (CheckableProxyModel, by Andre Somers) which adds the checkbox
    //  to the source model (QFileSystemModel).

    // Set up theQFileSystemModel, which will be the source model for the CheckableProxyModel.
    m_scripts_model_base = new QFileSystemModel(this);
    //m_scripts_model_base->setSorting(QDir::DirsFirst | QDir::Name);
    m_scripts_model_base->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    QStringList extensions("*.scp");
    m_scripts_model_base->setNameFilters(extensions);   // filter out unneeded files (grays them)
    m_scripts_model_base->setNameFilterDisables(false); // don't show them
    m_scripts_model_base->setReadOnly(true);            // can't do actions like rename, copy, cut, etc.

    // Set up the CheckableProxyModel.
    m_scripts_model_proxy = new CheckableProxyModel(this);
    //m_scripts_model_proxy->setDefaultCheckState(false);
    m_scripts_model_proxy->setSourceModel(m_scripts_model_base);
    //m_scripts_model_proxy->invalidate();
    ui->treeView_scripts->setModel(m_scripts_model_proxy);
    ui->treeView_scripts->setDisabled(true);

    // hide columns showing size, file type, etc.
    ui->treeView_scripts->hideColumn(1);
    ui->treeView_scripts->hideColumn(2);
    ui->treeView_scripts->hideColumn(3);
    // stretch the column to fit the view width.
    ui->treeView_scripts->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->pushButton_profileDelete->setEnabled(false);
    ui->pushButton_profileSave->setEnabled(false);

    connect(m_scripts_model_base, SIGNAL(directoryLoaded(QString)), this, SLOT(ms_scriptTree_directoryLoaded(QString)));

    updateProfilesView();
}


Dlg_ProfileScripts_Options::~Dlg_ProfileScripts_Options()
{
    delete m_profiles_model;
    delete m_scripts_model_proxy;
    delete m_scripts_model_base;

    delete ui;
}

// TODO: fai che ogni volta che segni un profilo come default controlli se ci sono altri profili default e li togli
// TODO: fai funzionare i pulsanti de-select all e clear all

void Dlg_ProfileScripts_Options::updateProfilesView()
{
    // Empty the model (so, empty the list)
    m_profiles_model->removeRows(0, m_profiles_model->rowCount());

    // Load all the profiles.
    //g_scriptsProfiles = ScriptsProfile::createFromJson();
    for (auto it = g_scriptsProfiles.begin(); it != g_scriptsProfiles.end(); ++it)
    {
        auto* newProfileItem = new QStandardItem(QString::fromStdString(it->m_name));
        if (it->m_defaultProfile)        // set blue text color for the default profile
            newProfileItem->setForeground(QBrush(QColor("blue")));
        m_profiles_model->appendRow(newProfileItem);
    }
}

void Dlg_ProfileScripts_Options::updateScriptsView(QString path)
{
    Q_ASSERT(m_scriptsViewHelper.viewStage == FSLoadingStage::Init);
    m_scriptsViewHelper.pendingDirectories = 1;
    m_scriptsViewHelper.viewStage = FSLoadingStage::ExpandingTreeStructure;

    m_scripts_model_base->setRootPath(path);    // This starts the first directory (the root path) loading, calling ms_scriptTree_directoryLoaded.

    // The QDirModel needs to have the Root Path, then the Tree View needs the Root Index (which is the shown directory of the model)
    const QModelIndex rootIndexBase = m_scripts_model_base->index(path);
    // You can set the root index of the view only if the index is from the proxy model, you can't use an index of the source one,
    //  so you need to map the source index to a proxy model index, which you can then apply.
    const QModelIndex rootIndex = m_scripts_model_proxy->mapFromSource(rootIndexBase);
    ui->treeView_scripts->setRootIndex(rootIndex);
    ui->treeView_scripts->setDisabled(false);
}


// ----


// ms = manual(ly connected) slot
void Dlg_ProfileScripts_Options::ms_scriptTree_directoryLoaded(QString element)
{
    m_scriptsViewHelper.pendingDirectories -= 1;
    Q_ASSERT(m_scriptsViewHelper.pendingDirectories >= 0);

    if ((m_scriptsViewHelper.pendingDirectories != 0) || (m_scriptsViewHelper.viewStage != FSLoadingStage::ExpandingTreeStructure))
        return;

    const bool modelChanged = (m_scripts_model_proxy != ui->treeView_scripts->rootIndex().model());
    Q_ASSERT(!modelChanged);

    // The argument "element" is a full path, not relative.
    standardizePath(element);
    const QModelIndex sourceDirIdx(m_scripts_model_base->index(element));
    if (!sourceDirIdx.isValid())
        return;

    QString modelRootPath(m_scripts_model_base->rootPath());
    standardizePath(modelRootPath);
    if (!modelRootPath.compare(element))
    {
        const auto rep = deleteElementReport(&m_scriptsViewHelper.dirsToExpand, element.toStdString());
        Q_ASSERT(rep.first);
    }

    const int nRows = m_scripts_model_base->rowCount(sourceDirIdx);
    if (nRows == 0)
        return;

    // Loop through all the rows on the same level (with the same parent).
    for (int iRow = 0; iRow < nRows; ++iRow)
    {
        const QModelIndex sourceChildIdx = m_scripts_model_base->index(iRow, 0, sourceDirIdx);
        if (!sourceChildIdx.isValid() || !m_scripts_model_base->isDir(sourceChildIdx))
            continue;

        QString childFullPath(m_scripts_model_base->filePath(sourceChildIdx));
        standardizePath(childFullPath);
        const auto rep = deleteElementReport(&m_scriptsViewHelper.dirsToExpand, childFullPath.toStdString());
        if (!rep.first)
            continue;

        const QModelIndex proxyChildIdx = m_scripts_model_proxy->mapFromSource(sourceChildIdx);
        Q_ASSERT(proxyChildIdx.isValid());

        m_scriptsViewHelper.pendingDirectories += 1;
        ui->treeView_scripts->expand(proxyChildIdx);
    }

    // Am I done?
    if (m_scriptsViewHelper.pendingDirectories == 0)
    {
        // Done.
        // Reporting missing folders.
        if (!m_scriptsViewHelper.dirsToExpand.empty())
        {
            appendToLog("Profile \"" + m_scriptsProfileLoaded->m_name + "\": The following saved directories were not found:");
            for (auto const& s : m_scriptsViewHelper.dirsToExpand)
            {
                appendToLog("-- " + s);
            }
        }

        //Now populate the checkboxes.
        m_scriptsViewHelper.viewStage = FSLoadingStage::PopulatingCheckboxes;

        std::vector<std::string> scriptsToLoad(m_scriptsProfileLoaded->m_scriptsToLoad);
        checkExpandedScriptsFromProfile(ui->treeView_scripts->rootIndex(), &scriptsToLoad);


        // End of the view setup process!
        m_scriptsViewHelper.viewStage = FSLoadingStage::Idle;
        setEnabledScriptsGroup(true);
    }
}


void Dlg_ProfileScripts_Options::checkExpandedScriptsFromProfile(const QModelIndex proxyParentIdx, std::vector<std::string>* scriptsToLoad)
{
    Q_ASSERT(m_scriptsViewHelper.viewStage == FSLoadingStage::PopulatingCheckboxes);
    Q_ASSERT(m_scriptsProfileLoaded != nullptr);
    Q_ASSERT(proxyParentIdx.model() == m_scripts_model_proxy);

    const QModelIndex sourceParentIdx = m_scripts_model_proxy->mapToSource(proxyParentIdx);
    Q_ASSERT(sourceParentIdx.isValid());

    // Loop through all the rows on the same level (with the same parent).
    for (int modelRow = 0; modelRow < m_scripts_model_base->rowCount(sourceParentIdx); ++modelRow)
    {
        const QModelIndex sourceChildIdx = m_scripts_model_base->index(modelRow, 0, sourceParentIdx);
        Q_ASSERT(sourceChildIdx.isValid());

        bool match = false;
        std::string childFileStr (m_scripts_model_base->filePath(sourceChildIdx).toStdString());
        standardizePath(childFileStr);
        for (std::string& strScriptFullPath : *scriptsToLoad)
        {
            if (childFileStr.compare(strScriptFullPath))
                continue;

            match = true;
            strScriptFullPath.clear();
            break;
        }

        const QModelIndex proxyChildIdx = m_scripts_model_proxy->mapFromSource(sourceChildIdx);
        Q_ASSERT(proxyChildIdx.isValid());
        if (match)
        {
            m_scripts_model_proxy->setData(proxyChildIdx, Qt::Checked, Qt::CheckStateRole);
        }
        if (m_scripts_model_base->isDir(sourceChildIdx))
        {
            checkExpandedScriptsFromProfile(proxyChildIdx, scriptsToLoad);
        }
    }

    // Am I done?
    if (proxyParentIdx == ui->treeView_scripts->rootIndex())
    {   // Topmost level. I'm done with the PopulatingCheckboxes task.

        // Reporting missing files.
        scriptsToLoad->erase(
                    std::remove_if(scriptsToLoad->begin(), scriptsToLoad->end(),
                                   [](std::string const& str) -> bool { return str.empty(); }),
                    scriptsToLoad->end()
                );
        if (!scriptsToLoad->empty())
        {
            appendToLog("Profile \"" + m_scriptsProfileLoaded->m_name + "\": The following saved files were not found:");
            for (auto const& s : *scriptsToLoad)
            {
                appendToLog("-- " + s);
            }
        }

        // Mark the completion of the task.
        m_scriptsViewHelper.viewStage = FSLoadingStage::Idle;
    }
}

void Dlg_ProfileScripts_Options::loadScriptsFromProfile(const ScriptsProfile *sp)
{
    // To set the check state of the scripts in the treeview:
    //-- First get the involved folders and, if needed, expand them in the treeView; this is done asynchronously
    //--   and recursively via the ms_scriptTree_directoryLoaded slot.
    //-- Then, check the boxes of the scripts.
    Q_ASSERT(m_scriptsViewHelper.viewStage == FSLoadingStage::Init);

    m_scriptsProfileLoaded = sp;
    updateScriptsView(QString::fromStdString(sp->m_scriptsPath));

    // Find the sub-folders containing the scripts.
    std::vector<std::string> scriptDirectories;
    for (std::string const& elem : sp->m_scriptsToLoad)
    {
        const std::string dir(getDirectoryFromString(elem));
        if (std::find(scriptDirectories.cbegin(), scriptDirectories.cend(), dir) != scriptDirectories.cend())
            continue;
        scriptDirectories.emplace_back(dir);
    }

    // Expand only those subfolders.
    //-- sort the folders in increasing order of depth.
    std::sort(scriptDirectories.begin(), scriptDirectories.end(), comparatorDirLevels);
    m_scriptsViewHelper.dirsToExpand = std::move(scriptDirectories);
}


// ----


void Dlg_ProfileScripts_Options::on_listView_profiles_clicked(const QModelIndex index)
{
    if (m_scriptsViewHelper.viewStage != FSLoadingStage::Idle)
        return;

    /* // For multiple selections:
    QItemSelectionModel selection = ui->listView_profiles->selectionModel()->selectedIndexes();
    if (selection.indexes().isEmpty())
    {
        // empty the scripts tree view
        updateScriptsView("");
        return;
    }
    if (selection.indexes().first().row() > (int)g_scriptsProfiles.size())
        return;
    */
    const int newProfileIndex = index.row();
    if (newProfileIndex == m_currentProfileIndex)
        return;

    setEnabledScriptsGroup(false);
    m_scriptsViewHelper.viewStage = FSLoadingStage::Init;

    m_currentProfileIndex = newProfileIndex;
    ScriptsProfile *sp = &g_scriptsProfiles[m_currentProfileIndex];

    m_scripts_model_proxy->resetToDefault();  // TODO: or another method?

    ui->checkBox_setDefaultProfile->setCheckState(sp->m_defaultProfile ? Qt::Checked : Qt::Unchecked);
    ui->lineEdit_editName->setText(QString::fromStdString(sp->m_name));
    ui->lineEdit_editPath->setText(QString::fromStdString(sp->m_scriptsPath));

    ui->pushButton_profileDelete->setDisabled(false);
    ui->pushButton_profileSave->setDisabled(false);

    if (sp->m_useSpheretables)
    {
        ui->checkBox_spheretables->setCheckState(Qt::Checked);
        ui->treeView_scripts->setDisabled(true);
        return;
    }

    ui->treeView_scripts->setDisabled(false);
    loadScriptsFromProfile(sp);
}

void Dlg_ProfileScripts_Options::on_lineEdit_editPath_textChanged(const QString &arg1)
{
    if (m_scriptsViewHelper.viewStage != FSLoadingStage::Idle)
        return;

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
    QFileDialog dlg;
    dlg.setOption(QFileDialog::ShowDirsOnly);
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setViewMode(QFileDialog::List);
    if (isValidDirectory(ui->lineEdit_editPath->text().toStdString()))
         dlg.setDirectory(ui->lineEdit_editPath->text());

    if (dlg.exec())
    {
        ui->lineEdit_editPath->setText(dlg.directory().absolutePath());
        updateScriptsView(ui->lineEdit_editPath->text());
    }
}

void Dlg_ProfileScripts_Options::saveProfilesToJson()
{
    // Open the file in which we store the Scripts Profiles.
    QFile jsonFile;
    jsonFile.setFileName("ScriptsProfiles.json");
    if (!jsonFile.open(QIODevice::WriteOnly))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText("Error: can't open ScriptsProfiles.json! Can't save the Scripts Profiles!");
        errorDlg.exec();
        return;
    }

    // Convert all profiles (plus the new one) in Json format.
    QJsonObject mainJsonField;
    QJsonObject scriptsProfileJsonField;
    for (size_t i = 0; i < g_scriptsProfiles.size(); ++i)
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
    std::string profileDir = ui->lineEdit_editPath->text().toStdString();
    standardizePath(profileDir);
    ScriptsProfile newProfile(profileDir);
    if (! ui->lineEdit_editName->text().isEmpty() )
        newProfile.m_name = ui->lineEdit_editName->text().toStdString();
    if (ui->checkBox_setDefaultProfile->checkState() == Qt::Checked)
    {
        newProfile.m_defaultProfile = true;
        for (size_t i = 0; i < g_scriptsProfiles.size(); ++i)
            g_scriptsProfiles[i].m_defaultProfile = false;
    }

    // Populate the scripts file list in the profile.
    QStringList selectedScripts = ModelUtils::
            CheckableProxy::FileSystem::extractCheckedFilesPath(m_scripts_model_base, m_scripts_model_proxy, ui->treeView_scripts->rootIndex(), true);
    for (int i = 0; i < selectedScripts.count(); ++i)
        newProfile.m_scriptsToLoad.push_back(selectedScripts.at(i).toStdString());

    g_scriptsProfiles.push_back(newProfile);

    // Refresh the profiles list.
    updateProfilesView();

    // Since the ScriptsProfile instance and the newProfileItem are added sequentially, the index of ScriptsProfile instance
    //  inside the g_scriptsProfiles vector corresponds to the QStandardItem row.
    saveProfilesToJson();
}

void Dlg_ProfileScripts_Options::on_pushButton_profileDelete_clicked()
{
    if (m_currentProfileIndex == -1)
        return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm action", "Are you sure you want to delete the selected profile?",
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
    standardizePath(sp->m_scriptsPath);
    sp->m_defaultProfile = (ui->checkBox_setDefaultProfile->checkState() == Qt::Unchecked) ? false: true;
    if (sp->m_defaultProfile)
    {
        for (int i = 0, sz = (int)g_scriptsProfiles.size(); i < sz; ++i)
        {
            if (i != m_currentProfileIndex)
                g_scriptsProfiles[i].m_defaultProfile = false;
        }
    }
    sp->m_useSpheretables = (ui->checkBox_spheretables->checkState() == Qt::Unchecked) ? false: true;

    // Refresh the profiles list.
    updateProfilesView();

    if (!sp->m_useSpheretables)
    {
        QStringList selectedScripts = ModelUtils::
                CheckableProxy::FileSystem::extractCheckedFilesPath(m_scripts_model_base, m_scripts_model_proxy, ui->treeView_scripts->rootIndex(), true);
        sp->m_scriptsToLoad.clear();
        for (int i = 0; i < selectedScripts.count(); ++i)
            sp->m_scriptsToLoad.push_back(selectedScripts.at(i).toStdString());
    }

    saveProfilesToJson();
}


void Dlg_ProfileScripts_Options::on_pushButton_SelectAllScripts_clicked()
{
    ModelUtils::CheckableProxy::resetCheckedState(m_scripts_model_proxy, true);
}

void Dlg_ProfileScripts_Options::on_pushButton_clearSelection_clicked()
{
    ModelUtils::CheckableProxy::resetCheckedState(m_scripts_model_proxy, false);
}
