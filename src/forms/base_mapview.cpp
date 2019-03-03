#include "base_mapview.h"

#include <QtConcurrent/QtConcurrent>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QScrollBar>
#include <QImage>

//#include <chrono>
//#include <QDebug>

#include "../uoclientfiles/uomap.h"
#include "../uoclientfiles/uoradarcol.h"
#include "../uoclientfiles/uostatics.h"
#include "../globals.h"
#include "subdlg_taskprogress.h"


static const Qt::CursorShape kMapViewCursorShapeDefault = Qt::ArrowCursor;

Base_MapView::Base_MapView() :
    m_imgFutureWatcher(this)
{
    m_parentWidget = nullptr;
    m_graphicsView = nullptr;


    m_updateDisabled = true;

    m_mapPlane = 0;
    m_selectedMapData = nullptr;
    m_drawFull = false;
    m_scaleFactor = 0;
    m_zoom = 1.0;
    m_selectedMapZ = 0;

    m_scene = nullptr;
    m_giMap = nullptr;
    m_giSelectedPointCursor = nullptr;
    m_mapImage = nullptr;
}

void Base_MapView::setup(QWidget* parentWidget, QGraphicsView *graphicsView)
{
    m_parentWidget = parentWidget;
    m_graphicsView = graphicsView;

    m_progressDlg = std::make_unique<SubDlg_TaskProgress>();
    m_progressDlg->setProgressMax(100);
    connect(this, SIGNAL(progressValChanged(int)), this, SLOT(progressValUpdate(int)));

    connect(&m_imgFutureWatcher, SIGNAL(finished()), this, SLOT(drawingFullDone()));

    // GraphicsView
    //  Set default cursor
    m_graphicsView->viewport()->setCursor(kMapViewCursorShapeDefault);
    //  Set event filter for mouse tracking and resize events)
    m_graphicsView->setMouseTracking(true);
    m_graphicsView->installEventFilter(this);
    //  Track scrollbars movement
    connect(m_graphicsView->horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollBarHorizontalChanged(int)));
    connect(m_graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollBarVerticalChanged(int)));
}

Base_MapView::~Base_MapView()
{
    //m_imgFutureWatcher.cancel(); // doesn't work for QtConcurrent::run
    m_imgFutureWatcher.waitForFinished(); // otherwise we'll crash, since the qimage class member is being written by the other thread

    if (m_scene)
        delete m_scene;
}


bool Base_MapView::eventFilter(QObject* watched, QEvent* event)
{
    // This filter is used for both GraphicsView and the Scene */
    const QEvent::Type evtype = event->type();


    /* Events sent only to the scene */
    // the scene has the size of the map, scaled by the scalefactor
    if (evtype == QEvent::GraphicsSceneMouseMove)
    {
        QGraphicsSceneMouseEvent *mev = static_cast<QGraphicsSceneMouseEvent*>(event);
        emit mouseMoved({int(mev->scenePos().x()), int(mev->scenePos().y())});
    }
    else if (evtype == QEvent::GraphicsSceneMousePress)
    {
        QGraphicsSceneMouseEvent *mev = static_cast<QGraphicsSceneMouseEvent*>(event);
        emit mouseClicked({int(mev->scenePos().x()), int(mev->scenePos().y())});
    }
    else if (evtype == QEvent::GraphicsSceneMouseRelease)
    {
        //QGraphicsScene *gs = static_cast<QGraphicsScene*>(watched);
        //QGraphicsView *gv = gs->views().first();
        m_graphicsView->viewport()->setCursor(kMapViewCursorShapeDefault);
    }

    /* Events sent only to the view */
    else if (evtype == QEvent::Resize)
    {
        // resize event: redraw?
        // check m_updateDisabled, since this event is called also at the dialog first setup/show, so avoid redrawing the map twice
        if (!m_updateDisabled && !m_drawFull)
        {
            if (m_selectedMapData)
            {
                const QRect viewGeometry = m_graphicsView->geometry();
                const QPoint mapSizeScaled = coordsFromMapToView( QPoint(int(m_selectedMapData->getWidth()), int(m_selectedMapData->getHeight())) );
                if ((viewGeometry.width() < mapSizeScaled.x()) || (viewGeometry.height() < mapSizeScaled.y()))
                    redrawMap();
            }
        }
    }

    return QObject::eventFilter(watched, event);
}


