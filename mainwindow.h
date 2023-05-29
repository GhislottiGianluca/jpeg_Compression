#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include <QBuffer>
#include <QPixmap>
#include <QAbstractScrollArea>
#include "blockManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_loadButton_clicked();

    void on_sliderQuality_valueChanged(int value);

    void onCompressionFinished();

    void on_blockSize_editingFinished();

    void on_zoomIn_clicked();

    void on_zoomOut_clicked();

    void on_scroll_original(int value);

    void on_scroll_compressed(int value);

signals:
    void finishCompression();

private:
    Ui::MainWindow *ui;
    int qualityFactor;
    int blockSize;
    QBuffer *buffer;
    QImage *image;
    QImage *imageCompressed;
    BlockManager *blockManager;
    double scaleFactor;
    int horizontalScrollBar;
    int verticalScrollBar;

    void startCompression();
    void updateMaximalValues();
    void updateImageSize(double scaleFactor);

};
#endif // MAINWINDOW_H
