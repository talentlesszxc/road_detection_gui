#include "imageview.h"
#include <opencv2/opencv.hpp>
#include <fdeep/fdeep.hpp>
imageview::imageview(QWidget *parent) : QGraphicsView(parent)
{
    setTransformationAnchor(AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);
}
//мастшаб колесом мыши
void imageview::wheelEvent(QWheelEvent *event)
{

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    double scaleFactor = 1.15;

    if (event->delta() > 0)
    {
        scale(scaleFactor, scaleFactor);
    }
    else
    {
        scale(1/scaleFactor, 1/scaleFactor);
    }
}
//вписывает все изображение в экран
void imageview::viewFit()
{
    fitInView(sceneRect(), Qt::KeepAspectRatio);
}
//масштабирует область изображения по размеру экрана
void imageview::rectFit()
{
    QRectF exactRect(4000, 0, 1792, 1024); //(х_начала, у_начала, размер_х_прямоугольника, размер_у_прямоугольника)
    fitInView(exactRect);
}
//void imageview::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    /* При нажатии мышью на графический элемент
//     * заменяем курсор на руку, которая держит этот элемента
//     * */
//    this->setCursor(QCursor(Qt::ClosedHandCursor));
//    Q_UNUSED(event);
//}


//альтернативный выбор файла через класс сразу
void imageview::new_choose()
{
    QString image_file = QFileDialog::getOpenFileName(this,
         tr("Open Image"), "D:/Nadenenko/CProjects/QtProjects/ProcessImage/ProcessImage/", tr("Image Files (*.png *.jpg *.bmp *.tiff *.tif)"));
        std::string image_path = image_file.toStdString();
        cv::Mat img_inp;
        img_inp = cv::imread(image_path);
        QGraphicsItem*item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(img_inp.data, img_inp.cols, img_inp.rows,
                                                                 img_inp.step, QImage::Format_RGB888)));
        QGraphicsScene*scene = new QGraphicsScene(this);
        //scene->setSceneRect(500, 50, 50, 50);
        setScene(scene);
        scene->addItem(item);

}
//void imageview::make_prediction()
//{
//    const int nearest_x = (img_inp.size().width / 256) * 256;
//    const int nearest_y = (img_inp.size().height / 256) * 256;
//    const auto model = fdeep::load_model("D:/Nadenenko/CProjects/QtProjects/ProcessImagePOKAZ/ProcessImage/model_8M_params_50e.json");
//    cv::Size dsize = cv::Size(nearest_x, nearest_y);
//    resize(img_inp, img_inp, dsize, 0, 0, cv::INTER_NEAREST);
//    std::cout << "New Width: " << img_inp.size().width << endl;
//    std::cout << "New Height: " << img_inp.size().height << endl;

//}
void imageview::change_image()
{

}

//зум+
void imageview::zoom_in()
{
    double scaleFactor = 1.15;
    scale(scaleFactor, scaleFactor);
}
//зум-
void imageview::zoom_out()
{
    double scaleFactor = 1.15;
    scale(1/scaleFactor, 1/scaleFactor);
}
//сохраняет выбранную область (не работает, сохраняет всё изображение). Потом убрать
void imageview::get_area()
{
    QRectF eRect(4000, 0, 1792, 1024);
    QImage image(eRect.size().toSize(), QImage::Format_ARGB32);
    //QImage zxc(eRect)

    image.fill(Qt::transparent);

    QPainter painter(&image);
    render(&painter);
    image.save("D:/file_name.png");
}
