#include "maintab_items.h"
#include "ui_maintab_items.h"
#include "globals.h"
#include "common.h"
#include "../spherescript/scriptobjects.h"
#include "../spherescript/scriptutils.h"
#include "../uofiles/uoart.h"
#include "subdlg_searchobj.h"
#include "subdlg_spawn.h"
#include <QMessageBox>
#include <QStandardItem>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
//#include <thread>


MainTab_Items::MainTab_Items(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTab_Items)
{
    ui->setupUi(this);

    installEventFilter(this);   // catch the keystrokes

    m_scriptSearch = nullptr;

    // Create the model for the organizer tree view
    m_organizer_model = new QStandardItemModel(0,0);
    ui->treeView_organizer->setModel(m_organizer_model);
    connect(ui->treeView_organizer->selectionModel(), SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(onManual_treeView_organizer_selectionChanged(const QModelIndex&,const QModelIndex&)));

    // Create the model for the charList table view
    m_objList_model = new QStandardItemModel(0,2);

    QStandardItem *itemList_header0 = new QStandardItem("Item Description");
    QStandardItem *itemList_header1 = new QStandardItem("Item ID");
    m_objList_model->setHorizontalHeaderItem(0, itemList_header0);
    m_objList_model->setHorizontalHeaderItem(1, itemList_header1);

    ui->treeView_objList->setModel(m_objList_model);
    connect(ui->treeView_objList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(onManual_treeView_objList_selectionChanged(const QModelIndex&,const QModelIndex&)));

    // Center column headers text
    ui->treeView_objList->header()->setDefaultAlignment(Qt::AlignHCenter);
    // Stretch horizontal headers
    ui->treeView_objList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView_objList->header()->setSectionResizeMode(1, QHeaderView::Interactive);

    // Set stretch factors to the splitter between the QGraphicsView and the lists layout
    ui->splitter_img_lists->setStretchFactor(0,3);  // column 0: lists layout
    ui->splitter_img_lists->setStretchFactor(1,1);  // column 1: QGraphicsView
}

MainTab_Items::~MainTab_Items()
{
    delete ui;

    delete m_organizer_model;
    delete m_objList_model;
}

bool MainTab_Items::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() != QEvent::KeyPress)
        return QObject::eventFilter(watched, event);

    QKeyEvent* keyEv = static_cast<QKeyEvent*>(event);
    if ( (keyEv->key()==Qt::Key_F2) || (keyEv->key()==Qt::Key_F3) )
    {   // pressed F1 or F2
        if (g_loadedScriptsProfile == -1)   // no profile loaded
            return false;
        if (m_scriptSearch == nullptr)      // i haven't set the search parameters
            return false;

        if (keyEv->key()==Qt::Key_F3)   // search forwards
            doSearch(false);
        else                            // search backwards
            doSearch(true);

        return false;
    }
    else if ( (keyEv->key()==Qt::Key_F) && (keyEv->modifiers() & Qt::ControlModifier) )
    {   // presset CTRL + F
        on_pushButton_search_clicked();
        return false;
    }
    else
        return QObject::eventFilter(watched, event);
}

