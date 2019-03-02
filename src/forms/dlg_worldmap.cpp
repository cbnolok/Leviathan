#include "dlg_worldmap.h"
#include "ui_dlg_worldmap.h"

#include <QTimer>
#include <QMessageBox>

#include "../keystrokesender/keystrokesender.h"
#include "../globals.h"


Dlg_WorldMap::Dlg_WorldMap(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_WorldMap)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);   // disable the '?' (what's this) in the title bar
    ui->setupUi(this);

    m_mapViewer.setup(this, ui->graphicsView_map);
    connect(&m_mapViewer, SIGNAL(mouseMoved(QPoint)), this, SLOT(mouseMove(QPoint)));
    connect(&m_mapViewer, SIGNAL(mouseClicked(QPoint)), this, SLOT(mouseClick(QPoint)));
    m_mapViewer.m_scaleFactor = uint(ui->horizontalSlider_scale->value());
    m_mapViewer.m_mapPlane = ui->spinBox_map->value();
    m_mapViewer.m_drawFull = (ui->checkBox_preRender->checkState() != Qt::CheckState::Unchecked);

    // draw image
    QTimer::singleShot(50, &m_mapViewer, SLOT(redrawMap()));
}

Dlg_WorldMap::~Dlg_WorldMap()
{
    delete ui;
}


void Dlg_WorldMap::mouseMove(QPoint mousePoint)
{
    mousePoint = m_mapViewer.coordsFromViewToMap(mousePoint);
    ui->label_coords_mouse_data->setText(QString("X: %1, Y: %2").arg(mousePoint.x()).arg(mousePoint.y()));
}

void Dlg_WorldMap::mouseClick(QPoint mousePoint)
{
    if (!m_mapViewer.selectPoint(mousePoint))
        return;

    int x = m_mapViewer.m_selectedMapPoint.x(), y = m_mapViewer.m_selectedMapPoint.y(), z = m_mapViewer.m_selectedMapZ;
    ui->label_coords_selected_data->setText(QString("X: %1, Y: %2, Z: %3").arg(x).arg(y).arg(z));
}


void Dlg_WorldMap::on_horizontalSlider_scale_valueChanged(int value)
{
    m_mapViewer.setScaleFactor(uint(value));
}

void Dlg_WorldMap::on_horizontalSlider_zoom_sliderMoved(int position)
{
    double nextZoom = (position - ui->horizontalSlider_zoom->value()) * 1.6;
    if (nextZoom < 0)
        nextZoom = (-1)/nextZoom;
    m_mapViewer.setDeltaZoom(nextZoom);
}

void Dlg_WorldMap::on_spinBox_map_valueChanged(int arg1)
{
    m_mapViewer.setMapPlane(arg1);
}

void Dlg_WorldMap::on_checkBox_preRender_stateChanged(int arg1)
{
    bool preRender = (arg1 != Qt::CheckState::Unchecked);

    m_mapViewer.setDrawFull(preRender);
    ui->horizontalSlider_zoom->setEnabled(preRender);
}


void Dlg_WorldMap::on_pushButton_go_clicked()
{
    int x = m_mapViewer.m_selectedMapPoint.x(), y = m_mapViewer.m_selectedMapPoint.y();
    int z = m_mapViewer.m_selectedMapZ, m = m_mapViewer.m_mapPlane;
    QString qstrToSend = QString(".go %1 %2 %3 %4").arg(x).arg(y).arg(z).arg(m);

    auto ksResult = ks::KeystrokeSender::sendStringFastAsync(qstrToSend.toStdString(), true, g_sendKeystrokeAndFocusClient);
    if (ksResult != ks::KSError::Ok)
    {
        QMessageBox errorDlg(QMessageBox::Warning, "Warning", ks::getErrorStringStatic(ksResult), QMessageBox::NoButton, this);
        errorDlg.exec();
    }
}

