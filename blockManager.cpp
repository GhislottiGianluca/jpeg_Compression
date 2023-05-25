#include "blockManager.h"
#include <iostream>
#include <fftw3.h>

BlockManager::BlockManager(const QImage *image, int blockSize, int cutDimension): imgWidth(image->width()), imgHeight(image->height()), blockSize(blockSize) {

    this->rows = floor(imgWidth / blockSize);
    this->columns = floor(imgHeight / blockSize);

    for(int i = 0; i < this->rows * this->columns; ++i) {
        blocks.push_back(new Block(blockSize, cutDimension));
    }

    for(int i = 0; i < this->rows; ++i) {
        QRgb *rgb = (QRgb*)image->scanLine(i);

        for(int j = 0; j < this->columns; ++j) {
            Block& block = getBlock(i, j);
            block.put_row(i % blockSize, rgb);
            rgb = rgb + blockSize;
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

void BlockManager::Block::dct2() {
    fftw_plan plan = fftw_plan_r2r_2d(blockSize, blockSize, values, values, FFTW_REDFT10, FFTW_REDFT10, 0);
    fftw_execute(plan);
    fftw_cleanup();
}

void BlockManager::cutFrequencies() {
    for (Block *block : blocks) {
        block->cutValues();
    }
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
