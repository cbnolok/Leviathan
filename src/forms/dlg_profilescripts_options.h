#ifndef DLG_PROFILESCRIPTS_OPTIONS_H
#define DLG_PROFILESCRIPTS_OPTIONS_H

#include "../settings/scriptsprofile.h"
#include <QDialog>
#include <QModelIndex>
#include <vector>

class QFileSystemModel;
class QStandardItemModel;
class CheckableProxyModel;
class ScriptObj;


namespace Ui {
class Dlg_ProfileScripts_Options;
}

class Dlg_ProfileScripts_Options : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_ProfileScripts_Options(QWidget *parent = nullptr);
    ~Dlg_ProfileScripts_Options();

private slots:
    void on_lineEdit_editPath_textChanged(const QString &arg1);
    void on_pushButton_pathBrowse_clicked();

    void on_listView_profiles_clicked(const QModelIndex index);
    void on_pushButton_profileAdd_clicked();
    void on_pushButton_profileSave_clicked();
    void on_pushButton_profileDelete_clicked();

    //void on_treeView_scripts_clicked(const QModelIndex &index);
    void on_pushButton_SelectAllScripts_clicked();
    void on_pushButton_clearSelection_clicked();

    void ms_scriptTree_directoryLoaded(QString);

private:
    Ui::Dlg_ProfileScripts_Options *ui;
    QFileSystemModel *m_scripts_model_base;
    CheckableProxyModel *m_scripts_model_proxy;
    QStandardItemModel *m_profiles_model;

    std::vector<ScriptsProfile> m_scriptsProfiles;
    int m_currentProfileIndex;
    const ScriptsProfile* m_scriptsProfileLoaded;

    void setEnabledScriptsGroup(bool);

    void updateProfilesView();
    void updateScriptsView(QString path);
    void saveProfilesToJson();

    void loadScriptsFromProfile(const ScriptsProfile *sp);
    void checkExpandedScriptsFromProfile(const QModelIndex proxyParentIdx, std::vector<std::string> *scriptsToLoad);

private:
    enum class FSLoadingStage
    {
        Idle,
        Init,
        ExpandingTreeStructure,
        PopulatingCheckboxes
    };
    struct
    {
        FSLoadingStage viewStage = FSLoadingStage::Idle;

        // Below are helpers for the ExpandingTreeStructure stage.
        int pendingDirectories = INT_MIN;
        std::vector<std::string> dirsToExpand;
    }
    m_scriptsViewHelper;
};


#endif // DLG_PROFILESCRIPTS_OPTIONS_H
