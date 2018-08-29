#include "dlg_huepicker.h"
#include "ui_dlg_huepicker.h"

#include <QImage>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QSignalMapper>
#include <QMessageBox>

#include "../globals.h"
#include "../uoclientfiles/uohues.h"
#include "../uoclientfiles/uoart.h"
#include "../uoclientfiles/uoanim.h"
#include "../spherescript/scriptsearch.h"
#include "../spherescript/scriptutils.h"
#include "../keystrokesender/keystrokesender.h"


Dlg_HuePicker::Dlg_HuePicker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlg_huepicker)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);   // disable the '?' (what's this) in the title bar

    m_selectedHueIndex = -1;
    m_brightnessPercent = 0;
    m_shadeIndex = 25;
    m_previewIsItem = true;
    m_previewDisplayId = 0x1F13;    // worldgem bit large (lg)

    ui->setupUi(this);

    ui->radioButton_item->setChecked(true);
    drawHueBar();

    ui->horizontalSlider_shade->setValue(m_shadeIndex);

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
            labelHueGridElem->setMinimumSize(9,5);
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

    delete m_hueTableMapClick;
    delete m_hueTableMapDoubleClick;
}

void Dlg_HuePicker::onManual_hueTableClicked_mapped(int index)
{
    m_selectedHueIndex = index;
    QString hueText = QString("0") + QString::number(index+1, 16) + " (dec: " + QString::number(index+1, 10) + ")";
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
    if (!g_UOHues)
        return;

    static const int colorBlockWidth = 14;
    static const int colorBlockHeight = 22;

    QImage qimgHueDetail(uocf::UOHueEntry::kColorTableSize * colorBlockWidth, colorBlockHeight, QImage::Format_RGB32);
    if (!g_UOHues || !m_selectedHueIndex)
        qimgHueDetail.fill(0);
    else
    {
        int hueIdx = (m_selectedHueIndex == -1) ? 0 : m_selectedHueIndex;
        uocf::UOHueEntry hueSelected = g_UOHues->getHueEntry(hueIdx);
        for (int i = 0; i < uocf::UOHueEntry::kColorTableSize; ++i)   // color blocks = 32
        {
            uocf::ARGB32 color32 = uocf::ARGB32(hueSelected.getColor(i));
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
    int hueIdx = 0;
    for (EnhancedLabel* curLabel : m_hueTableBlocks)
    {
        uocf::UOHueEntry curHue = g_UOHues->getHueEntry(hueIdx);

        // Get the "mean" color
        /*
        unsigned int meanR, meanG, meanB;
        meanR = meanG = meanB = 0;
        for (int i = 0; i < UOHueEntry::kColorTableSize; ++i)   // color blocks = 32
        {
            uocf::ARGB32 color32 = uocf::ARGB32(curHue.getColor(i));
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
        */

        uocf::ARGB32 color32 = uocf::ARGB32(curHue.getColor(m_shadeIndex));
        color32.adjustBrightness(m_brightnessPercent);
        QImage qimg(1,1,QImage::Format_RGB32);
        qimg.fill(color32.getVal());
        QPixmap qpix = QPixmap::fromImage(qimg);
        curLabel->setPixmap(qpix);

        ++hueIdx;
    }
    setUpdatesEnabled(true);
}

void Dlg_HuePicker::drawPreview()
{
    if (!g_UOArt || !g_UOAnim || !g_UOHues)
        return;

    if (m_previewDisplayId == 0)
    {
        if (ui->graphicsView->scene() != nullptr)
            delete ui->graphicsView->scene();
        return;
    }

    QImage* frameimg = nullptr;
    int hueIdx = (m_selectedHueIndex == -1) ? 0 : m_selectedHueIndex;
    if (m_previewIsItem)
    {
        g_UOArt->setHuesCachePointer(g_UOHues); // reset the right address (in case it has changed) to the hues to be used
        frameimg = g_UOArt->drawArt(uocf::UOArt::kItemsOffset + m_previewDisplayId, hueIdx+1, false);
    }
    else
    {
        g_UOAnim->setHuesCachePointer(g_UOHues); // reset the right address (in case it has changed) to the hues to be used
        frameimg = g_UOAnim->drawAnimFrame(m_previewDisplayId, 0, 1, 0, hueIdx+1);
    }
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
    if (value != m_brightnessPercent)
    {
        m_brightnessPercent = value * 5;
        drawHueTable();
    }
}

void Dlg_HuePicker::on_horizontalSlider_shade_valueChanged(int value)
{
    if (value != m_shadeIndex)
    {
        m_shadeIndex = value;
        drawHueTable();
    }
}

void Dlg_HuePicker::on_pushButton_set_clicked()
{
    std::string strToSend = ".xcolor " + std::to_string(m_selectedHueIndex + 1);
    auto ksResult = ks::KeystrokeSender::sendStringFastAsync(strToSend, true, g_sendKeystrokeAndFocusClient);
    if (ksResult != ks::KSERR_OK)
    {
        QMessageBox errorDlg(QMessageBox::Warning, "Warning", ks::getErrorStringStatic(ksResult), QMessageBox::NoButton, this);
        errorDlg.exec();
    }
}

void Dlg_HuePicker::on_pushButton_setClose_clicked()
{
    std::string strToSend = ".xcolor " + std::to_string(m_selectedHueIndex);
    auto ksResult = ks::KeystrokeSender::sendStringFastAsync(strToSend, true, g_sendKeystrokeAndFocusClient);
    if (ksResult != ks::KSERR_OK)
    {
        QMessageBox errorDlg(QMessageBox::Warning, "Warning", ks::getErrorStringStatic(ksResult), QMessageBox::NoButton, this);
        errorDlg.exec();
    }
    else
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

