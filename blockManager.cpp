#include "blockManager.h"
#include <iostream>
#include <fftw3.h>

BlockManager::BlockManager(const QImage *image, int blockSize, int cutDimension): imgWidth(image->width()), imgHeight(image->height()), blockSize(blockSize) {

    this->rows = floor(imgHeight / blockSize);
    this->columns = floor(imgWidth / blockSize);

    for(int i = 0; i < this->rows * this->columns; ++i) {
        blocks.push_back(new Block(blockSize, cutDimension));
    }

    for(int i = 0; i < this->imgHeight - imgHeight % blockSize; ++i) {
        int row = (int)(i / blockSize);
        for(int j = 0; j < this->imgWidth - imgWidth % blockSize; ++j) {
            Block& block = getBlock(row, j / blockSize);
            block(i % blockSize, j % blockSize) = qGray(image->pixel(j, i));
        }
    }

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
}

BlockManager::~BlockManager() {
    for (int i = 0; i < blocks.size(); ++i)
        delete blocks[i];
}

BlockManager::Block::~Block() {
    delete[] values;
}

void BlockManager::apply_dct2() {
    for(auto it = blocks.begin(); it != blocks.end(); ++it)
        (*it)->dct2();
}

void BlockManager::apply_idct2() {
    for(auto it = blocks.begin(); it != blocks.end(); ++it)
        (*it)->idct2();
}

void BlockManager::Block::idct2() {
    fftw_plan plan = fftw_plan_r2r_2d(blockSize, blockSize, values, values, FFTW_REDFT01, FFTW_REDFT01, 0);
    fftw_execute(plan);

    double denominator = 4 * blockSize * blockSize;

    for(int i = 0; i < blockSize; ++i) {
        for(int j = 0; j < blockSize; ++j) {
            operator()(i, j) = (operator()(i, j) / denominator);
        }
    }
}

void BlockManager::Block::dct2() {
    fftw_plan plan = fftw_plan_r2r_2d(blockSize, blockSize, values, values, FFTW_REDFT10, FFTW_REDFT10, 0);
    fftw_execute(plan);
}

void BlockManager::cutFrequencies() {
    for (Block *block : blocks) {
        block->cutValues();
    }
}

BlockManager::Iterator BlockManager::begin() {
    return BlockManager::Iterator(blockSize, 0, 0, getBlock(0, 0), *this);
}

BlockManager::Iterator BlockManager::end() {
    return BlockManager::Iterator(-1, rows, columns, getBlock(rows - 1, columns - 1), *this);
}

void BlockManager::Block::cutValues() {
    int rowLimit = cutDimension - blockSize + 1;
    if (rowLimit < 0) {
        rowLimit = 0;
    }

    for (int i = rowLimit; i < blockSize; ++i) {
        int colLimit = cutDimension - i + 1;
        if (colLimit < 0) {
            colLimit = 0;
        }

        for (int j = colLimit; j < blockSize; ++j) {
            values[i * blockSize + j] = 0;
        }
    }
}
