#include "subdlg_taskprogress.h"
#include "ui_subdlg_taskprogress.h"


SubDlg_TaskProgress::SubDlg_TaskProgress(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SubDlg_TaskProgress)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::CustomizeWindowHint);    // Don't show window borders or close button. Not needed, since it's a widget and not a dialog.
}

SubDlg_TaskProgress::~SubDlg_TaskProgress()
{
    delete ui;
}

void SubDlg_TaskProgress::setProgressMax(int max)
{
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(max);
}

void SubDlg_TaskProgress::setProgressVal(int val)
{
    ui->progressBar->setValue(val);
}

// TODO: dividi in due slots: uno che aggiorna solo la label e l'altro che aggiorna solo il testo
void SubDlg_TaskProgress::setLabelText(QString msg)
{
    ui->label->setText(msg);
}

