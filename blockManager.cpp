#include "blockManager.h"
#include <iostream>
#include <fftw3.h>
#include <thread>
#include <cmath>

void BlockManager::parallelTask(const std::function<void(int, int)> &function, bool wait) {
    int coreCount = ceil(sqrt((double) std::thread::hardware_concurrency()));

    int rowsPerThread = ceil((double)rows / (double)coreCount);
    int colsPerThread = ceil((double)columns / (double)coreCount);

    workers = new std::thread*[(int)(ceil((double)rows / (double)rowsPerThread) * ceil((double)columns / (double)colsPerThread))];

    threadsCount = 0;
    for (int threadRow = 0; threadRow * rowsPerThread < rows; ++threadRow) {
        for (int threadCol = 0; threadCol * colsPerThread < columns; ++threadCol) {
            workers[threadsCount] = new std::thread([&function, threadRow, threadCol, rowsPerThread, colsPerThread, this](){
                for (int i = threadRow * rowsPerThread; i < threadRow * rowsPerThread + rowsPerThread && i < rows; ++i) {
                    for (int j = threadCol * colsPerThread; j < threadCol * colsPerThread + colsPerThread && j < columns; ++j) {
                        function(i, j);
                    }
                }

            });
            threadsCount++;
        }
    }

    if (!wait) {
        return;
    }

    for (int i = 0; i < threadsCount; ++i) {
        workers[i]->join();
        delete workers[i];
    }

    delete[] workers;
    workers = nullptr;
}


BlockManager::BlockManager(const QImage *image, int blockSize, int cutDimension): workers(nullptr), imgWidth(image->width()), imgHeight(image->height()), blockSize(blockSize), threadsCount(0), cutDimension(cutDimension) {

    rows = ceil((double)imgHeight /(double) blockSize);
    columns = ceil((double)imgWidth / (double)blockSize);

    values = new double[imgHeight * imgWidth];
    dctPlan = fftw_plan_r2r_2d(blockSize, blockSize, values, values, FFTW_REDFT10, FFTW_REDFT10, 0);
    idctPlan = fftw_plan_r2r_2d(blockSize, blockSize, values, values, FFTW_REDFT01, FFTW_REDFT01, 0);

    int lastBlockHeight = imgHeight % blockSize == 0 ? blockSize : imgHeight % blockSize;
    int lastBlockWidth = imgWidth % blockSize == 0 ? blockSize : imgWidth % blockSize;

    dctPlanLastRow = fftw_plan_r2r_2d(lastBlockHeight, blockSize, values, values, FFTW_REDFT10, FFTW_REDFT10, 0);
    idctPlanLastRow = fftw_plan_r2r_2d(lastBlockHeight, blockSize, values, values, FFTW_REDFT01, FFTW_REDFT01, 0);

    dctPlanLastColumn = fftw_plan_r2r_2d(blockSize, lastBlockWidth, values, values, FFTW_REDFT10, FFTW_REDFT10, 0);
    idctPlanLastColumn = fftw_plan_r2r_2d(blockSize, lastBlockWidth, values, values, FFTW_REDFT01, FFTW_REDFT01, 0);

    dctPlanLastElement = fftw_plan_r2r_2d(lastBlockHeight, lastBlockWidth, values, values, FFTW_REDFT10, FFTW_REDFT10, 0);
    idctPlanLastElement = fftw_plan_r2r_2d(lastBlockHeight, lastBlockWidth, values, values, FFTW_REDFT01, FFTW_REDFT01, 0);

    updateImage(*image);
}


double* BlockManager::getBlock(int row, int column) {
    int lastColumnWidth = imgWidth % blockSize == 0 ? blockSize : imgWidth % blockSize;
    int lastRowHeight = getBlockHeight(row, column);
    int lastColumn = row * (lastColumnWidth * blockSize);
    int lastRowPixels = column * blockSize * lastRowHeight;
    int centerPixels = row * (columns - 1) * blockSize * blockSize;

    return &values[lastColumn + lastRowPixels + centerPixels];
}


BlockManager::~BlockManager() {
    delete[] values;
    delete[] workers;
    fftw_destroy_plan(dctPlan);
    fftw_destroy_plan(idctPlan);
    fftw_destroy_plan(dctPlanLastRow);
    fftw_destroy_plan(idctPlanLastRow);
    fftw_destroy_plan(dctPlanLastColumn);
    fftw_destroy_plan(idctPlanLastColumn);
    fftw_destroy_plan(dctPlanLastElement);
    fftw_destroy_plan(idctPlanLastElement);
    fftw_cleanup();
}


