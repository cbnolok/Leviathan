#ifndef TAB_LOG_H
#define TAB_LOG_H

#include <QWidget>
#include <string>


namespace Ui {
class MainTab_Log;
}

class MainTab_Log : public QWidget
{
    Q_OBJECT

public:
    explicit MainTab_Log(QWidget *parent = nullptr);
    ~MainTab_Log();

private slots:
    void on_pushButton_clear_clicked();
    void appendText(QString str);

private:
    Ui::MainTab_Log *ui;
};

#endif // TAB_LOG_H
