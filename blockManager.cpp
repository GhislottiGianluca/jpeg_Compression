#include "blockManager.h"
#include <iostream>
#include <fftw3.h>
#include <thread>
BlockManager::BlockManager(const QImage *image, int blockSize, int cutDimension): imgWidth(image->width()), imgHeight(image->height()), blockSize(blockSize) {

    this->rows = floor(imgHeight / blockSize);
    this->columns = floor(imgWidth / blockSize);

    for(int i = 0; i < this->rows * this->columns; ++i) {
        blocks.push_back(new Block(blockSize, cutDimension));
    }

    updateImage(*image);
}


BlockManager::Block& BlockManager::getBlock(int row, int column) {
    return *blocks[(row * this->columns) + column];
}

void BlockManager::Block::put_row(int row, QRgb *iterator) {
    for(int i = 0; i < blockSize; ++i){
        values[(row * blockSize) + i] = qGray(iterator[i]);
    }
}

BlockManager::Block::Block(int blockSize, int cutDimension): blockSize(blockSize), cutDimension(cutDimension), values(nullptr) {
    values = new double[blockSize * blockSize];
    dct_plan = fftw_plan_r2r_2d(blockSize, blockSize, values, values, FFTW_REDFT10, FFTW_REDFT10, 0);
    idct_plan = fftw_plan_r2r_2d(blockSize, blockSize, values, values, FFTW_REDFT01, FFTW_REDFT01, 0);
}

BlockManager::~BlockManager() {
    for (int i = 0; i < blocks.size(); ++i)
        delete blocks[i];
}

BlockManager::Block::~Block() {
    delete[] values;
    fftw_destroy_plan(dct_plan);
    fftw_destroy_plan(idct_plan);
    fftw_cleanup();
}

QImage* BlockManager::compress() {
    QImage *out = new QImage(imgWidth, imgHeight, QImage::Format_RGB32);
    int coreCount = std::thread::hardware_concurrency();
    workers = std::vector<std::thread*>();


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

    QRgb *imageBits = (QRgb*)out->bits();
    for (int threadRow = 0; threadRow * rowsPerThread < rows; ++threadRow) {
        for (int threadCol = 0; threadCol * colsPerThread < cols; ++threadCol) {

            workers.push_back(new std::thread([rows, cols, rowsPerThread, colsPerThread, threadCol, threadRow, this, out, imageBits] () {
                for (int i = threadRow * rowsPerThread; i < threadRow * rowsPerThread + rowsPerThread && i < rows; ++i) {
                    for (int j = threadCol * colsPerThread; j < threadCol * colsPerThread + colsPerThread && j < cols; ++j) {
                        Block &block = getBlock(i, j);
                        for(int k = 0; k < blockSize; ++k) {
                            for(int h = 0; h < blockSize; ++h) {
                                block(k, h) -= 128;
                            }
                        }
                        block.dct2();
                        block.cutValues();
                        block.idct2();

                        for(int k = 0; k < blockSize; ++k) {
                            for(int h = 0; h < blockSize; ++h) {
                                block(k, h) += 128;
                            }
                        }

                        for (int pixelRow = 0; pixelRow <  blockSize; ++pixelRow) {

                            for (int pixelCol = 0; pixelCol < blockSize; ++pixelCol) {
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
            }));
        }
    }



    for (auto thread : workers) {
        thread->join();
        delete thread;
    }

    for (int j = 0; j < imgWidth; ++j) {
        QRgb valueToCopy = out->pixel(j, blockSize * rows - 1);
        for (int i = rows * blockSize; i < imgHeight; ++ i) {
            out->setPixel(j, i, valueToCopy);
        }
    }

    for (int i = 0; i < imgHeight; ++i) {
        QRgb valueToCopy = out->pixel(blockSize * cols - 1, i);
        for (int j = cols * blockSize; j < imgWidth; ++j) {
            out->setPixel(j, i, valueToCopy);
        }
    }


    return out;
}


void BlockManager::Block::idct2() {
    fftw_execute(idct_plan);

    double denominator = 4 * blockSize * blockSize;

    for(int i = 0; i < blockSize; ++i) {
        for(int j = 0; j < blockSize; ++j) {
            operator()(i, j) = (operator()(i, j) / denominator);
        }
    }
}

void BlockManager::Block::dct2() {
    fftw_execute(dct_plan);
}

void BlockManager::cutFrequencies() {
    for (Block *block : blocks) {
        block->cutValues();
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
    int rowLimit = cutDimension - blockSize;
    if (rowLimit < 0) {
        rowLimit = 0;
    }

    for (int i = rowLimit; i < blockSize; ++i) {
        int colLimit = cutDimension - i;
        if (colLimit < 0) {
            colLimit = 0;
        }

        for (int j = colLimit; j < blockSize; ++j) {
            values[i * blockSize + j] = 0;
        }
    }
}

void BlockManager::setCutDimension(int cutDimension) {
    for (Block *block : blocks) {
        block->setCutDimension(cutDimension);
    }
}

void BlockManager::updateImage(const QImage &image) {
    for(int i = 0; i < this->imgHeight - imgHeight % blockSize; ++i) {
        int row = (int)(i / blockSize);
        for(int j = 0; j < this->imgWidth - imgWidth % blockSize; ++j) {
            Block& block = getBlock(row, j / blockSize);
            block(i % blockSize, j % blockSize) = qGray(image.pixel(j, i));
        }
    }
}
