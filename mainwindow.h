#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include <QBuffer>
#include <QPixmap>

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

signals:
    void finishCompression();

private:
    Ui::MainWindow *ui;
    int qualityFactor;
    int blockSize;
    QBuffer *buffer;
    QImage *image;
    QImage *imageCompressed;

    void startCompression();
    void updateMaximalValues();
    void setImage(std::string targetComponent, QPixmap pixmap);

};
#endif // MAINWINDOW_H