void MainTab_Items::updateViews()
{
    // Populate the Model to be applied to the QTreeView
    m_organizer_model->removeRows(0, m_organizer_model->rowCount());
    m_categoryMap.clear();
    m_subsectionMap.clear();
    m_objList_model->removeRows(0, m_objList_model->rowCount());
    m_objMapQItemToScript.clear();
    m_objMapScriptToQItem.clear();

    QStandardItem *root = m_organizer_model->invisibleRootItem();

    ScriptObjTree *trees[3] = { g_scriptObjTree_Items, g_scriptObjTree_Templates, g_scriptObjTree_Multis };
    for (int tree_i = 0; tree_i < 3; ++tree_i)
    {
        for (size_t category_i = 0; category_i < trees[tree_i]->m_categories.size(); ++category_i)
        {
            ScriptCategory *categoryInst = trees[tree_i]->m_categories[category_i];
            QStandardItem *categoryItem = new QStandardItem(categoryInst->m_categoryName.c_str());
            m_categoryMap[categoryItem] = categoryInst;
            categoryItem->setSelectable(false);
            if (tree_i == 1)
                categoryItem->setForeground(QBrush(QColor("purple")));
            else if (tree_i == 2)
                categoryItem->setForeground(QBrush(QColor("orange")));
            root->appendRow(categoryItem);
            for (size_t subsection_i = 0; subsection_i < categoryInst->m_subsections.size(); ++subsection_i)
            {
                ScriptSubsection *subsectionInst = categoryInst->m_subsections[subsection_i];
                QStandardItem *subsectionItem = new QStandardItem(subsectionInst->m_subsectionName.c_str());
                categoryItem->appendRow(subsectionItem);
                m_subsectionMap[subsectionItem] = subsectionInst;    // So that i know at which ScriptSubsection each QStandardItem corresponds.
            }
        }
    }
    //m_organizer_model->sort(0, Qt::AscendingOrder);   // Order alphabetically. -> not needed anymore, since the whole ScriptObjTree is now sorted after the parsing
}

void MainTab_Items::onManual_treeView_organizer_selectionChanged(const QModelIndex &selected, const QModelIndex& /* UNUSED deselected */ )
{
    if (m_organizer_model->rowCount() == 0)     // Empty list, can't proceed.
        return;

    QStandardItem *subsection_item = m_organizer_model->itemFromIndex(selected);
    if (!m_subsectionMap.count(subsection_item)) // If the selected item isn't in the map, it is a Category, not a Subsection.
        return;
    ScriptSubsection *subsection_inst = m_subsectionMap[subsection_item];
    //if (subsectionObj == nullptr)   // Checking again (maybe useless, but...), because if the pointer isn't valid the program will crash.
    //    return;

    m_objList_model->removeRows(0,m_objList_model->rowCount());
    m_objMapQItemToScript.clear();
    m_objMapScriptToQItem.clear();

	// disable redrawing the view for every item inserted, so that it will update only once, after all the insertions
    ui->treeView_organizer->setUpdatesEnabled(false);
    ui->treeView_objList->setUpdatesEnabled(false);

    QStandardItem *root = m_objList_model->invisibleRootItem();
    QList<QStandardItem*> dupeItem_rowsElements;

    /* Populate the object list */
    for (size_t subsection_i = 0; subsection_i < subsection_inst->m_objects.size(); ++subsection_i)
    {
        // Build the two QStandardItem for each ScriptObj in this Subsection

        ScriptObj *obj = subsection_inst->m_objects[subsection_i];
        QList<QStandardItem*> row;

        /* Build the description part */
        QStandardItem *description_item = new QStandardItem(obj->m_description.c_str());

        QString bodyStr;
        if (obj->m_baseDef)
            bodyStr = "Base Itemdef";
        else
            bodyStr = "Child Itemdef (Parent: " + QString(obj->m_ID.c_str()) + ")";
        QString colorStr;
        //if (isStringNumericHex(obj->m_color))
        //{
        //    int colorNum = ScriptUtils::strToSphereInt16(obj->m_color);
        //    colorStr = QString("0%1").arg(colorNum, 1, 16, QChar('0')) +
        //            " (Dec: " + QString("%1").arg(colorNum, 0, 10) + ")";
        //}
        //else
            colorStr = obj->m_color.c_str();

        description_item->setToolTip(
                    bodyStr                                                                     + "\n" +
                    "Color: "               + colorStr                                          + "\n" +
                    "Script File: "         + g_scriptFileList[obj->m_scriptFileIndex].c_str()  + "\n" +
                    "Script File Line: "    + QString::number(obj->m_scriptLine));

        if (obj->m_type == SCRIPTOBJ_TYPE_TEMPLATE)
            description_item->setForeground(QBrush(QColor("purple")));
        else if (obj->m_type == SCRIPTOBJ_TYPE_MULTI)
            description_item->setForeground(QBrush(QColor("orange")));

        row.append(description_item);

        /* Build the defname part */
        std::string def = obj->m_defname.empty() ? obj->m_ID : obj->m_defname;
        QStandardItem *defname_item = new QStandardItem(def.c_str());
        row.append(defname_item);

        /* Store the elements in the map and append the whole row to the view */
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
        for (auto it = dupeObj->m_subsection->m_objects.begin(), end = dupeObj->m_subsection->m_objects.end(); it != end; ++it)
        {
            if (!(*it)->m_dupeItem.empty())
                continue;   // it's another dupe item

            if (isStringNumericHex(dupeObj->m_dupeItem))
            {
                // DUPEITEM property is numerical, so it's an ID
                if ((*it)->m_ID != dupeObj->m_dupeItem)
                    continue;
                QList<QStandardItem*> dupeItem_row;
                dupeItem_row << dupeItem_rowsElements[dupeItem_rowsElements_i] << dupeItem_rowsElements[dupeItem_rowsElements_i+1];
                m_objMapScriptToQItem[*it]->appendRow(dupeItem_row);
            }
            else
            {
                // DUPEITEM property is a string, so a defname
                if ((*it)->m_defname != (dupeObj->m_dupeItem))
                    continue;
                QList<QStandardItem*> dupeItem_row;
                dupeItem_row << dupeItem_rowsElements[dupeItem_rowsElements_i] << dupeItem_rowsElements[dupeItem_rowsElements_i+1];
                m_objMapScriptToQItem[*it]->appendRow(dupeItem_row);
            }
            break;
        }
    }

    //m_objList_model->sort(0, Qt::AscendingOrder);   // Order alphabetically. -> not needed anymore, since the whole ScriptObjTree is now sorted after the parsing
    ui->treeView_organizer->setUpdatesEnabled(true);
    ui->treeView_objList->setUpdatesEnabled(true);
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

    if (!g_keystrokeSender.sendString((".add " + IDIndex.data().toString().toStdString()).c_str()))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(g_keystrokeSender.getErrorString().c_str());
        errorDlg.exec();
    }
}

