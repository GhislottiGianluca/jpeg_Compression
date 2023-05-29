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
#include <QSpinBox>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include "blockManager.h"
#include <QColor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    qualityFactor(2),
    buffer(new QBuffer()),
    image(nullptr),
    imageCompressed(nullptr),
    blockSize(10),
    blockManager(nullptr)
{
    ui->setupUi(this);
    findChild<QLabel*>("labelQualityValue")->setAlignment(Qt::AlignCenter);
    updateMaximalValues();

    connect(this, SIGNAL(finishCompression()), this, SLOT(onCompressionFinished()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete image;
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

        auto *oldImage = image;
        image = new QImage(bitmap.toImage());
        scroll->setWidget(label);
        delete oldImage;

        findChild<QLabel*>("labelOriginalTitle")->setText("<h3>Original (" + QString::number(size / 1000.0) +  " KB)</h3>");
        findChild<QLabel*>("labelCompressedTitle")->setText("<h3>Compressed</h3>");
        updateMaximalValues();
        blockManager = new BlockManager(image, blockSize, qualityFactor);
        startCompression();
    }
}

void MainWindow::onCompressionFinished() {
    QLabel *label = new QLabel();
    label->setAlignment(Qt::AlignCenter);
    QScrollArea *scroll = findChild<QScrollArea*>("scrollCompressed");
    QPixmap compressed;
    compressed.convertFromImage(*imageCompressed);

    label->setPixmap(compressed);
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    scroll->setWidget(label);

}

void MainWindow::startCompression(){
    auto *oldImage = imageCompressed;
    imageCompressed = blockManager->compress();
    delete oldImage;

    emit finishCompression();
}

void MainWindow::on_sliderQuality_valueChanged(int value) {
    findChild<QLabel*>("labelQualityValue")->setText(QString::number(value));
    qualityFactor = value;
    if(image != nullptr) {
        blockManager->setCutDimension(qualityFactor);
        blockManager->updateImage(*image);
        startCompression();
    }
}


void MainWindow::on_blockSize_editingFinished() {
    int newBlockSize = findChild<QSpinBox*>("blockSize")->value();
    if(image != nullptr && newBlockSize != blockSize){
        blockSize = newBlockSize;
        blockManager = new BlockManager(image, blockSize, qualityFactor);
        startCompression();
    }
    updateMaximalValues();
}

void MainWindow::updateMaximalValues() {
    if(image != nullptr)
        findChild<QSpinBox*>("blockSize")->setProperty("maximum", std::min(image->width(), image->height()));
    findChild<QSlider*>("sliderQuality")->setProperty("maximum", 2 * blockSize - 1);
}

