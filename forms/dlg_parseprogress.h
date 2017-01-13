#ifndef DLG_PARSEPROGRESS_H
#define DLG_PARSEPROGRESS_H

#include <QWidget>


namespace Ui {
class Dlg_ParseProgress;
}

class Dlg_ParseProgress : public QWidget
{
    Q_OBJECT

public:
    explicit Dlg_ParseProgress(QWidget *parent = 0);
    ~Dlg_ParseProgress();

public slots:
    void setProgressMax(int max);
    void setProgressVal(int val);
    void setLabelText(QString msg);

private:
    Ui::Dlg_ParseProgress *ui;
};

#endif // DLG_PARSEPROGRESS_H
