#ifndef BASE_MAPVIEW_H
#define BASE_MAPVIEW_H

#include <QPoint>
#include <QFutureWatcher>
#include <memory>


namespace uocf {
class UOMap;
}
class SubDlg_TaskProgress;

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsItem;
class QGraphicsView;
class QImage;


class Base_MapView : public QObject
{
    Q_OBJECT

public:
    Base_MapView();
    virtual ~Base_MapView();

    void setup(QWidget* parentWidget, QGraphicsView* graphicsView);

private:
    QWidget* m_parentWidget;
    QGraphicsView* m_graphicsView;

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void mouseMoved(QPoint);
    void mouseClicked(QPoint);
    void progressValChanged(int i);

public slots:
    void redrawMap();
    void progressValUpdate(int i);
    void drawingFullDone();
    void scrollBarHorizontalChanged(int val);
    void scrollBarVerticalChanged(int val);


//private:
public:
    bool m_updateDisabled;

    uint m_mapPlane;
    uocf::UOMap* m_selectedMapData;

    bool m_drawFull;
    uint m_scaleFactor;
    double m_zoom;

    QPoint m_selectedMapPoint;
    int m_selectedMapZ;

public:
    /*
    int getMapPlane() const {
        return m_mapPlane;
    }
    bool getDrawFull() const {
        return m_drawFull;
    }
    uint getScaleFactor() const {
        return m_scaleFactor;
    }
    double getZoom() const {
        return m_zoom;
    }
    */
    void setMapPlane(uint mapPlane);
    void setDrawFull(bool drawFull);
    void setScaleFactor(uint scaleFactor);
    void setDeltaZoom(double deltaZoom);

public:
    std::unique_ptr<SubDlg_TaskProgress> m_progressDlg;

    QPoint coordsClipToMap(const QPoint& coords) const;
    QPoint coordsFromViewToMap(const QPoint& graphicsViewCoords) const;
    QPoint coordsFromMapToView(const QPoint& graphicsViewCoords) const;
    bool drawMapReset();
    void drawMap();
    void drawMapFull();
    void drawMapPart(const QPoint& imageOffset, const QRect& rectToDraw);

    bool selectPoint(QPoint pointOnView);
    QGraphicsPixmapItem* drawCursor(const QPoint &coordsOnView, const QColor& color);
    void redrawSelectedPointCursor();

public:
    QGraphicsScene* m_scene;
    QGraphicsItem* m_giMap;
    QGraphicsItem* m_giSelectedPointCursor;

    QImage* m_mapImage;
    // For full & async map render
    QFutureWatcher<bool> m_imgFutureWatcher;
    QFuture<bool> m_imgFuture;
};

#endif // BASE_MAPVIEW_H
