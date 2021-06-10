#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <opencv2/opencv.hpp>
#include <QMouseEvent>
#include <fdeep/fdeep.hpp>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    const QMouseEvent* const me = static_cast<const QMouseEvent*>(event);
    const QPoint position = me->pos();
    ui->statusbar->showMessage(QString("(x,y) coordinates: (%1,%2)").arg(position.x()).arg(position.y()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//кнопка открыть изображение. Выбранное изображение помещается в лейбл input_image_label
void MainWindow::on_open_image_button_triggered()
{
    QString image_file = QFileDialog::getOpenFileName(this,
         tr("Open Image"), "D:/Nadenenko/project_rsw/", tr("Image Files (*.png *.jpg *.bmp *.tiff *.tif)"));

        std::string image_path = image_file.toStdString();
        cv::Mat img_inp;
        img_inp = cv::imread(image_path);
        //int w = ui->input_image_label->width();
        //int h = ui->input_image_label->height();
//        ui->input_image_label->setPixmap(
//                    QPixmap::fromImage(QImage(img_inp.data, img_inp.cols, img_inp.rows,
//                                                           img_inp.step, QImage::Format_RGB888)).scaled(256,128,Qt::KeepAspectRatio));
        //ui->scrollArea->setPixmap
        //ui->input_image_label->refreshLabel();

        //ui->input_image_label->adjustSize();
            //QImage imgz(img_inp.data, img_inp.cols, img_inp.rows);
            //QImage image("D:/z.png");
            //item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
            item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(img_inp.data, img_inp.cols, img_inp.rows,
                                                                     img_inp.step, QImage::Format_RGB888)));
            scene = new QGraphicsScene(this);
            //scene->setSceneRect(500, 50, 50, 50);
            ui->graphicsView->setScene(scene);
            scene->addItem(item);
            ui->path_to_image->setText(image_file);

}

void MainWindow::on_start_processing_triggered()
{

    const std::string i_p = ui->path_to_image->text().toStdString();
    cv::Mat img;
    img = cv::imread(i_p);

   const auto model = fdeep::load_model("D:/Nadenenko/CProjects/QtProjects/ProcessImagePOKAZ/ProcessImage/model_8M_params_50e.json");
   const int nearest_x = (img.size().width / 256) * 256;
   const int nearest_y = (img.size().height / 256) * 256;

       cv::Size dsize = cv::Size(nearest_x, nearest_y);
       cv::resize(img, img, dsize, 0, 0, cv::INTER_NEAREST);

       std::vector<cv::Mat> images;
       for (int j = 0; j <= nearest_x - 1; j += 256)
           {
               for (int k = 0; k <= nearest_y - 1; k += 128)
               {
                   cv::Mat cropped = img(cv::Rect(0 + j, 0 + k, 256, 128)).clone();
                   images.push_back(cropped);
               }
           }
       std::vector<fdeep::tensors> multi_inputs;
          for (const auto& image : images) {
              multi_inputs.push_back({
                  fdeep::tensor_from_bytes(image.ptr(),
                  static_cast<std::size_t>(image.rows),
                  static_cast<std::size_t>(image.cols),
                  static_cast<std::size_t>(image.channels()),
                  0.0f, 1.0f)
                  });
          }
          std::cout << "Preprocessing is done" << std::endl;

          const auto result = model.predict_multi(multi_inputs, true);
          std::vector<cv::Mat> predictions;
              for (const auto& r : result)
              {
                  const auto single = fdeep::internal::single_tensor_from_tensors(r);
                  const cv::Mat buffer(
                      cv::Size(single.shape().width_, single.shape().height_), CV_32FC1);
                  const auto values = single.to_vector();
                  std::memcpy(buffer.data, values.data(), values.size() * sizeof(float));
                  // normalize into byte cv::Mat (image3 to image5)
                  cv::Mat tempImage5;
                  cv::Mat image5;
                  cv::normalize(buffer, tempImage5, 255.0, 0.0, cv::NORM_MINMAX);
                  tempImage5.convertTo(image5, CV_8UC3);
                  cvtColor(image5, image5, cv::COLOR_GRAY2RGB);
                  //predictions.push_back(buffer);
                  predictions.push_back(image5);
              }
            std::cout << "Prediction is done " << std::endl;
              int final_height = img.size().height; //max(img1.rows, img2.rows);
                  int final_width = img.size().width; //img1.cols + img2.cols;

                  cv::Mat3b res(final_height, final_width, cv::Vec3b(0, 0, 0));

                  int step = 0;
                          for (int i = 0; i <= final_width-1; i += 256)
                          {
                              for (int j = 0; j <= final_height-1; j += 128)
                              {
                                  //place_pred(predictions[step], res, i, j);
                                  predictions[step].copyTo(res(cv::Rect(i, j, predictions[step].cols, predictions[step].rows)));
                                  //place_pred(predictions[i/256 * (final_width / 256 - 1) + j/128], res, i, j);
                                  //cout << "index = " << (i / 256 * (final_width / 256 - 1) + j / 128) << endl;

                                  step++;
                              }
                          }

                      //Накладываем res на image для показа
                      cv::Mat gray;
                      cvtColor(res, gray, cv::COLOR_BGR2GRAY);

                      cv::Mat thresholded;
                      threshold(gray, thresholded, 50, 255, cv::THRESH_BINARY);
                      /// Find contours
                      std::vector<std::vector<cv::Point> > contours;
                      std::vector<cv::Vec4i> hierarchy;
                      //flip(thresholded, thresholded, 0); //чтобы в тест билдере построилось нормально
                      findContours(thresholded, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

                      //cv::Mat drawing = cv::Mat::zeros(gray.size(), CV_8UC3); //gray просто для размера


                        cv::Mat show = img.clone();
                      for (int i = 0; i < contours.size(); i++)
                      {
                          if (contours[i].size() > 50)
                          {
                          drawContours(show, contours, i, cv::Scalar(255, 0, 0), -1, 8, hierarchy, 0, cv::Point());
                              //drawContours(drawing, contours, i, cv::Scalar(255,0,0), cv::FILLED, 8, hierarchy);
                          }
                      }

                      //fillPoly(drawing, contours, cv::Scalar(255, 0, 0));
                      //cv::Mat final_img;
                      //add(img, drawing, final_img);
                      //item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(final_img.data, final_img.cols, final_img.rows,
                                                                               //final_img.step, QImage::Format_RGB888)));

                      item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(show.data, show.cols, show.rows,
                                                                               show.step, QImage::Format_RGB888)));
                      scene = new QGraphicsScene(this);
                      ui->graphicsView->setScene(scene);
                      scene->addItem(item);

                          //imwrite("D:/Nadenenko/POKAZ/imwrite/final_img.png", final_img);
                          //imwrite("D:/Nadenenko/POKAZ/imwrite/drawing.png", drawing);
                          imwrite("D:/Nadenenko/POKAZ/imwrite/pred.png", res);
                          imwrite("D:/Nadenenko/POKAZ/imwrite/thresholded.png", thresholded);
                          imwrite("D:/Nadenenko/POKAZ/imwrite/orig.png", img);
                          imwrite("D:/Nadenenko/POKAZ/imwrite/final_img.png", show);
}

