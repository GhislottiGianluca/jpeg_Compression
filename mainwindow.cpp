#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QLabel>
#include <QSlider>
#include <thread>

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
        QLabel *imageLabel = findChild<QLabel*>("labelOriginal");
        QPixmap bitmap(select);
        imageLabel->setPixmap(bitmap);
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

