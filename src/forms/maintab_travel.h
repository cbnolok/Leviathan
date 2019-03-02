#ifndef MAINTAB_TRAVEL_H
#define MAINTAB_TRAVEL_H

#include <QWidget>

namespace Ui {
class MainTab_Travel;
}

class MainTab_Travel : public QWidget
{
    Q_OBJECT

public:
    explicit MainTab_Travel(QWidget *parent = nullptr);
    ~MainTab_Travel();

private:
    Ui::MainTab_Travel *ui;
};

/*
class TGs : public QGraphicsScene
{
public:
    TGs(QObject *parent = 0);

public slots:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    bool over;
    QString str;
    QGraphicsTextItem cursor;
};
*/

#endif // MAINTAB_TRAVEL_H
