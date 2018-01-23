#include "maintab_tools.h"
#include "ui_maintab_tools.h"
#include "dlg_huepicker.h"


MainTab_Tools::MainTab_Tools(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTab_Tools)
{
    m_dlg_huepicker = nullptr;

    ui->setupUi(this);
}

MainTab_Tools::~MainTab_Tools()
{
    delete ui;

    delete m_dlg_huepicker;
}

void MainTab_Tools::on_pushButton_huePicker_clicked()
{
    if (!m_dlg_huepicker)
        m_dlg_huepicker = new Dlg_HuePicker();
    m_dlg_huepicker->show();
}
