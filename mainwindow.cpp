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
    blockSize(10)
{
    ui->setupUi(this);
    findChild<QLabel*>("labelQualityValue")->setAlignment(Qt::AlignCenter);

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
        delete image;
        delete scroll->widget();
        image = new QImage(bitmap.toImage());
        scroll->setWidget(label);

        findChild<QLabel*>("labelOriginalTitle")->setText("<h3>Original (" + QString::number(size / 1000.0) +  " KB)</h3>");
        findChild<QLabel*>("labelCompressedTitle")->setText("<h3>Compressed</h3>");
        updateMaximalValues();

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
    delete scroll->widget();
    scroll->setWidget(label);

}

void MainWindow::startCompression(){
    BlockManager mgr = BlockManager(image, blockSize, qualityFactor);
    mgr.apply_dct2();
    mgr.cutFrequencies();
    mgr.apply_idct2();

    delete imageCompressed;
    imageCompressed = new QImage(*image);

    BlockManager::Iterator it = mgr.begin();
    BlockManager::Iterator end = mgr.end();

    for(int i = 0; i < mgr.columns * blockSize; ++i){
        for(int j = 0; j < mgr.rows * blockSize && it != end; ++j){
            imageCompressed->setPixel(i, j, QColor(*it, *it, *it).rgb());
            ++it;
        }
    }


    emit finishCompression();
}

void MainWindow::on_sliderQuality_valueChanged(int value) {
    findChild<QLabel*>("labelQualityValue")->setText(QString::number(value));
    qualityFactor = value;
    if(image != nullptr)
        startCompression();
}


void MainWindow::on_blockSize_editingFinished() {
    blockSize = findChild<QSpinBox*>("blockSize")->value();
    updateMaximalValues();
    if(image != nullptr)
        startCompression();
}

void MainWindow::updateMaximalValues() {
    if(image != nullptr)
        findChild<QSpinBox*>("blockSize")->setProperty("maximum", std::min(image->width(), image->height()));
    findChild<QSlider*>("sliderQuality")->setProperty("maximum", 2 * (blockSize - 1));
}

