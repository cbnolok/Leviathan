#include "maintab_items.h"
#include "ui_maintab_items.h"
#include "globals.h"
#include "../spherescript/scriptobjects.h"
#include "../spherescript/scriptutils.h"
#include "../uofiles/uoclientcom.h"

#include <QMessageBox>
#include <sstream>  // for std::stringstream
#include <ios>      // for std::hex
#include <thread>

#include <QDebug>

MainTab_Items::MainTab_Items(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTab_Items)
{
    ui->setupUi(this);

    // Get ready to send text to the client
    m_UOCom = new UOClientCom();

    // Create the model for the organizer tree view
    m_organizer_model = new QStandardItemModel(0,0);
    ui->treeView_organizer->setModel(m_organizer_model);

    // Create the model for the charList table view
    m_objList_model = new QStandardItemModel(0,2);

    QStandardItem *itemList_header0 = new QStandardItem("Item Description");
    QStandardItem *itemList_header1 = new QStandardItem("Item ID");
    m_objList_model->setHorizontalHeaderItem(0, itemList_header0);
    m_objList_model->setHorizontalHeaderItem(1, itemList_header1);

    ui->treeView_objList->setModel(m_objList_model);

    // Center column headers text
    ui->treeView_objList->header()->setDefaultAlignment(Qt::AlignHCenter);
    // Stretch horizontal headers
    ui->treeView_objList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView_objList->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}

MainTab_Items::~MainTab_Items()
{
    delete ui;

    delete m_UOCom;
    delete m_organizer_model;
    delete m_objList_model;
}

void MainTab_Items::updateViews()
{
    // Populate the Model to be applied to the QTreeView
    m_organizer_model->removeRows(0, m_organizer_model->rowCount());
    m_subsectionMap.clear();
    m_objList_model->removeRows(0, m_objList_model->rowCount());
    m_objMapQItemToScript.clear();
    m_objMapScriptToQItem.clear();

    QStandardItem *root = m_organizer_model->invisibleRootItem();

    ScriptObjTree *trees[3] = { g_scriptObjTree_Items, g_scriptObjTree_Templates, g_scriptObjTree_Multis };
    for (int tree_i = 0; tree_i < 3; tree_i++)
    {
        for (size_t category_i = 0; category_i < trees[tree_i]->m_categories.size(); category_i++)
        {
            ScriptCategory *category_inst = trees[tree_i]->m_categories[category_i];
            QStandardItem *category_item = new QStandardItem(category_inst->m_categoryName.c_str());
            category_item->setSelectable(false);
            if (tree_i == 1)
                category_item->setForeground(QBrush(QColor("purple")));
            else if (tree_i == 2)
                category_item->setForeground(QBrush(QColor("orange")));
            root->appendRow(category_item);
            for (size_t subsection_i = 0; subsection_i < category_inst->m_subsections.size(); subsection_i++)
            {
                ScriptSubsection *subsection_inst = category_inst->m_subsections[subsection_i];
                QStandardItem *subsectionItem = new QStandardItem(subsection_inst->m_subsectionName.c_str());
                category_item->appendRow(subsectionItem);
                m_subsectionMap[subsectionItem] = subsection_inst;    // So that i know at which ScriptSubsection each QStandardItem corresponds.
            }
        }
    }
    m_organizer_model->sort(0, Qt::AscendingOrder);         // Order alphabetically.
}

