#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include <QBuffer>

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

    void on_buttonSave_clicked();

    void onCompressionFinished();

signals:
    void finishCompression();

private:
    Ui::MainWindow *ui;
    int qualityFactor;
    QBuffer *buffer;
    QImage *image;

    void saveImage();
    void startCompression();

};
#endif // MAINWINDOW_H
