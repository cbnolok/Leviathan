#include "dlg_parseprogress.h"
#include "ui_dlg_parseprogress.h"


Dlg_ParseProgress::Dlg_ParseProgress(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Dlg_ParseProgress)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::CustomizeWindowHint);    // Don't show window borders or close button. Not needed, since it's a widget and not a dialog.
}

Dlg_ParseProgress::~Dlg_ParseProgress()
{
    delete ui;
}

void Dlg_ParseProgress::setProgressMax(int max)
{
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(max);
}

void Dlg_ParseProgress::setProgressVal(int val)
{
    ui->progressBar->setValue(val);
}

// TODO: dividi in due slots: uno che aggiorna solo la label e l'altro che aggiorna solo il testo
void Dlg_ParseProgress::setLabelText(QString msg)
{
    ui->label->setText(msg);
}