void MainTab_Items::on_treeView_organizer_clicked(const QModelIndex &index)
{
    if (m_organizer_model->rowCount() == 0)     // Empty list, can't proceed.
        return;

    QStandardItem *subsection_item = m_organizer_model->itemFromIndex(index);
    if (!m_subsectionMap.count(subsection_item)) // If the selected item isn't in the map, it is a Category, not a Subsection.
        return;
    ScriptSubsection *subsection_inst = m_subsectionMap[subsection_item];
    //if (subsectionObj == nullptr)   // Checking again (maybe useless, but...), because if the pointer isn't valid the program will crash.
    //    return;

    m_objList_model->removeRows(0,m_objList_model->rowCount());
    m_objMapQItemToScript.clear();
    m_objMapScriptToQItem.clear();

    QStandardItem *root = m_objList_model->invisibleRootItem();
    QList<QStandardItem*> dupeItem_rowsElements;

    // Build the two QStandardItem for each ScriptObj in this Subsection
    for (size_t subsection_i = 0; subsection_i < subsection_inst->m_objects.size(); subsection_i++)
    {
        ScriptObj *obj = subsection_inst->m_objects[subsection_i];
        QList<QStandardItem*> row;
        QStandardItem *description_item = new QStandardItem(obj->m_description.c_str());
        description_item->setToolTip(
                    "Color: " + QString("0%1").arg(obj->m_color, 1, 16, QChar('0')) + "\n" +
                    "Script File: " + obj->m_scriptFile.c_str() + "\n" +
                    "Script File Line: " + QString::number(obj->m_scriptLine));
        if (obj->m_type == SCRIPTOBJ_TYPE_TEMPLATE)
            description_item->setForeground(QBrush(QColor("purple")));
        else if (obj->m_type == SCRIPTOBJ_TYPE_MULTI)
            description_item->setForeground(QBrush(QColor("orange")));
        row.append(description_item);
        std::string def;
        if (obj->m_defname.empty())
        {
            std::stringstream ssDef;
            ssDef << "0x" << std::hex << obj->m_ID;
            def = ssDef.str();
        }
        else
        {
            def = obj->m_defname;
        }
        QStandardItem *defname_item = new QStandardItem(def.c_str());
        row.append(defname_item);
        m_objMapQItemToScript[description_item] = obj;    // For each row, only the Description item is "linked" to the obj via the map.
        m_objMapScriptToQItem[obj] = description_item;

        if (obj->m_dupeItem.empty())
            root->appendRow(row);
        else
            dupeItem_rowsElements.append(row);  // store sequentially all the description and defname rows in one array
    }

    // Check if we have Dupe Items in this Subsection. For each of them, loop through all the Script Objects in the Subsection,
    //  if the looped obj isn't a dupe item and the id/defname matches the Dupe Item's m_dupeItem property, then it's the Original one.
    // Then we can append the Dupe as a child of the Original.
    for (int dupeItem_rowsElements_i = 0; dupeItem_rowsElements_i < dupeItem_rowsElements.size() - 1; dupeItem_rowsElements_i += 2)
    {
        ScriptObj *dupeObj = m_objMapQItemToScript[dupeItem_rowsElements[dupeItem_rowsElements_i]];
        // If it's a Dupe item, find the parent item inside the same Category and Subsection and append one another.
        for (auto it = dupeObj->m_subsection->m_objects.begin(); it != dupeObj->m_subsection->m_objects.end(); it++)
        {
            if (!(*it)->m_dupeItem.empty())
                continue;   // it's another dupe item

            int dupeNum = ScriptUtils::strToSphereInt(dupeObj->m_dupeItem);
            if (dupeNum != -1)
            {
                // DUPEITEM property is numerical, so it's a ID
                if ((*it)->m_ID != dupeNum)
                    continue;
                QList<QStandardItem*> dupeItem_row;
                dupeItem_row << dupeItem_rowsElements[dupeItem_rowsElements_i] << dupeItem_rowsElements[dupeItem_rowsElements_i+1];
                m_objMapScriptToQItem[*it]->appendRow(dupeItem_row);
            }
            else
            {
                // DUPEITEM property is a string, so a defname
                if ((*it)->m_defname.compare(dupeObj->m_dupeItem) != 0)
                    continue;
                QList<QStandardItem*> dupeItem_row;
                dupeItem_row << dupeItem_rowsElements[dupeItem_rowsElements_i] << dupeItem_rowsElements[dupeItem_rowsElements_i+1];
                m_objMapScriptToQItem[*it]->appendRow(dupeItem_row);
            }
            break;
        }
    }

    m_objList_model->sort(0, Qt::AscendingOrder);  // Order alphabetically.
}

void MainTab_Items::on_treeView_objList_doubleClicked(const QModelIndex &index)
{
    if (m_objList_model->rowCount() == 0)     // Empty list, can't proceed.
        return;

    //QStandardItem *objItem = m_objList_model->itemFromIndex(index);
    //if (!m_objMapQItemToScript.count(objItem)) // If the selected item isn't in the map there's something wrong.
    //    return;
    //ScriptObj *obj = m_objMapQItemToScript[objItem];

    QModelIndex IDIndex(m_objList_model->index(index.row(), 1, index.parent()));

    if (!m_UOCom->sendString((".add " + IDIndex.data().toString().toStdString()).c_str()))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(m_UOCom->getErrorString());
        errorDlg.exec();
    }
}

void MainTab_Items::on_pushButton_collapseAll_clicked()
{
    ui->treeView_organizer->collapseAll();
    ui->treeView_objList->collapseAll();
}

void MainTab_Items::on_pushButton_add_clicked()
{
    if (m_objList_model->rowCount() == 0)     // Empty list, can't proceed.
        return;
    QItemSelectionModel *selection = ui->treeView_objList->selectionModel();
    if (!selection->hasSelection())
        return;

    if (!m_UOCom->sendString((".add " + selection->selectedRows(1)[0].data().toString().toStdString()).c_str()))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(m_UOCom->getErrorString());
        errorDlg.exec();
    }
}

void MainTab_Items::on_pushButton_remove_clicked()
{
    const char command[]=".remove";
    if (!m_UOCom->sendString(command))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(m_UOCom->getErrorString());
        errorDlg.exec();
    }
}

