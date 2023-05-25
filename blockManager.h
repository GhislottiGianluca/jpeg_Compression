#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include <vector>
#include <QList>
#include <QImage>
#include <QRgb>
#include <QListIterator>


class BlockManager {
    
public:

    struct Block {
    private:
        double *values;
        int blockSize;
        int cutDimension;

    public:
        ~Block();
        Block(int blockSize, int cutDimension);
        void put_row(int row, QRgb *iterator);
        void dct2();
        void cutValues();

        double &operator()(int row, int column) {
            return values[(row * blockSize) + column];
        }
    };


    BlockManager(const QImage *image, int blockSize, int cutDimension);
    ~BlockManager();
    Block& getBlock(int row, int column);
    void apply_dct2();
    void cutFrequencies();



private:
    std::vector<Block*> blocks;
    int rows;
    int columns;
    int imgWidth;
    int imgHeight;
    int blockSize;

};


#endif
