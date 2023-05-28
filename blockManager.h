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

typedef double pixel;

class BlockManager {
    
public:

    struct Block {
    private:


        int cutDimension;
        fftw_plan dct_plan;
        fftw_plan idct_plan;

    public:
        int height;
        int width;
        pixel *values;
        ~Block();
        Block(int height, int width, int cutDimension);
        void put_row(int row, QRgb *iterator);
        void dct2();
        void idct2();
        void cutValues();
        void setCutDimension(int cutDimension);

        double &operator()(int row, int column) {
            return values[(row * width) + column];
        }
    };

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = pixel;
        using pointer           = pixel*;  // or also value_type*
        using reference         = pixel&;  // or also value_type&

        Iterator(int blockSize, int currRow, int currColumn, Block& currentBlock, BlockManager &mgr): blockSize(blockSize), currColumnInsideBlock(0), currRowInsideBlock(0), currRow(currRow), currColumn(currColumn), mgr(mgr) {
            m_ptr = &currentBlock(currRow, currColumn);
            if(blockSize == -1)
                m_ptr = nullptr;
        }

        reference operator*() const { return *m_ptr; }
        pointer operator->() { return m_ptr; }

        Iterator& operator++() {
            ++currColumnInsideBlock;
            if(currColumnInsideBlock >= blockSize) {
                currColumnInsideBlock = 0;
                ++currColumn;
                if(currColumn >= mgr.columns) {
                    currColumn = 0;
                    ++currRowInsideBlock;
                    if(currRowInsideBlock >= blockSize) {
                        currRowInsideBlock = 0;
                        ++currRow;
                    }
                }

                if(currRow >= mgr.rows)
                    m_ptr = nullptr;
                else {
                    Block& block = mgr.getBlock(currRow, currColumn);
                    m_ptr = &block(currRowInsideBlock, currColumnInsideBlock);
                }
            } else {
                m_ptr++;
            }
            return *this;
        }

        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; }
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; }


    private:
        pointer m_ptr;
        int blockSize;
        int currColumnInsideBlock;
        int currRowInsideBlock;
        int currRow;
        int currColumn;
        int row;
        BlockManager& mgr;
    };


    BlockManager(const QImage *image, int blockSize, int cutDimension);
    ~BlockManager();
    Block& getBlock(int row, int column);
    Iterator begin();
    Iterator end();
    void cutFrequencies();
    void setCutDimension(int cutDimension);
    QImage* compress();

public:
    int rows;
    int columns;
    int imgWidth;
    int imgHeight;

    void updateImage(const QImage &image);

private:
    Block** blocks;
    int blockSize;
    std::thread** workers;
};


#endif