void Base_MapView::setMapPlane(uint mapPlane)
{
    m_mapPlane = mapPlane;
    m_selectedMapPoint = QPoint();
    if (m_giSelectedPointCursor)
    {
        delete m_giSelectedPointCursor;
        m_giSelectedPointCursor = nullptr;
    }

    if (m_selectedMapData)
        m_selectedMapData->freeDataCache();

    redrawMap();
}

void Base_MapView::setDrawFull(bool drawFull)
{
    m_drawFull = drawFull;

    if (!m_drawFull)
        m_graphicsView->scale(1/m_zoom, 1/m_zoom);
    else
        m_graphicsView->scale(m_zoom, m_zoom);

    redrawMap();
}

void Base_MapView::setScaleFactor(uint scaleFactor)
{
    m_scaleFactor = scaleFactor;
    redrawMap();
}

void Base_MapView::setDeltaZoom(double deltaZoom)
{
    m_zoom *= deltaZoom;
    /*
    if (!m_drawFull)
    {
        const QRectF sceneRect = m_scene->sceneRect();
        m_scene->setSceneRect(QRectF(0, 0, int(sceneRect.width() / deltaZoom), int(sceneRect.height() / deltaZoom)));
    }
    */
    m_graphicsView->scale(deltaZoom, deltaZoom);
}


QPoint Base_MapView::coordsClipToMap(const QPoint& coords) const
{
    int x = (coords.x() < 0) ? 0 : coords.x();
    int y = (coords.y() < 0) ? 0 : coords.y();
    if (m_selectedMapData)
    {
        const int w = int(m_selectedMapData->getWidth()), h = int(m_selectedMapData->getHeight());
        if (x > w)
            x = w;
        if (y > h)
            y = h;
    }
    return {x, y};
}

QPoint Base_MapView::coordsFromViewToMap(const QPoint& graphicsViewCoords) const
{
    unsigned x = (graphicsViewCoords.x() < 0) ? 0 : unsigned(graphicsViewCoords.x());
    unsigned y = (graphicsViewCoords.y() < 0) ? 0 : unsigned(graphicsViewCoords.y());
    uocf::UOMap::scaleCoordsImageToMap(m_scaleFactor, &x, &y);
    return coordsClipToMap(QPoint(int(x), int(y)));
}

QPoint Base_MapView::coordsFromMapToView(const QPoint& graphicsViewCoords) const
{
    unsigned x = (graphicsViewCoords.x() < 0) ? 0 : unsigned(graphicsViewCoords.x());
    unsigned y = (graphicsViewCoords.y() < 0) ? 0 : unsigned(graphicsViewCoords.y());
    uocf::UOMap::scaleCoordsMapToImage(m_scaleFactor, &x, &y);
    return {int(x), int(y)};
}


void Base_MapView::redrawMap()
{
    m_updateDisabled = false;
    if (!drawMapReset())
        return;

    if (m_drawFull)
        drawMapFull();
    else
    {
        const QRect& viewGeometry = m_graphicsView->geometry();
        const QPoint& viewSizeScaled = coordsFromViewToMap({viewGeometry.width(), viewGeometry.height()});
        const QRect rectToDraw(0, 0, viewSizeScaled.x(), viewSizeScaled.y());
        drawMapPart(QPoint(0,0), rectToDraw);
    }
}

bool Base_MapView::drawMapReset()
{
    m_selectedMapData = g_UOMaps.empty() ? nullptr : g_UOMaps[m_mapPlane];
    if (!m_selectedMapData)
        return false;
    uocf::UOStatics *selectedStaticsData = g_UOStatics.empty() ? nullptr : g_UOStatics[m_mapPlane];
    m_selectedMapData->setCachePointers(g_UORadarCol, selectedStaticsData, g_UOHues);

    if (m_mapImage)
        delete m_mapImage;
    m_mapImage = nullptr;

    if (m_giSelectedPointCursor && m_scene && (m_giSelectedPointCursor->scene() == m_scene))
        m_scene->removeItem(m_giSelectedPointCursor);
    if (m_scene != nullptr)
        delete m_scene;
    m_scene = new QGraphicsScene();
    m_scene->installEventFilter(this);
    m_graphicsView->setScene(m_scene);

    // Deleted by the scene destructor
    m_giMap = nullptr;

    unsigned mapWidthScaled = m_selectedMapData->getWidth(), mapHeightScaled = m_selectedMapData->getHeight();
    uocf::UOMap::scaleCoordsMapToImage(m_scaleFactor, &mapWidthScaled, &mapHeightScaled);

    m_mapImage = new QImage(int(mapWidthScaled), int(mapHeightScaled), QImage::Format_RGB32);
    m_mapImage->fill(uocf::UOMap::kUninitializedRGB);

    if (!m_drawFull)
    {
        m_scene->setSceneRect(QRectF(0, 0, mapWidthScaled, mapHeightScaled));
        //m_scene->setSceneRect(QRectF(0, 0, int(mapWidthScaled / m_zoom), int(mapHeightScaled / m_zoom)));
        /*
        QScrollBar *hScroll = ui->graphicsView_map->horizontalScrollBar();
        QScrollBar *vScroll = ui->graphicsView_map->verticalScrollBar();
        hScroll->setMaximum(int(mapWidthScaled));
        vScroll->setMaximum(int(mapHeightScaled));
        */
    }

    return true;
}


