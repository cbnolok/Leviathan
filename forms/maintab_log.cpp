#include "ui_maintab_log.h"
#include "maintab_log.h"


MainTab_Log::MainTab_Log(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTab_Log)
{
    ui->setupUi(this);
}

MainTab_Log::~MainTab_Log()
{
    delete ui;
}

void MainTab_Log::appendText(const char* str)
{
    ui->textBrowser->append(str);
}

void MainTab_Log::on_pushButton_clear_clicked()
{
    ui->textBrowser->clear();
}
