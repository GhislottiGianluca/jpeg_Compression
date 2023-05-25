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
#include <assert.h>
#include "blockManager.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    qualityFactor(90),
    buffer(new QBuffer()),
    image(nullptr)
{
    ui->setupUi(this);
    findChild<QLabel*>("labelQualityValue")->setAlignment(Qt::AlignCenter);

    connect(this, SIGNAL(finishCompression()), this, SLOT(onCompressionFinished()));
}

MainWindow::~MainWindow()
{
    delete ui;
    //delete image;
}

void MainWindow::on_loadButton_clicked() {
    QString select = QFileDialog::getOpenFileName(this, "Select a Bitmap image:", "", "Bitmap (*.bmp) ;; All Files (*.*)");
    if (!select.isEmpty()) {
        QFile file(select);
        file.open(QIODevice::ReadOnly);
        double size = file.size();
        file.close();
        QLabel *label = new QLabel();
        QScrollArea *scroll = findChild<QScrollArea*>("scrollOriginal");
        QPixmap bitmap(select);
        label->setPixmap(bitmap);
        label->setAlignment(Qt::AlignCenter);

        //delete image;
        image = new QImage(bitmap.toImage());

        delete scroll->widget();
        scroll->setWidget(label);
        findChild<QLabel*>("labelOriginalTitle")->setText("<h3>Original (" + QString::number(size / 1000.0) +  " KB)</h3>");
        findChild<QLabel*>("labelCompressedTitle")->setText("<h3>Compressed</h3>");
        startCompression();
        findChild<QPushButton*>("buttonSave")->setEnabled(true);
    }
}

void MainWindow::onCompressionFinished() {
    QLabel *label = new QLabel();
    label->setAlignment(Qt::AlignCenter);
    QScrollArea *scroll = findChild<QScrollArea*>("scrollCompressed");
    QPixmap compressed;
    compressed.loadFromData(buffer->buffer(), "JPG");

    label->setPixmap(compressed);
    delete scroll->widget();
    scroll->setWidget(label);
    findChild<QLabel*>("labelCompressedTitle")->setText("<h3>Compressed (" + QString::number((double)buffer->buffer().size() / 1000.0) +  " KB)</h3>");
}

void MainWindow::startCompression(){
    QImageWriter writer;

    BlockManager mgr = BlockManager(image, 5, 0);
    mgr.apply_dct();

    delete buffer;
    buffer = new QBuffer();
    writer.setDevice(buffer);
    writer.setFormat("jpeg");
    writer.setQuality(qualityFactor);
    writer.setOptimizedWrite(true);
    writer.setProgressiveScanWrite(true);

    writer.write(((QLabel*)findChild<QScrollArea*>("scrollOriginal")->widget())->pixmap().toImage());
    emit finishCompression();
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
        startCompression();
    }
}

void MainWindow::on_buttonSave_clicked() {
    saveImage();
}

