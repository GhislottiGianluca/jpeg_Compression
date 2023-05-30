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
#include <QScrollBar>
#include "blockManager.h"
#include <QColor>

#define ZOOM_SCALE_INCREMENT  0.5

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    qualityFactor(2),
    buffer(new QBuffer()),
    image(nullptr),
    imageCompressed(nullptr),
    blockSize(10),
    blockManager(nullptr),
    scaleFactor(1)
{
    ui->setupUi(this);
    findChild<QPushButton*>("zoomIn")->setIcon(QIcon(":/icons/zoomIn.png"));
    findChild<QPushButton*>("zoomOut")->setIcon(QIcon(":/icons/zoomOut.png"));
    findChild<QLabel*>("labelQualityValue")->setAlignment(Qt::AlignCenter);
    updateMaximalValues();

    connect(this, SIGNAL(finishCompression()), this, SLOT(onCompressionFinished()));

    QScrollBar *horizontalScroll = findChild<QScrollBar*>("horizontalScrollBar");
    QScrollBar *verticalScroll = findChild<QScrollBar*>("verticalScrollBar");

    horizontalScroll->setHidden(true);
    verticalScroll->setHidden(true);

    connect(findChild<QScrollArea*>("scrollOriginal")->horizontalScrollBar(), &QScrollBar::valueChanged, [&](int value){
        findChild<QScrollBar*>("horizontalScrollBar")->setValue(value);
    });
    connect(findChild<QScrollArea*>("scrollCompressed")->horizontalScrollBar(), &QScrollBar::valueChanged, [&](int value){
        findChild<QScrollBar*>("horizontalScrollBar")->setValue(value);
    });
    connect(findChild<QScrollArea*>("scrollOriginal")->verticalScrollBar(), &QScrollBar::valueChanged, [&](int value){
        findChild<QScrollBar*>("verticalScrollBar")->setValue(value);
    });
    connect(findChild<QScrollArea*>("scrollCompressed")->verticalScrollBar(), &QScrollBar::valueChanged, [&](int value){
        findChild<QScrollBar*>("verticalScrollBar")->setValue(value);
    });
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
        auto *label = new QLabel();
        auto *scroll = findChild<QScrollArea*>("scrollOriginal");
        QPixmap bitmap(select);
        label->setObjectName("originalImage");
        label->setPixmap(bitmap);
        label->setScaledContents(true);
        label->setAlignment(Qt::AlignCenter);
        scroll->setWidget(label);

        scaleFactor = 1;

        if (image == nullptr) {
            image = new QImage(bitmap.toImage());
        } else {
            *image = bitmap.toImage();
        }

        findChild<QLabel*>("labelOriginalTitle")->setText("<h3>Original (" + QString::number(size / 1000.0) +  " KB)</h3>");
        findChild<QLabel*>("labelCompressedTitle")->setText("<h3>Compressed</h3>");
        updateMaximalValues();
        blockManager = new BlockManager(image, blockSize, qualityFactor);

        startCompression();
    }
}

void MainWindow::onCompressionFinished() {
    int oldVertical = verticalScrollValue;
    int oldHorizontal = horizontalScrollValue;

    QLabel *label = new QLabel();
    label->setAlignment(Qt::AlignCenter);
    QScrollArea *scroll = findChild<QScrollArea*>("scrollCompressed");
    QPixmap compressed;
    compressed.convertFromImage(*imageCompressed);

    label->setPixmap(compressed);
    label->setObjectName("compressedImage");
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    label->setScaledContents(true);
    scroll->setWidget(label);
    updateImageSize(scaleFactor);
    scroll->horizontalScrollBar()->setValue(oldHorizontal);
    scroll->verticalScrollBar()->setValue(oldVertical);
}

void MainWindow::startCompression(){
    auto *oldImage = imageCompressed;
    imageCompressed = blockManager->compress();
    delete oldImage;

    emit finishCompression();
}

