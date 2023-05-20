#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QLabel>
#include <QSlider>
#include <thread>
#include <QScrollArea>
#include <QImageWriter>
#include <QPushButton>
#include <QBuffer>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    qualityFactor(90),
    buffer(new QBuffer())
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
        delete scroll->widget();
        scroll->setWidget(label);
        startCompression(qualityFactor);
        findChild<QPushButton*>("buttonSave")->setEnabled(true);
    }
}

void MainWindow::startCompression(int quality){

    QImageWriter writer;
    delete buffer;
    buffer = new QBuffer();
    writer.setDevice(buffer);
    writer.setFormat("jpeg");
    writer.setQuality(quality);


    writer.write(((QLabel*)findChild<QScrollArea*>("scrollOriginal")->widget())->pixmap().toImage());

    QLabel *label = new QLabel();
    QScrollArea *scroll = findChild<QScrollArea*>("scrollCompressed");
    QPixmap compressed;
    compressed.loadFromData(buffer->buffer(), "JPG");

    label->setPixmap(compressed);
    scroll->setWidget(label);

}

void MainWindow::saveImage() {
    QString saveFileName = QFileDialog::getSaveFileName(this, "Save compressed image", "compressed_image", "Jpeg (*.jpg);; All Files (*)");
    if (!saveFileName.isEmpty()){
        QFile image(saveFileName);
        image.open(QIODevice::WriteOnly);
        image.write(buffer->buffer());
        image.close();
    }
}

void MainWindow::on_sliderQuality_valueChanged(int value) {
    findChild<QLabel*>("labelQualityValue")->setText(QString::number(value));
    qualityFactor = value;
    if (findChild<QPushButton*>("buttonSave")->isEnabled()) {
        startCompression(value);
    }
}

void MainWindow::on_buttonSave_clicked() {
    saveImage();
}

