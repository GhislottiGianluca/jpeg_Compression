#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>

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

private:
    Ui::MainWindow *ui;
    std::thread *compressionThread;
    bool alreadyRunningCompression;

    void stopCompression();
    void startCompression();
    void zoomInOriginal();


};
#endif // MAINWINDOW_H