void Base_MapView::progressValUpdate(int i)
{
    m_progressDlg->setProgressVal(i);
}

void Base_MapView::drawMapFull()
{
    if (m_imgFutureWatcher.isRunning())
        return;

    if (m_mapPlane > uocf::UOMap::kMaxSupportedMap)
        return;
    if (uint(m_mapPlane + 1) > g_UOMaps.size())
        return;

    if (!m_selectedMapData)
        return;

    m_progressDlg->setParent(m_parentWidget->window());
    m_progressDlg->move(m_parentWidget->window()->rect().center() - m_progressDlg->rect().center());
    m_progressDlg->setProgressVal(0);
    m_progressDlg->show();

    // Loading stuff
    m_progressDlg->setLabelText("Rendering...");
    auto emitUpdateSignal = [this](int i)
    {
        emit progressValChanged(i);
    };

    auto render = [=]() -> bool
    {
      return m_selectedMapData->drawRectInImage(m_mapImage, 0, 0,
                                              emitUpdateSignal,
                                              0, 0, m_selectedMapData->getWidth(), m_selectedMapData->getHeight(),
                                              m_scaleFactor, true);
    };

    m_parentWidget->setEnabled(false);
    m_imgFuture = QtConcurrent::run(render);
    m_imgFutureWatcher.setFuture(m_imgFuture);
}

void Base_MapView::drawingFullDone()
{
    m_progressDlg->close();
    m_parentWidget->setEnabled(true);

    if (!m_selectedMapData)
        return;
    m_selectedMapData->freeDataCache();

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(*m_mapImage));

    m_scene->addItem(item);
    if (m_giMap)
    {
        if (m_giMap->scene() == m_scene)
            m_scene->removeItem(m_giMap);
        delete m_giMap;
    }
    m_giMap = item;

    redrawSelectedPointCursor();
}


void Base_MapView::scrollBarHorizontalChanged(int val)
{
    if (m_drawFull || !m_mapImage)
        return;

    const QScrollBar *vScroll = m_graphicsView->verticalScrollBar();
    const QPoint imageOffset(val, vScroll->value());
    const QPoint imageOffsetScaledToMap = coordsFromViewToMap(imageOffset);

    const QRect& viewGeometry = m_graphicsView->geometry();
    //const QPoint viewSizeScaledToMap = coordsFromViewToMap({int(zoom * viewGeometry.width()), int(zoom * viewGeometry.height())});
    const QPoint viewSizeScaledToMap = coordsFromViewToMap({viewGeometry.width(), viewGeometry.height()});

    const QRect rectToDraw(imageOffsetScaledToMap.x(), imageOffsetScaledToMap.y(), viewSizeScaledToMap.x(), viewSizeScaledToMap.y());
    drawMapPart(imageOffset, rectToDraw);
}

void Base_MapView::scrollBarVerticalChanged(int val)
{
    if (m_drawFull || !m_mapImage)
        return;

    const QScrollBar *hScroll = m_graphicsView->horizontalScrollBar();
    const QPoint imageOffset(hScroll->value(), val);
    const QPoint imageOffsetScaledToMap = coordsFromViewToMap(imageOffset);

    const QRect& viewGeometry = m_graphicsView->geometry();
    //const QPoint viewSizeScaledToMap = coordsFromViewToMap({int(zoom * viewGeometry.width()), int(zoom * viewGeometry.height())});
    const QPoint viewSizeScaledToMap = coordsFromViewToMap({viewGeometry.width(), viewGeometry.height()});

    const QRect rectToDraw(imageOffsetScaledToMap.x(), imageOffsetScaledToMap.y(), viewSizeScaledToMap.x(), viewSizeScaledToMap.y());
    drawMapPart(imageOffset, rectToDraw);
}

