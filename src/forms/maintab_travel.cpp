#include "maintab_travel.h"
#include "ui_maintab_travel.h"

MainTab_Travel::MainTab_Travel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainTab_Travel)
{
    ui->setupUi(this);
}

MainTab_Travel::~MainTab_Travel()
{
    delete ui;
}

/*
int viewWidth  = ui->graphicsView_map->geometry().width();
int viewHeight = ui->graphicsView_map->geometry().height();
int rectWidth  = int(viewWidth * (1 / m_zoom));
int rectHeight = int(viewHeight * (1 / m_zoom));

int tileX0, tileX;
tileX0 = m_centerX - (rectWidth / 2);
if (tileX0 < 0)
{
    tileX0 = 0;
    tileX = rectWidth;
}
else
{
    tileX = m_centerX + (rectWidth / 2);
}
if (tileX > mapWidth)
    tileX = mapWidth;

int tileY0, tileY;
tileY0 = m_centerY - (rectHeight / 2);
if (tileY0 < 0)
{
    tileY0 = 0;
    tileY = rectHeight;
}
else
{
    tileY = m_centerY + (rectHeight / 2);
}
if (tileY > mapHeight)
    tileY = mapHeight;

//m_mapImage = m_mapData->drawRect(true,
//                                    unsigned(tileX0), unsigned(tileY0), unsigned(tileX - tileX0), unsigned(tileY - tileY0),
//                                    m_scaleFactor);
*/


/*
TGs::TGs(QObject *parent)
    :QGraphicsScene(parent)
{ // Constructor of Scene
    this->over = false;
}

void TGs::drawBackground(QPainter *painter, const QRectF &rect)
{

#define adjy 30
#define adjx 30

    int j = 0;
    int alto = 0;

    QPen pen;
    pen.setWidth(1);
    pen.setBrush(Qt::lightGray);
    painter->setPen(pen);

    painter->drawText(-225, 10, this->str);
    alto = _alto;  // 50 + 2

    for(int i = 0; i < alto; ++i)
    {
        j = i * adjy - 17;

        painter->drawLine(QPoint(-210, j), QPoint(_ancho, j));
    }

    for(int i = 0; i < 300; ++i)
    {
        j = i * adjx - 210;

        painter->drawLine(QPoint(j, 0), QPoint(j, _ancho * 2));
    }
}

void TGs::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QString string = QString("%1, %2")
               .arg(mouseEvent->scenePos().x())
              .arg(mouseEvent->scenePos().y()); // Update the cursor position text
    this->str = string;
    this->update();
}

void TGs::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    this->update();
}

void TGs::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    this->update();
}
*/



/*
void GraphicsSceneCross::drawForeground(QPainter* painter, const QRectF& rect) {
    QRectF SceneRect = this->sceneRect();

    painter->setPen(QPen(Qt::black, 1));
    painter->drawLine(SceneRect.left(), m_MousePos.y(), SceneRect.right(), m_MousePos.y());
    painter->drawLine(m_MousePos.x(), SceneRect.top(), m_MousePos.x(), SceneRect.bottom());
}
*/
