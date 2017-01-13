#ifndef MAINTAB_ITEMS_H
#define MAINTAB_ITEMS_H

#include <QWidget>
#include <QStandardItemModel>

class UOClientCom;
class ScriptCategory;
class ScriptSubsection;
class ScriptObj;


namespace Ui {
class MainTab_Items;
}

class MainTab_Items : public QWidget
{
    Q_OBJECT

public:
    explicit MainTab_Items(QWidget *parent = 0);
    ~MainTab_Items();
    void updateViews();

private slots:
    void on_treeView_organizer_clicked(const QModelIndex &index);
    void on_treeView_objList_doubleClicked(const QModelIndex &index);
    void on_pushButton_collapseAll_clicked();
    void on_pushButton_add_clicked();
    void on_pushButton_remove_clicked(); 

private:
    Ui::MainTab_Items *ui;
    UOClientCom *m_UOCom;
    std::map<QStandardItem*, ScriptSubsection*> m_subsectionMap;    // link the entry (QStandardItem) in the organizer to the corresponding ScriptSubsection class instance.
    std::map<QStandardItem*, ScriptObj*> m_objMapQItemToScript;     // link the entry (QStandardItem) in the itemList to the corresponding ScriptObj class instance.
    std::map<ScriptObj*, QStandardItem*> m_objMapScriptToQItem;     // link the ScriptObj class instance in the itemList to the corresponding entry (QStandardItem).
    QStandardItemModel *m_organizer_model;
    QStandardItemModel *m_objList_model;
};

#endif // MAINTAB_ITEMS_H
