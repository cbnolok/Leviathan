#include "dlg_huepicker.h"
#include "ui_dlg_huepicker.h"
#include "../globals.h"
#include "../uofiles/uohues.h"
#include "../uofiles/uoart.h"
#include "../uofiles/uoanim.h"
#include "../spherescript/scriptsearch.h"
#include "../spherescript/scriptutils.h"
#include "../keystrokesender/keystrokesender.h"
#include <QImage>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QSignalMapper>


const int colorsPerHue = 32;      // how much colors does a hue entry contain


Dlg_HuePicker::Dlg_HuePicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_huepicker)
{
    m_selectedHueIndex = 0;
    m_brightnessPercent = 0;
    m_previewIsItem = true;
    m_previewDisplayId = 0x1F13;    // worldgem bit large (lg)

    ui->setupUi(this);

    ui->radioButton_item->setChecked(true);
    drawHueBar();

    // Set up the hue table - we use a QLabel for each hue in the table, and set its color by creating a QPixmap

    static const int hueTableBlocksH = 50;
    static const int hueTableBlocksV = 60;
    m_hueTableBlocks.reserve(hueTableBlocksH*hueTableBlocksV);

    // default image for the grid element
    QImage qimg(1,1,QImage::Format_RGB32);
    qimg.fill(qRgb(0,0,0));
    QPixmap qpix = QPixmap::fromImage(qimg);

    m_hueTableMapClick = new QSignalMapper();
    m_hueTableMapDoubleClick = new QSignalMapper();

    setUpdatesEnabled(false);
    int hueIdx = 0;
    for (int x = 0; x < hueTableBlocksH; ++x)
    {
        for (int y = 0; y < hueTableBlocksV; ++y)
        {
            // build each grid element
            EnhancedLabel* labelHueGridElem = new EnhancedLabel(this);
            labelHueGridElem->setMinimumSize(10,6);
            labelHueGridElem->setScaledContents(true);
            m_hueTableBlocks.push_back(labelHueGridElem);

            // connect the signal to track the click and double click events to each one
            connect(labelHueGridElem, SIGNAL(clicked()), m_hueTableMapClick, SLOT(map()));
            m_hueTableMapClick->setMapping(labelHueGridElem, hueIdx);
            connect(labelHueGridElem, SIGNAL(doubleclicked()), m_hueTableMapDoubleClick, SLOT(map()));
            m_hueTableMapDoubleClick->setMapping(labelHueGridElem, hueIdx);

            // set the color
            labelHueGridElem->setPixmap(qpix);

            // add dynamically our label to our layout
            ui->gridLayout_hueTable->addWidget(labelHueGridElem, y, x, 1, 1);
            ++hueIdx;
        }
    }
    // connect the mapper
    connect(m_hueTableMapClick, SIGNAL(mapped(int)), this, SLOT(onManual_hueTableClicked_mapped(int)));
    connect(m_hueTableMapDoubleClick, SIGNAL(mapped(int)), this, SLOT(onManual_hueTableDoubleClicked_mapped(int)));

    drawHueTable();
    drawPreview();
    setUpdatesEnabled(true);
}

Dlg_HuePicker::~Dlg_HuePicker()
{
    delete ui;
}

void Dlg_HuePicker::onManual_hueTableClicked_mapped(int index)
{
    m_selectedHueIndex = index + 1;
    QString hueText = QString("0") + QString::number(index, 16) + " (dec: " + QString::number(index, 10) + ")";
    ui->label_hueIdx->setText(hueText);
    QString hueName(g_UOHues->getHueEntry(index).getName().c_str());
    ui->label_hueName->setText(hueName);
    drawHueBar();
    drawPreview();
}

void Dlg_HuePicker::onManual_hueTableDoubleClicked_mapped(int index)
{
    onManual_hueTableClicked_mapped(index);
    on_pushButton_set_clicked();
}

void Dlg_HuePicker::drawHueBar()
{
    static const int colorBlockWidth = 14;
    static const int colorBlockHeight = 22;

    QImage qimgHueDetail(colorsPerHue * colorBlockWidth, colorBlockHeight, QImage::Format_RGB32);
    if (!g_UOHues || !m_selectedHueIndex)
        qimgHueDetail.fill(0);
    else
    {
        UOHueEntry hueSelected = g_UOHues->getHueEntry(m_selectedHueIndex);
        for (int i = 0; i < colorsPerHue; ++i)   // color blocks = 32
        {
            ARGB32 color32 = ARGB32(hueSelected.getColor(i));
            color32.adjustBrightness(m_brightnessPercent);
            uint rawcolor = color32.getVal();
            for (int x = (colorBlockWidth * i); x < (colorBlockWidth * i) + colorBlockWidth; ++x)
            {
                for (int y = 0; y < colorBlockHeight; ++y)
                    qimgHueDetail.setPixel(x, y, rawcolor);
            }
        }
    }
    QPixmap qpixHueDetail = QPixmap::fromImage(qimgHueDetail);
    ui->label_hueDisplay->setPixmap(qpixHueDetail);
}

