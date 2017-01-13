#ifndef TAB_CHARS_H
#define TAB_CHARS_H

#include <QWidget>
#include <QStandardItemModel>

class UOClientCom;
class ScriptCategory;
class ScriptSubsection;
class ScriptObj;


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

private slots:
    void on_treeView_organizer_clicked(const QModelIndex &index);
    void on_treeView_objList_doubleClicked(const QModelIndex &index);
    void on_pushButton_collapseAll_clicked();
    void on_pushButton_summon_clicked();
    void on_pushButton_remove_clicked();

private:
    Ui::MainTab_Chars *ui;
    UOClientCom *m_UOCom;
    std::map<QStandardItem*, ScriptSubsection*> m_subsectionMap;    // link the entry (QStandardItem) in the organizer to the corresponding ScriptSubsection class instance.
    std::map<QStandardItem*, ScriptObj*> m_objMap;          // link the entry (QStandardItem) in the charList to the corresponding ScriptObj class instance.
    QStandardItemModel *m_organizer_model;
    QStandardItemModel *m_objList_model;
};


#endif // TAB_CHARS_H