void BlockManager::cutValues(int row, int column) {
    double *block = getBlock(row, column);

    int blockWidth = getBlockWidth(row, column);
    int blockHeight = getBlockHeight(row, column);


    int rowLimit = cutDimension - blockHeight;
    if (rowLimit < 0) {
        rowLimit = 0;
    }

    for (int i = rowLimit; i < blockHeight; ++i) {
        int colLimit = cutDimension - i;
        if (colLimit < 0) {
            colLimit = 0;
        }

        for (int j = colLimit; j < blockWidth; ++j) {
            block[i * blockWidth + j] = 0;
        }
    }
}

fftw_plan BlockManager::selectDctPlan(int i, int j) {
    if (i < rows - 1 && j < columns - 1) {
        return dctPlan;
    }
    if (i < rows - 1 && j == columns - 1) {
        return dctPlanLastColumn;
    }
    if (i == rows - 1 && j < columns - 1) {
        return dctPlanLastRow;
    }
    return dctPlanLastElement;
}

fftw_plan BlockManager::selectIdctPlan(int i, int j) {
    if (i < rows - 1 && j < columns - 1) {
        return idctPlan;
    }
    if (i < rows - 1 && j == columns - 1) {
        return idctPlanLastColumn;
    }
    if (i == rows - 1 && j < columns - 1) {
        return idctPlanLastRow;
    }
    return idctPlanLastElement;
}

QImage* BlockManager::compress() {
    auto *out = new QImage(imgWidth, imgHeight, QImage::Format_RGB32);
    QRgb *imageBits = (QRgb*)out->bits();

    parallelTask([&](int i, int j){
        double* block = getBlock(i, j);
        int blockWidth = getBlockWidth(i, j);
        int blockHeight = getBlockHeight(i, j);

        fftw_plan dct = selectDctPlan(i, j);
        fftw_plan idct = selectIdctPlan(i, j);

        fftw_execute_r2r(dct, block, block);
        cutValues(i, j);
        fftw_execute_r2r(idct, block, block);

        int count = 0;
        for (int pixelRow = 0; pixelRow <  blockHeight; ++pixelRow) {
            for (int pixelCol = 0; pixelCol < blockWidth; ++pixelCol) {
                int excessColumnWidth = imgWidth % blockSize;
                int fullCols = columns;
                if (excessColumnWidth > 0) {
                    --fullCols;
                }

                int realRow = i * (blockSize * fullCols + excessColumnWidth) * blockSize + pixelRow * (blockSize * fullCols + excessColumnWidth);
                int realCol = j * blockSize + pixelCol;

                int value = (int)(block[count] / (4 * blockWidth * blockHeight));
                if (value < 0) value = 0;
                if (value > 255) value = 255;

                imageBits[realRow + realCol] = QColor(value, value, value).rgba();
                ++count;
            }
        }
    });

    return out;
}

void BlockManager::setCutDimension(int dimension) {
    this->cutDimension = dimension;
}

void BlockManager::updateImage(const QImage &image) {
    QRgb * imageBits = (QRgb*)image.bits();

    parallelTask([&](int i, int j){
        int blockWidth = getBlockWidth(i, j);
        int blockHeight = getBlockHeight(i, j);
        double *block = getBlock(i, j);

        int count = 0;
        for (int pixelRow = 0; pixelRow < blockHeight; ++pixelRow) {
            for (int pixelCol = 0; pixelCol < blockWidth; ++pixelCol) {
                int excessColumnWidth = imgWidth % blockSize;
                int fullCols = columns;
                if (excessColumnWidth > 0) {
                    --fullCols;
                }

                int realRow = i * (blockSize * fullCols + excessColumnWidth) * blockSize + pixelRow * (blockSize * fullCols + excessColumnWidth);
                int realCol = j * blockSize + pixelCol;

                block[count] = qGray(imageBits[realRow + realCol]);
                ++count;
            }
        }
    });
}

int BlockManager::getBlockHeight(int i, int j) const {
    if (i == rows - 1 && imgHeight % blockSize > 0) {
        return imgHeight % blockSize;
    }
    return blockSize;
}

int BlockManager::getBlockWidth(int i, int j) const {
    if (j == columns - 1 && imgWidth % blockSize > 0) {
        return imgWidth % blockSize;
    }
    return blockSize;
}
