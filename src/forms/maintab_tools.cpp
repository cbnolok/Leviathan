#include "maintab_tools.h"
#include "ui_maintab_tools.h"
#include "dlg_huepicker.h"
#include "dlg_worldmap.h"


MainTab_Tools::MainTab_Tools(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTab_Tools)
{
    m_dlg_huepicker = nullptr;
    m_dlg_worldmap = nullptr;

    ui->setupUi(this);
}

MainTab_Tools::~MainTab_Tools()
{
    delete ui;

    delete m_dlg_huepicker;
    delete m_dlg_worldmap;
}

void MainTab_Tools::on_pushButton_huePicker_clicked()
{
    if (!m_dlg_huepicker)
        m_dlg_huepicker = new Dlg_HuePicker();
    m_dlg_huepicker->show();
}

void MainTab_Tools::on_pushButton_worldMap_clicked()
{
    if (!m_dlg_worldmap)
        m_dlg_worldmap = new Dlg_WorldMap();
    m_dlg_worldmap->show();
}