void MainTab_Items::onManual_treeView_objList_selectionChanged(const QModelIndex &selected, const QModelIndex& /*UNUSED deselected*/)
{
    if (g_UOArt == nullptr)
        return;

    if (m_objList_model->rowCount() == 0)     // Empty list, can't proceed.
        return;

    QStandardItem *obj_item = m_objList_model->itemFromIndex(selected);
    if (!m_objMapQItemToScript.count(obj_item)) // If the selected item isn't in the map, it is a Category, not a Subsection.
        return;

    ScriptObj *script = m_objMapQItemToScript[obj_item];
    emit selectedScriptObjChanged(script);

    int id = 0x4000 + script->m_display;
    int hue = ScriptUtils::strToSphereInt16(script->m_color);
    if (hue < 0)    // template or random expr (not supported yet) or strange string
        hue = 0;
    QImage* art = g_UOArt->drawArt(id, hue, false);
    if (art == nullptr)
        return;

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(*art));
    delete art;
    if (ui->graphicsView->scene() != nullptr)
        delete ui->graphicsView->scene();
    QGraphicsScene* scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);
    scene->clear();
    scene->addItem(item);
    //qDebug() << "Item added to scene";
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

    if (!g_keystrokeSender.sendString((".add " + selection->selectedRows(1)[0].data().toString().toStdString()).c_str()))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(g_keystrokeSender.getErrorString().c_str());
        errorDlg.exec();
    }
}

void MainTab_Items::on_pushButton_remove_clicked()
{
    const char command[]=".remove";
    if (!g_keystrokeSender.sendString(command))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(g_keystrokeSender.getErrorString().c_str());
        errorDlg.exec();
    }
}

