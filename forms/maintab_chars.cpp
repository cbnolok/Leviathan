#include "maintab_chars.h"
#include "ui_maintab_chars.h"
#include "globals.h"
#include "common.h"
#include "../spherescript/scriptobjects.h"
#include "../spherescript/scriptutils.h"
#include "../keystrokesender/keystrokesender.h"
#include "../uofiles/uoanim.h"
#include <QMessageBox>
#include <QStandardItem>
#include <QGraphicsPixmapItem>
//#include <thread>



MainTab_Chars::MainTab_Chars(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTab_Chars)
{
    ui->setupUi(this);

    // Get ready to send text to the client
    m_keystrokeSender = new keystrokesender::KeystrokeSender();

    // Create the model for the organizer tree view
    m_organizer_model = new QStandardItemModel(0,0);
    ui->treeView_organizer->setModel(m_organizer_model);

    // Create the model for the charList tree view
    m_objList_model = new QStandardItemModel(0,2);

    QStandardItem *charList_header0 = new QStandardItem("NPC Description");
    QStandardItem *charList_header1 = new QStandardItem("NPC ID");
    m_objList_model->setHorizontalHeaderItem(0, charList_header0);
    m_objList_model->setHorizontalHeaderItem(1, charList_header1);

    ui->treeView_objList->setModel(m_objList_model);
    connect(ui->treeView_objList->selectionModel(), SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(onManual_treeView_objList_selectionChanged(const QModelIndex&,const QModelIndex&)));

    // Center column headers text
    ui->treeView_objList->header()->setDefaultAlignment(Qt::AlignHCenter);
    // Stretch column headers
    ui->treeView_objList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView_objList->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}


MainTab_Chars::~MainTab_Chars()
{
    delete ui;

    delete m_keystrokeSender;
    delete m_organizer_model;
    delete m_objList_model;
}

void MainTab_Chars::updateViews()
{
    // Populate the Model to be applied to the QTreeView
    m_organizer_model->removeRows(0, m_organizer_model->rowCount());
    m_subsectionMap.clear();
    m_objList_model->removeRows(0, m_objList_model->rowCount());
    m_objMapQItemToScript.clear();
    m_objMapScriptToQItem.clear();

    QStandardItem *root = m_organizer_model->invisibleRootItem();

    ScriptObjTree *trees[2] = { g_scriptObjTree_Chars, g_scriptObjTree_Spawns };
    for (int tree_i = 0; tree_i < 2; tree_i++)
    {
        for (size_t category_i = 0; category_i < trees[tree_i]->m_categories.size(); category_i++)
        {
            ScriptCategory *category_inst = trees[tree_i]->m_categories[category_i];
            QStandardItem *category_item = new QStandardItem(category_inst->m_categoryName.c_str());
            category_item->setSelectable(false);
            if (tree_i == 1)
                category_item->setForeground(QBrush(QColor("red")));
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

void MainTab_Chars::on_treeView_organizer_clicked(const QModelIndex &index)
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

    /* Populate the object list */
    for (size_t subsection_i = 0; subsection_i < subsection_inst->m_objects.size(); subsection_i++)
    {
        // Build the two QStandardItem for each ScriptObj in this Subsection

        ScriptObj *obj = subsection_inst->m_objects[subsection_i];
        QList<QStandardItem*> row;

        /* Build the description part */
        QStandardItem *description_item = new QStandardItem(obj->m_description.c_str());

        QString bodyStr;
        if (obj->m_baseDef)
            bodyStr = "Base Chardef";
        else
            bodyStr = "Child Chardef (Parent: " + QString(obj->m_ID.c_str()) + ")";
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

        if (obj->m_type == SCRIPTOBJ_TYPE_SPAWN)
            description_item->setForeground(QBrush(QColor("red")));

        row.append(description_item);

        /* Build the defname part */
        std::string def = obj->m_defname.empty() ? obj->m_ID : obj->m_defname;
        QStandardItem *defname_item = new QStandardItem(def.c_str());
        row.append(defname_item);

        /* Store the elements in the map and append the whole row to the view */
        m_objMapQItemToScript[description_item] = obj;    // For each row, only the Description item is "linked" to the obj via the map.
        m_objMapScriptToQItem[obj] = description_item;

        m_objList_model->appendRow(row);        
    }
    m_objList_model->sort(0, Qt::AscendingOrder);  // Order alphabetically.
}

void MainTab_Chars::on_treeView_objList_doubleClicked(const QModelIndex &index)
{
    if (m_objList_model->rowCount() == 0)     // Empty list, can't proceed.
        return;

    //QStandardItem *objItem = m_objList_model->itemFromIndex(index);
    //if (!m_objMapQItemToScript.count(objItem)) // If the selected item isn't in the map there's something wrong.
    //    return;
    //ScriptObj *obj = m_objMapQItemToScript[objItem];

    QModelIndex IDIndex(m_objList_model->index(index.row(), 1, index.parent()));

    if (!m_keystrokeSender->sendString((".add " + IDIndex.data().toString().toStdString()).c_str()))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(m_keystrokeSender->getErrorString().c_str());
        errorDlg.exec();
    }
}

void MainTab_Chars::onManual_treeView_objList_selectionChanged(const QModelIndex &selected, const QModelIndex& /* UNUSED deselected */ )
{
    if (g_UOAnim == nullptr)
        return;

    if (m_objList_model->rowCount() == 0)     // Empty list, can't proceed.
        return;

    QStandardItem *obj_item = m_objList_model->itemFromIndex(selected);
    if (!m_objMapQItemToScript.count(obj_item)) // If the selected item isn't in the map, it is a Category, not a Subsection.
        return;

    ScriptObj *script = m_objMapQItemToScript[obj_item];
    int id = script->m_display;
    int hue = ScriptUtils::strToSphereInt16(script->m_color);
    if (hue < 0)    // template or random expr (not supported yet) or strange string
        hue = 0;
    QImage* frameimg = g_UOAnim->drawAnimFrame(id, 0, 1, 0, hue);
    if (frameimg == nullptr)
        return;

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(*frameimg));
    delete frameimg;
    if (ui->graphicsView->scene() != nullptr)
        delete ui->graphicsView->scene();
    QGraphicsScene* scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);
    scene->clear();
    scene->addItem(item);
    //qDebug() << "Char added to scene";
}

void MainTab_Chars::on_pushButton_collapseAll_clicked()
{
    ui->treeView_organizer->collapseAll();
    ui->treeView_objList->collapseAll();
}

void MainTab_Chars::on_pushButton_summon_clicked()
{
    if (m_objList_model->rowCount() == 0)     // Empty list, can't proceed.
        return;
    QItemSelectionModel *selection = ui->treeView_objList->selectionModel();
    if (!selection->hasSelection())
        return;

    if (!m_keystrokeSender->sendString((".add " + selection->selectedRows(1)[0].data().toString().toStdString()).c_str()))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(m_keystrokeSender->getErrorString().c_str());
        errorDlg.exec();
    }
}

void MainTab_Chars::on_pushButton_remove_clicked()
{
    const char command[]=".remove";
    if (!m_keystrokeSender->sendString(command))
    {
        QMessageBox errorDlg(this);
        errorDlg.setText(m_keystrokeSender->getErrorString().c_str());
        errorDlg.exec();
    }
}