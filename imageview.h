#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QtWidgets>
#include <QWidget>
#include <QMouseEvent>
#include <QStatusBar>
class imageview : public QGraphicsView
{

public:
    imageview(QWidget* parent = 0);
    void viewFit();
    void rectFit();
    void zoom_in();
    void zoom_out();
    void get_area();

    void new_choose(); //альтернативный выбор файла через класс сразу
    //void make_prediction();

    void change_image();


protected:
    virtual void wheelEvent(QWheelEvent *event);
    //virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

#endif // IMAGEVIEW_H
