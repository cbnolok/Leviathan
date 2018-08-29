#ifndef DLG_PROFILECLIENT_OPTIONS_H
#define DLG_PROFILECLIENT_OPTIONS_H

#include <vector>
#include <QDialog>
#include <QModelIndex>
#include "../settings/clientprofile.h"

class QStandardItemModel;


namespace Ui {
class Dlg_ProfileClient_Options;
}

class Dlg_ProfileClient_Options : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_ProfileClient_Options(QWidget *parent = 0);
    ~Dlg_ProfileClient_Options();

private slots:
    void on_lineEdit_editPath_textChanged(const QString &arg1);
    void on_pushButton_pathBrowse_clicked();
    void on_pushButton_profileDelete_clicked();
    void on_pushButton_profileAdd_clicked();
    void on_pushButton_profileSave_clicked();
    void on_listView_profiles_clicked(const QModelIndex &index);

private:
    Ui::Dlg_ProfileClient_Options *ui;
    QStandardItemModel *m_profiles_model;

    std::vector<ClientProfile> m_scriptsProfiles;
    int m_currentProfileIndex = -1;

    void updateProfilesView();
    void saveProfilesToJson();
};

#endif // DLG_PROFILECLIENT_OPTIONS_H
