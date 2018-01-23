#ifndef DLG_HUEPICKER_H
#define DLG_HUEPICKER_H

#include <QDialog>
#include <QLabel>
#include <vector>


// We need to add to QLabel some functionalities
class EnhancedLabel : public QLabel
{
    Q_OBJECT

public:
    explicit EnhancedLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~EnhancedLabel();
    //int m_index;

signals:
    void clicked();
    void doubleclicked();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    //void paintEvent(QPaintEvent *e);
};



class ScriptObj;
class QSignalMapper;

namespace Ui {
class dlg_huepicker;
}

class Dlg_HuePicker : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_HuePicker(QWidget *parent = 0);
    ~Dlg_HuePicker();

private slots:
    void on_horizontalSlider_brightness_valueChanged(int value);
    void on_pushButton_set_clicked();
    void on_pushButton_setClose_clicked();
    void on_radioButton_item_toggled(bool checked);
    void on_radioButton_npc_toggled(bool checked);
    void on_lineEdit_preview_returnPressed();
    void onManual_hueTableClicked_mapped(int);
    void onManual_hueTableDoubleClicked_mapped(int);

private:
    Ui::dlg_huepicker *ui;

    int m_selectedHueIndex;
    int m_brightnessPercent;
    bool m_previewIsItem;
    int m_previewDisplayId;
    std::vector<EnhancedLabel*> m_hueTableBlocks;
    QSignalMapper* m_hueTableMapClick;
    QSignalMapper* m_hueTableMapDoubleClick;

    void drawHueBar();
    void drawHueTable();
    void drawPreview();
};

#endif // DLG_HUEPICKER_H
