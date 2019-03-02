#ifndef SUBDLG_TASKPROGRESS_H
#define SUBDLG_TASKPROGRESS_H

#include <QWidget>


namespace Ui {
class SubDlg_TaskProgress;
}

class SubDlg_TaskProgress : public QWidget
{
    Q_OBJECT

public:
    explicit SubDlg_TaskProgress(QWidget *parent = nullptr);
    ~SubDlg_TaskProgress();

public slots:
    void setProgressMax(int max);
    void setProgressVal(int val);
    void setLabelText(QString msg);

private:
    Ui::SubDlg_TaskProgress *ui;
};

#endif // SUBDLG_TASKPROGRESS_H