void Dlg_HuePicker::drawHueTable()
{
    if (!g_UOHues)
        return;

    setUpdatesEnabled(false);
    int hueIdx = 1;
    for (QLabel* curLabel : m_hueTableBlocks)
    {
        UOHueEntry curHue = g_UOHues->getHueEntry(hueIdx);
        unsigned int meanR, meanG, meanB;
        meanR = meanG = meanB = 0;
        for (int i = 0; i < colorsPerHue; ++i)   // color blocks = 32
        {
            ARGB32 color32 = ARGB32(curHue.getColor(i));
            color32.adjustBrightness(m_brightnessPercent);
            meanR += color32.getR();
            meanG += color32.getG();
            meanB += color32.getB();
        }
        meanR /= 32;
        meanG /= 32;
        meanB /= 32;

        QImage qimg(1,1,QImage::Format_RGB32);
        qimg.fill(qRgb(meanR,meanG,meanB));
        QPixmap qpix = QPixmap::fromImage(qimg);
        curLabel->setPixmap(qpix);

        ++hueIdx;
    }
    setUpdatesEnabled(true);
}

void Dlg_HuePicker::drawPreview()
{
    if (m_previewDisplayId == 0)
    {
        if (ui->graphicsView->scene() != nullptr)
            delete ui->graphicsView->scene();
        return;
    }

    QImage* frameimg = nullptr;
    if (m_previewIsItem)
        frameimg = g_UOArt->drawArt(UOArt::kItemsOffset + m_previewDisplayId, m_selectedHueIndex, false);
    else
        frameimg = g_UOAnim->drawAnimFrame(m_previewDisplayId, 0, 1, 0, m_selectedHueIndex);
    if (frameimg == nullptr)
        return;

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(*frameimg));
    delete frameimg;

    if (ui->graphicsView->scene() != nullptr)
        delete ui->graphicsView->scene();
    QGraphicsScene* scene = new QGraphicsScene();
    ui->graphicsView->setScene(scene);

    scene->addItem(item);
}

void Dlg_HuePicker::on_horizontalSlider_brightness_valueChanged(int value)
{
    if ((value != m_brightnessPercent))
    {
        m_brightnessPercent = value * 5;
        drawHueTable();
    }
}

void Dlg_HuePicker::on_pushButton_set_clicked()
{
    ks::KeystrokeSender::sendStringFastAsync(std::string(".xcolor ") + std::to_string(m_selectedHueIndex), true, true);
}

void Dlg_HuePicker::on_pushButton_setClose_clicked()
{
    ks::KeystrokeSender::sendStringFastAsync(std::string(".xcolor ") + std::to_string(m_selectedHueIndex), true, true);
    this->close();
}

void Dlg_HuePicker::on_radioButton_item_toggled(bool checked)
{
    if (checked)
        m_previewIsItem = true;
}

void Dlg_HuePicker::on_radioButton_npc_toggled(bool checked)
{
    if (checked)
        m_previewIsItem = false;
}

void Dlg_HuePicker::on_lineEdit_preview_returnPressed()
{
    if (ui->lineEdit_preview->text().isEmpty())
        return;

    ScriptSearch::SearchData_t sd;
    sd.caseSensitive = false;
    sd.key = ui->lineEdit_preview->text().toStdString();
    if (ScriptUtils::strToSphereInt(sd.key) == -1)
        sd.searchBy = ScriptSearch::SearchBy_t::Defname;
    else
        sd.searchBy = ScriptSearch::SearchBy_t::ID;
    sd.initialized = true;

    std::vector<ScriptObjTree*> treesToSearchInto;
    treesToSearchInto.push_back( m_previewIsItem ? getScriptObjTree(SCRIPTOBJ_TYPE_ITEM) : getScriptObjTree(SCRIPTOBJ_TYPE_CHAR) );
    ScriptSearch searchInstance(treesToSearchInto, sd);
    ScriptObj* previewObj = searchInstance.next();
    if (previewObj)
    {
        m_previewDisplayId = previewObj->m_display;
        drawPreview();
    }
    else
        QApplication::beep();
}



/* Enhanced Label implementation */

EnhancedLabel::EnhancedLabel(QWidget* parent, Qt::WindowFlags /*f*/)
    : QLabel(parent)
{
}

EnhancedLabel::~EnhancedLabel()
{
}

void EnhancedLabel::mousePressEvent(QMouseEvent* /*event*/)
{
    emit clicked();
}

void EnhancedLabel::mouseDoubleClickEvent(QMouseEvent* /*event*/)
{
    emit doubleclicked();
}

/*
void EnhancedLabel::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.drawRoundedRect(0,5,width()-5, height()-7,3,3);
    QWidget::paintEvent(e);
}
*/


