#include "blockManager.h"
#include <iostream>
#include <fftw3.h>
#include <thread>
#include <cmath>

void BlockManager::parallelTask(const std::function<void(int, int)> &function, bool wait) {
    int coreCount = ceil(sqrt((double) std::thread::hardware_concurrency())) * 2;

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


BlockManager::BlockManager(const QImage *image, int blockSize, int cutDimension): imgWidth(image->width()), imgHeight(image->height()), blockSize(blockSize), threadsCount(0) {

    rows = imgHeight / blockSize;
    columns = imgWidth / blockSize;

    blocks = new Block*[rows * columns];

    parallelTask([&](int i, int j){
        if (i < rows - 1 && j < columns - 1) {
            blocks[i * columns + j] = new Block(blockSize, blockSize, cutDimension);
        }
    });

    int lastRowHeight = blockSize + imgHeight % blockSize;
    for (int j = 0; j < columns - 1; ++j) {
        blocks[(rows - 1) * columns + j] = new Block(lastRowHeight, blockSize, cutDimension);
    }

    int lastColWidth = blockSize + imgWidth % blockSize;
    for (int i = 0; i < rows - 1; ++i) {
        blocks[i * columns + columns - 1] = new Block(blockSize, lastColWidth, cutDimension);
    }

    blocks[(rows - 1) * columns + columns - 1] = new Block(lastRowHeight, lastColWidth, cutDimension);
    for (int i = 0; i < rows * columns; ++i) {
        blocks[i]->createPlans();
    }

    updateImage(*image);
}


BlockManager::Block& BlockManager::getBlock(int row, int column) {
    return *blocks[(row * this->columns) + column];
}

void BlockManager::Block::put_row(int row, QRgb *iterator) {
    for(int i = 0; i < height; ++i){
        values[(row * width) + i] = qGray(iterator[i]);
    }
}

BlockManager::Block::Block(int height, int width, int cutDimension): height(height), width(width), cutDimension(cutDimension), values(nullptr) {
    values = new double[width * height];
}

BlockManager::~BlockManager() {
    for (int i = 0; i < rows * columns; ++i)
        delete blocks[i];
    delete[] blocks;
}

BlockManager::Block::~Block() {
    delete[] values;
    fftw_destroy_plan(dct_plan);
    fftw_destroy_plan(idct_plan);
    fftw_cleanup();
}

void BlockManager::Block::createPlans() {
    dct_plan = fftw_plan_r2r_2d(height, width, values, values, FFTW_REDFT10, FFTW_REDFT10, 0);
    idct_plan = fftw_plan_r2r_2d(height, width, values, values, FFTW_REDFT01, FFTW_REDFT01, 0);
}

QImage* BlockManager::compress() {
    QImage *out = new QImage(imgWidth, imgHeight, QImage::Format_RGB32);

    QRgb *imageBits = (QRgb*)out->bits();

    parallelTask([&](int i, int j){
        Block &block = getBlock(i, j);

        block.dct2();
        block.cutValues();
        block.idct2();

        for (int pixelRow = 0; pixelRow <  block.height; ++pixelRow) {

            for (int pixelCol = 0; pixelCol < block.width; ++pixelCol) {
                int value = block(pixelRow, pixelCol);
                if (value < 0) value = 0;
                if (value > 255) value = 255;

                int realRow = i * (blockSize * columns + imgWidth % blockSize) * blockSize + pixelRow * (blockSize * columns + imgWidth % blockSize);
                int realCol = j * blockSize + pixelCol;

                imageBits[realRow + realCol] = QColor(value, value, value).rgba();
            }
        }
    });

    return out;
}


void BlockManager::Block::idct2() {
    fftw_execute(idct_plan);

    double denominator = 4 * width * height;

    for(int i = 0; i < height; ++i) {
        for(int j = 0; j < width; ++j) {
            operator()(i, j) = (operator()(i, j) / denominator);
        }
    }
}

void BlockManager::Block::dct2() {
    fftw_execute(dct_plan);
}

void BlockManager::cutFrequencies() {
    for (int i = 0; i < rows * columns; ++i) {
        blocks[i]->cutValues();
    }
}

void BlockManager::Block::setCutDimension(int cutDimension) {
    this->cutDimension = cutDimension;
}

BlockManager::Iterator BlockManager::begin() {
    return BlockManager::Iterator(blockSize, 0, 0, getBlock(0, 0), *this);
}

BlockManager::Iterator BlockManager::end() {
    return BlockManager::Iterator(-1, rows, columns, getBlock(rows - 1, columns - 1), *this);
}

void BlockManager::Block::cutValues() {
    int rowLimit = cutDimension - height;
    if (rowLimit < 0) {
        rowLimit = 0;
    }

    for (int i = rowLimit; i < height; ++i) {
        int colLimit = cutDimension - i;
        if (colLimit < 0) {
            colLimit = 0;
        }

        for (int j = colLimit; j < width; ++j) {
            values[i * width + j] = 0;
        }
    }
}

void BlockManager::setCutDimension(int cutDimension) {
    for (int i = 0; i < rows * columns; ++i) {
        blocks[i]->setCutDimension(cutDimension);
    }
}

void BlockManager::updateImage(const QImage &image) {
    QRgb * imageBits = (QRgb*)image.bits();
    parallelTask([&](int i, int j){
        Block &block = getBlock(i, j);
        for (int pixelRow = 0; pixelRow < block.height; ++pixelRow) {
            for (int pixelCol = 0; pixelCol < block.width; ++pixelCol) {
                int realRow = i * (blockSize * columns + imgWidth % blockSize) * blockSize + pixelRow * (blockSize * columns + imgWidth % blockSize);
                int realCol = j * blockSize + pixelCol;
                block(pixelRow, pixelCol) = qGray(imageBits[realRow + realCol]);
            }
        }
    });
}
