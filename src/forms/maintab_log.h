#ifndef TAB_LOG_H
#define TAB_LOG_H

#include <QWidget>


namespace Ui {
class MainTab_Log;
}

class MainTab_Log : public QWidget
{
    Q_OBJECT

public:
    explicit MainTab_Log(QWidget *parent = 0);
    ~MainTab_Log();
    void appendText(const char *str);

private slots:
    void on_pushButton_clear_clicked();

private:
    Ui::MainTab_Log *ui;
};

#endif // TAB_LOG_H