void Base_MapView::drawMapPart(const QPoint &imageOffset, const QRect &rectToDraw)
{
    if (m_mapPlane > uocf::UOMap::kMaxSupportedMap)
        return;
    if (uint(m_mapPlane + 1) > g_UOMaps.size())
        return;

    if (!m_selectedMapData)
        return;

    //static qulonglong rendercnt = 0;
    //auto tStart = std::chrono::steady_clock::now();
    unsigned int xRenderStart = unsigned(rectToDraw.x()), yRenderStart = unsigned(rectToDraw.y());
    unsigned int renderWidth = unsigned(rectToDraw.width()), renderHeight = unsigned(rectToDraw.height());
    m_selectedMapData->clipCoordsToMapSize(&xRenderStart, &yRenderStart, &renderWidth, &renderHeight);
    m_selectedMapData->drawRectInImage(m_mapImage, imageOffset.x(), imageOffset.y(),
                                     nullptr,
                                     xRenderStart, yRenderStart, renderWidth, renderHeight,
                                     m_scaleFactor, true);
    //auto tEnd = std::chrono::steady_clock::now();
    //qDebug() << "rendermap" << ++rendercnt << "-> ms" << std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();

    const QPoint pixmapSize = coordsFromMapToView(QPoint(int(renderWidth), int(renderHeight)));
    QPixmap pix(pixmapSize.x(), pixmapSize.y());
    //pix.fill(Qt::transparent); // fill with something, if we create a larger than needed pixmap
    pix.fill(Qt::white); // transparent may be slower than white (which has no alpha?)
    const QPoint copyDestOffset(0, 0);
    const QRect copySourceRect(imageOffset.x(), imageOffset.y(), pixmapSize.x(), pixmapSize.y());
    QPainter painter(&pix);
    painter.drawImage(copyDestOffset, *m_mapImage, copySourceRect);
    painter.end();
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(pix);
    //item->setPos(imageOffset / m_zoom);
    item->setPos(imageOffset);

    m_graphicsView->setDisabled(true);

    m_scene->addItem(item);
    if (m_giMap)
    {
        if (m_giMap->scene() == m_scene)
            m_scene->removeItem(m_giMap);
        delete m_giMap;
    }
    m_giMap = item;

    redrawSelectedPointCursor();

    m_graphicsView->setDisabled(false);
}


bool Base_MapView::selectPoint(QPoint pointOnView)
{
    if (!m_selectedMapData)
        return false;

    pointOnView = coordsClipToMap(pointOnView);
    if (pointOnView.isNull())
        return false;

    QColor color = Qt::red;
    /*
    if (m_mapImage)
    {
         QRgb rgb = m_mapImage->pixel(mousePoint);
         color = qRgb(255 - qRed(rgb), 255 - qGreen(rgb), 255 - qBlue(rgb)); // negative color
    }
    */
    if (m_giSelectedPointCursor)
        delete m_giSelectedPointCursor;
    m_giSelectedPointCursor = drawCursor(pointOnView, color);

    m_selectedMapPoint = coordsFromViewToMap(pointOnView);
    m_selectedMapZ = m_selectedMapData->getTopZAtXY(unsigned(m_selectedMapPoint.x()), unsigned(m_selectedMapPoint.y()));
    return true;
}

QGraphicsPixmapItem* Base_MapView::drawCursor(const QPoint& coordsOnView, const QColor &color)
{
    if (coordsOnView.isNull() || !m_mapImage)
        return nullptr;

    const QSize cursorSize(14, 14);
    QPixmap pix(cursorSize);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setPen(QPen(color, 2));
    painter.drawLine(0, (cursorSize.height() / 2), cursorSize.width(), (cursorSize.height() / 2)); // horiz line
    painter.drawLine((cursorSize.width() / 2), 0, (cursorSize.width() / 2), cursorSize.height());  // vert line
    painter.end();

    QGraphicsPixmapItem* pixItem = new QGraphicsPixmapItem(pix);
    pixItem->setOffset(-cursorSize.width()/2, -cursorSize.height()/2);
    pixItem->setPos(coordsOnView);
    pixItem->setZValue(1); // to show on top even after redrawings of the map
    m_scene->addItem(pixItem);

    return pixItem;
}

void Base_MapView::redrawSelectedPointCursor()
{
    if (m_giSelectedPointCursor)
    {
        m_giSelectedPointCursor->setPos(coordsFromMapToView(m_selectedMapPoint));
        if (m_giSelectedPointCursor->scene() != m_scene)
            m_scene->addItem(m_giSelectedPointCursor);
    }
}

