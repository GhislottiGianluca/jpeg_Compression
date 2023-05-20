#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QLabel>
#include <QSlider>
#include <thread>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    alreadyRunningCompression(false),
    compressionThread(nullptr)
{
    ui->setupUi(this);
    findChild<QLabel*>("labelQualityValue")->setAlignment(Qt::AlignCenter);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_loadButton_clicked() {
    QString select = QFileDialog::getOpenFileName(this, "Select a Bitmap image:", "", "Bitmap (*.bmp) ;; All Files (*.*)");
    if (!select.isEmpty()) {
        QLabel *label = new QLabel();
        QScrollArea *scroll = findChild<QScrollArea*>("scrollOriginal");
        QPixmap bitmap(select);
        label->setPixmap(bitmap);
        scroll->setWidget(label);
    }
}

void MainWindow::startCompression(){
    alreadyRunningCompression = true;
    compressionThread = new std::thread([&]{
        // Do something............

        alreadyRunningCompression = false;
    });
}

void MainWindow::stopCompression() {
    if (compressionThread != nullptr) {
        compressionThread->join();
        delete compressionThread;
        compressionThread = nullptr;
    }
}

void MainWindow::on_sliderQuality_valueChanged(int value) {
    findChild<QLabel*>("labelQualityValue")->setText(QString::number(value));
    if (!alreadyRunningCompression) {
        stopCompression();
        startCompression();
    }
}

void MainWindow::zoomInOriginal() {
    QScrollArea *scrollArea = findChild<QScrollArea*>("scrollOriginal");
    QPixmap old_img = ((QLabel*)scrollArea->widget())->pixmap();
    QPixmap new_img = old_img.scaled(1.5 * old_img.width(), 1.5 * old_img.height(), Qt::KeepAspectRatio);
    ((QLabel*)scrollArea->widget())->setPixmap(new_img);
}