//при нажатии на зум+ выполняется функция zoom_in (описана в imageview.cpp, объявлена в заголовочном)
void MainWindow::on_zoom_in_button_triggered()
{
    ui->graphicsView->zoom_in();
}

//при нажатии на зум- выполняется функция zoom_out (описана в imageview.cpp, объявлена в заголовочном)
void MainWindow::on_zoom_out_button_triggered()
{
    ui->graphicsView->zoom_out();
}

//при нажатии на кнопку фит выполняется функция viewFit (описана в imageview.cpp, объявлена в заголовочном)
void MainWindow::on_fit_button_triggered()
{
    ui->graphicsView->viewFit();

}

//при нажатии на кнопку фит_inv выполняется функция rectFit (описана в imageview.cpp, объявлена в заголовочном)
void MainWindow::on_fit_inv_button_triggered()
{
    ui->graphicsView->rectFit();
}

void MainWindow::on_exit_button_triggered()
{
    QApplication::quit();
}


void MainWindow::on_choose_area_triggered()
{
    ui->graphicsView->get_area();
}

void MainWindow::on_change_view_triggered()
{
    cv::Mat img;
    img = cv::imread("D:/Nadenenko/POKAZ/imwrite/thresholded.png");
    item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(img.data, img.cols, img.rows,
                                                             img.step, QImage::Format_RGB888)));
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->addItem(item);




}

void MainWindow::on_new_choose_triggered()
{
    ui->graphicsView->new_choose();
}



void MainWindow::on_show_orig_triggered()
{
    cv::Mat img;
    img = cv::imread("D:/Nadenenko/POKAZ/imwrite/orig.png");
    item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(img.data, img.cols, img.rows,
                                                             img.step, QImage::Format_RGB888)));
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->addItem(item);
}

void MainWindow::on_show_result_triggered()
{
    cv::Mat img;
    img = cv::imread("D:/Nadenenko/POKAZ/imwrite/pred.png");
    item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(img.data, img.cols, img.rows,
                                                             img.step, QImage::Format_RGB888)));
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->addItem(item);
}

void MainWindow::on_show_final_triggered()
{
    cv::Mat img;
    img = cv::imread("D:/Nadenenko/POKAZ/imwrite/final_img.png");
    item = new QGraphicsPixmapItem(QPixmap::fromImage(QImage(img.data, img.cols, img.rows,
                                                             img.step, QImage::Format_RGB888)));
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->addItem(item);
}
