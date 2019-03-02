#ifndef MAINTAB_ITEMS_H
#define MAINTAB_ITEMS_H

#include <QWidget>
#include <memory>   // for smart pointers

namespace ks {
class KeystrokeSender;
}
class SubDlg_SearchObj;
struct ScriptSearch;
class ScriptCategory;
class ScriptSubsection;
class ScriptObj;
class QStandardItem;
class QStandardItemModel;


namespace Ui {
class MainTab_Items;
}

class MainTab_Items : public QWidget
{
    Q_OBJECT

public:
    explicit MainTab_Items(QWidget *parent = nullptr);
    ~MainTab_Items();
    void updateViews();

protected:
    bool eventFilter(QObject* watched, QEvent* event);

signals:
    void selectedScriptObjChanged(ScriptObj* selected);

private slots:
    void onManual_treeView_organizer_selectionChanged(const QModelIndex &selected, const QModelIndex& /*UNUSED deselected*/);
    void on_treeView_objList_doubleClicked(const QModelIndex &index);
    void onManual_treeView_objList_selectionChanged(const QModelIndex &selected, const QModelIndex& /*UNUSED deselected*/);
    void on_pushButton_collapseAll_clicked();
    void on_pushButton_add_clicked();
    void on_pushButton_remove_clicked(); 
    void on_pushButton_search_clicked();
    void on_pushButton_search_back_clicked();
    void on_pushButton_search_next_clicked();
    void on_pushButton_spawner_clicked();
    void on_checkBox_lockDown_stateChanged(int arg1);

private:
    Ui::MainTab_Items *ui;

    std::map<QStandardItem*, ScriptCategory*> m_categoryMap;        // link the entry (QStandardItem) in the organizer to the corresponding ScriptCategory class instance.
    std::map<QStandardItem*, ScriptSubsection*> m_subsectionMap;    // link the entry (QStandardItem) in the organizer to the corresponding ScriptSubsection class instance.
    std::map<QStandardItem*, ScriptObj*> m_objMapQItemToScript;     // link the entry (QStandardItem) in the itemList to the corresponding ScriptObj class instance.
    std::map<ScriptObj*, QStandardItem*> m_objMapScriptToQItem;     // link the ScriptObj class instance in the itemList to the corresponding entry (QStandardItem).

    QStandardItemModel *m_organizer_model;
    QStandardItemModel *m_objList_model;

    std::unique_ptr<SubDlg_SearchObj> m_subdlg_searchObj;
    std::unique_ptr<ScriptSearch> m_scriptSearch;

    bool m_lockDown;

    void doSearch (bool backwards);
};

#endif // MAINTAB_ITEMS_H
