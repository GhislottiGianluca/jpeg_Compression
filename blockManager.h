#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include <vector>
#include <QList>
#include <QImage>
#include <QRgb>
#include <QListIterator>
#include <iterator>
#include <cstddef>
#include <iostream>
#include <thread>
#include <fftw3.h>

class BlockManager {
    
public:
    BlockManager(const QImage *image, int blockSize, int cutDimension);
    ~BlockManager();
    double* getBlock(int row, int column);
    void setCutDimension(int dimension);
    QImage* compress();

public:
    int rows;
    int columns;
    int imgWidth;
    int imgHeight;

    void updateImage(const QImage &image);

private:
    int getBlockWidth(int i, int j) const;
    int getBlockHeight(int i, int j) const;
    fftw_plan selectDctPlan(int i, int j);
    fftw_plan selectIdctPlan(int i, int j);
    void cutValues(int row, int column);
    void parallelTask(const std::function<void(int, int)> &function, bool wait = true);
    int threadsCount;
    int cutDimension;
    double *values;
    int blockSize;
    std::thread** workers;
    fftw_plan dctPlan;
    fftw_plan idctPlan;
    fftw_plan dctPlanLastRow;
    fftw_plan idctPlanLastRow;
    fftw_plan dctPlanLastColumn;
    fftw_plan idctPlanLastColumn;
    fftw_plan dctPlanLastElement;
    fftw_plan idctPlanLastElement;
};


#endif
