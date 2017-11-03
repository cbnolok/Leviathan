#ifndef MAINTAB_CHARS_H
#define MAINTAB_CHARS_H

#include "subdlg_searchobj.h"
#include <QWidget>
#include <memory>   // for smart pointers


namespace keystrokesender {
class KeystrokeSender;
}
class ScriptCategory;
class ScriptSubsection;
class ScriptObj;
class QStandardItem;
class QStandardItemModel;

namespace Ui {
class MainTab_Chars;
}

class MainTab_Chars : public QWidget
{
    Q_OBJECT

public:
    explicit MainTab_Chars(QWidget *parent = 0);
    ~MainTab_Chars();
    void updateViews();

protected:
    bool eventFilter(QObject* watched, QEvent* event);

private slots:
    void onManual_treeView_organizer_selectionChanged(const QModelIndex &selected, const QModelIndex& /* UNUSED deselected */);
    void on_treeView_objList_doubleClicked(const QModelIndex &index);
    void onManual_treeView_objList_selectionChanged(const QModelIndex &selected, const QModelIndex& /* UNUSED deselected */);
    void on_pushButton_collapseAll_clicked();
    void on_pushButton_summon_clicked();
    void on_pushButton_remove_clicked();
    void on_pushButton_search_clicked();

private:
    Ui::MainTab_Chars *ui;

    std::map<QStandardItem*, ScriptCategory*> m_categoryMap;        // link the entry (QStandardItem) in the organizer to the corresponding ScriptCategory class instance.
    std::map<QStandardItem*, ScriptSubsection*> m_subsectionMap;    // link the entry (QStandardItem) in the organizer to the corresponding ScriptSubsection class instance.
    std::map<QStandardItem*, ScriptObj*> m_objMapQItemToScript;     // link the entry (QStandardItem) in the itemList to the corresponding ScriptObj class instance.
    std::map<ScriptObj*, QStandardItem*> m_objMapScriptToQItem;     // link the ScriptObj class instance in the itemList to the corresponding entry (QStandardItem).

    QStandardItemModel *m_organizer_model;
    QStandardItemModel *m_objList_model;

    keystrokesender::KeystrokeSender *m_keystrokeSender;
    std::unique_ptr<ScriptSearch> m_scriptSearch;

    void doSearch (bool backwards);
};


#endif // MAINTAB_CHARS_H
