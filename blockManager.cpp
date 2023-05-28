#include "blockManager.h"
#include <iostream>
#include <fftw3.h>
#include <thread>
BlockManager::BlockManager(const QImage *image, int blockSize, int cutDimension): imgWidth(image->width()), imgHeight(image->height()), blockSize(blockSize) {


    this->rows = floor(imgHeight / blockSize);
    this->columns = floor(imgWidth / blockSize);

    blocks = new Block*[rows * columns];

    for(int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < columns - 1; ++j) {
            blocks[i * columns + j] = new Block(blockSize, blockSize, cutDimension);
        }
    }

    int lastRowHeight = blockSize + imgHeight % blockSize;
    for (int j = 0; j < columns - 1; ++j) {
        blocks[(rows - 1) * columns + j] = new Block(lastRowHeight, blockSize, cutDimension);
    }

    int lastColWidth = blockSize + imgWidth % blockSize;
    for (int i = 0; i < rows - 1; ++i) {
        blocks[i * columns + columns - 1] = new Block(blockSize, lastColWidth, cutDimension);
    }

    blocks[(rows - 1) * columns + columns - 1] = new Block(lastRowHeight, lastColWidth, cutDimension);

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
    dct_plan = fftw_plan_r2r_2d(height, width, values, values, FFTW_REDFT10, FFTW_REDFT10, 0);
    idct_plan = fftw_plan_r2r_2d(height, width, values, values, FFTW_REDFT01, FFTW_REDFT01, 0);
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

QImage* BlockManager::compress() {
    QImage *out = new QImage(imgWidth, imgHeight, QImage::Format_RGB32);
    int coreCount = ceil(sqrt((double) std::thread::hardware_concurrency()));



    int rows = imgHeight / blockSize;
    int cols = imgWidth / blockSize;

    if (rows < coreCount) {
        coreCount = rows;
    }
    if (cols < coreCount) {
        coreCount = cols;
    }

    int rowsPerThread = ceil((double)rows / (double)coreCount);
    int colsPerThread = ceil((double)cols / (double)coreCount);

    workers = new std::thread*[ceil((double)rows / (double)rowsPerThread) * ceil((double)columns / (double)colsPerThread)];

    QRgb *imageBits = (QRgb*)out->bits();

    int threadsCount = 0;
    for (int threadRow = 0; threadRow * rowsPerThread < rows; ++threadRow) {
        for (int threadCol = 0; threadCol * colsPerThread < cols; ++threadCol) {

            workers[threadsCount] = new std::thread([rows, cols, rowsPerThread, colsPerThread, threadCol, threadRow, this, out, imageBits] () {
                for (int i = threadRow * rowsPerThread; i < threadRow * rowsPerThread + rowsPerThread && i < rows; ++i) {
                    for (int j = threadCol * colsPerThread; j < threadCol * colsPerThread + colsPerThread && j < cols; ++j) {
                        Block &block = getBlock(i, j);

                        block.dct2();
                        block.cutValues();
                        block.idct2();

                        for (int pixelRow = 0; pixelRow <  block.height; ++pixelRow) {

                            for (int pixelCol = 0; pixelCol < block.width; ++pixelCol) {
                                int value = block(pixelRow, pixelCol);
                                if (value < 0) value = 0;
                                if (value > 255) value = 255;

                                int realRow = i * (blockSize * cols + imgWidth % blockSize) * blockSize + pixelRow * (blockSize * cols + imgWidth % blockSize);
                                int realCol = j * blockSize + pixelCol;

                                imageBits[realRow + realCol] = QColor(value, value, value).rgba();
                            }
                        }
                    }
                }
            });

            threadsCount++;
        }
    }

    for (int i = 0; i < threadsCount; ++i) {
        workers[i]->join();
        delete workers[i];
    }

    delete[] workers;
    workers = nullptr;


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

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            Block &block = getBlock(i, j);
            for (int pixelRow = 0; pixelRow < block.height; ++pixelRow) {
                for (int pixelCol = 0; pixelCol < block.width; ++pixelCol) {
                    block(pixelRow, pixelCol) = qGray(image.pixel(j * blockSize + pixelCol, i * blockSize + pixelRow));
                }
            }
        }
    }
}