void MainWindow::updateScrollBar() {
    QScrollArea *scrollOriginal = findChild<QScrollArea*>("scrollOriginal");
    QScrollArea *scrollCompressed = findChild<QScrollArea*>("scrollCompressed");

    scrollCompressed->verticalScrollBar()->setValue(verticalScrollValue);
    scrollCompressed->horizontalScrollBar()->setValue(horizontalScrollValue);

    scrollOriginal->verticalScrollBar()->setValue(verticalScrollValue);
    scrollOriginal->horizontalScrollBar()->setValue(horizontalScrollValue);

    //QScrollBar *verticalScrollbar = findChild<QScrollBar*>("verticalScrollBar");
    //QScrollBar *horizontalScrollbar = findChild<QScrollBar*>("horizontalScrollBar");
    //verticalScrollbar->setValue(verticalScrollValue);
    //horizontalScrollbar->setValue(horizontalScrollValue);
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
    blockSize = findChild<QSpinBox*>("blockSize")->value();
    updateMaximalValues();

    if(image != nullptr){
        blockManager = new BlockManager(image, blockSize, qualityFactor);
        startCompression();
    }
}

void MainWindow::updateMaximalValues() {
    if(image != nullptr) {
        findChild<QSpinBox*>("blockSize")->setProperty("maximum", std::min(image->width(), image->height()));
        blockSize = std::min(blockSize, std::min(image->width(), image->height()));
    }
    findChild<QSlider*>("sliderQuality")->setProperty("maximum", 2 * blockSize - 1);
    updateScrollBar();

    auto *scroll = findChild<QScrollArea*>("scrollOriginal");

    QScrollBar *horizontalScroll = findChild<QScrollBar*>("horizontalScrollBar");
    if(scroll->horizontalScrollBar()->maximum() == 0)
        horizontalScroll->setHidden(true);
    else
        horizontalScroll->setHidden(false);
    horizontalScroll->setMaximum(scroll->horizontalScrollBar()->maximum());


    QScrollBar *verticalScroll = findChild<QScrollBar*>("verticalScrollBar");
    if(scroll->verticalScrollBar()->maximum() == 0)
        verticalScroll->setHidden(true);
    else
        verticalScroll->setHidden(false);
    verticalScroll->setMaximum(scroll->verticalScrollBar()->maximum());
}


void MainWindow::on_zoomIn_clicked()
{
    scaleFactor += ZOOM_SCALE_INCREMENT;
    updateImageSize(scaleFactor);
    updateMaximalValues();
}

void MainWindow::on_zoomOut_clicked()
{
    if(scaleFactor - ZOOM_SCALE_INCREMENT <= 0)
        return;

    scaleFactor -= ZOOM_SCALE_INCREMENT;
    updateImageSize(scaleFactor);
    updateMaximalValues();
}


void MainWindow::updateImageSize(double scaleFactor){
    QLabel *originalImageLabel = findChild<QLabel*>("originalImage");
    if(originalImageLabel != nullptr){
        originalImageLabel->setPixmap(QPixmap::fromImage(image->scaled(image->width() * scaleFactor, image->height() * scaleFactor)));
        originalImageLabel->resize(originalImageLabel->pixmap().size());
    }

    QLabel *compressedImageLabel = findChild<QLabel*>("compressedImage");
    if(compressedImageLabel != nullptr){
        compressedImageLabel->setPixmap(QPixmap::fromImage(imageCompressed->scaled(imageCompressed->width() * scaleFactor, imageCompressed->height() * scaleFactor)));
        compressedImageLabel->resize(compressedImageLabel->pixmap().size());
    }

    updateMaximalValues();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateMaximalValues();
}


void MainWindow::on_horizontalScrollBar_valueChanged(int value)
{
    horizontalScrollValue = value;
    updateScrollBar();
}


void MainWindow::on_verticalScrollBar_valueChanged(int value)
{
    verticalScrollValue = value;
    updateScrollBar();
}

