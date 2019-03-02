#ifndef DLG_WORLDMAP_H
#define DLG_WORLDMAP_H

#include <QDialog>
#include "base_mapview.h"

namespace Ui {
class Dlg_WorldMap;
}


class Dlg_WorldMap : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_WorldMap(QWidget *parent = nullptr);
    ~Dlg_WorldMap();

private slots:
    void on_horizontalSlider_scale_valueChanged(int value);
    void on_horizontalSlider_zoom_sliderMoved(int position);
    void on_spinBox_map_valueChanged(int arg1);
    void on_checkBox_preRender_stateChanged(int arg1);
    void on_pushButton_go_clicked();

    void mouseMove(QPoint mousePoint);
    void mouseClick(QPoint mousePoint);

private:
    Ui::Dlg_WorldMap *ui;
    Base_MapView m_mapViewer;
};

#endif // DLG_WORLDMAP_H