void MainTab_Items::doSearch(bool backwards)
{
    ScriptObj* obj;
    if (!backwards)     // search forwards
        obj = m_scriptSearch->next();
    else                // search backwards
        obj = m_scriptSearch->previous();
    if (obj == nullptr) // not found
    {
        QMessageBox errorDlg(this);
        errorDlg.setText("Not found!");
        errorDlg.exec();
        return;
    }

    ScriptCategory* category = obj->m_category;
    auto categoryIt = mapSearchByKey(m_categoryMap, category);
    if (categoryIt == m_categoryMap.end())      // not found? odd..
        return;
    //QStandardItem* categoryQ = categoryIt->first;

    ScriptSubsection* subsection = obj->m_subsection;
    auto subsectionIt = mapSearchByKey(m_subsectionMap, subsection);
    if (subsectionIt == m_subsectionMap.end())  // not found? odd..
        return;
    QStandardItem* subsectionQ = subsectionIt->first;

    QModelIndex emptyIdx;
    QModelIndex subsectionIdx = m_organizer_model->indexFromItem(subsectionQ);
    //QModelIndex categoryIdx = m_organizer_model->indexFromItem(categoryQ);
    static QModelIndex prevSubsectionIdx;

    ui->treeView_organizer->scrollTo(subsectionIdx, QAbstractItemView::PositionAtCenter);
    //ui->treeView_organizer->setExpanded(categoryIdx, true);   // automatically expanded by the select method
    ui->treeView_organizer->selectionModel()->select(subsectionIdx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
    if (subsectionIdx != prevSubsectionIdx)
    {
        onManual_treeView_organizer_selectionChanged(subsectionIdx, emptyIdx);
        prevSubsectionIdx = subsectionIdx;
    }

    QStandardItem* objQ = m_objMapScriptToQItem[obj];   // should check if found/not found?
    QModelIndex objIdx = m_objList_model->indexFromItem(objQ);

    onManual_treeView_objList_selectionChanged(objIdx, emptyIdx);
    ui->treeView_objList->scrollTo(objIdx, QAbstractItemView::PositionAtCenter);
    ui->treeView_objList->selectionModel()->select(objIdx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Toggle);
}

void MainTab_Items::on_pushButton_search_clicked()
{
    if (g_loadedScriptsProfile == -1)
        return;

    SubDlg_SearchObj dlg(window());
    if (!dlg.exec())
        return;

    const std::vector<ScriptObjTree*> trees =
    {
        objTree(SCRIPTOBJ_TYPE_ITEM),
        objTree(SCRIPTOBJ_TYPE_TEMPLATE),
        objTree(SCRIPTOBJ_TYPE_MULTI)
    };

    ScriptSearch::SearchBy_t searchBy;
    bool caseSensitive;
    std::string key;
    dlg.getSearchData(searchBy, caseSensitive, key);
    m_scriptSearch.reset(new ScriptSearch(trees, searchBy, caseSensitive, key));
    doSearch(false);
}

void MainTab_Items::on_pushButton_search_back_clicked()
{
    if (g_loadedScriptsProfile == -1)   // no profile loaded
        return;
    if (m_scriptSearch == nullptr)      // i haven't set the search parameters
        return;

    doSearch(true);    // search backwards
}

void MainTab_Items::on_pushButton_search_next_clicked()
{
    if (g_loadedScriptsProfile == -1)   // no profile loaded
        return;
    if (m_scriptSearch == nullptr)      // i haven't set the search parameters
        return;

       doSearch(false);    // search forwards
}

void MainTab_Items::on_pushButton_spawner_clicked()
{
    SubDlg_Spawn* spawner = new SubDlg_Spawn();
    connect(this, SIGNAL(selectedScriptObjChanged(ScriptObj*)), spawner, SLOT(onCust_selectedObj_changed(ScriptObj*)));
    spawner->show();

    // get the selected object and update the spawner dialog with the current object
    QItemSelectionModel *selection = ui->treeView_objList->selectionModel();
    if (!selection->hasSelection())
        return;
    QStandardItem* qitem = m_objList_model->itemFromIndex( selection->selectedRows(0)[0] );
    ScriptObj *script = m_objMapQItemToScript[qitem];
    emit selectedScriptObjChanged(script);
}
