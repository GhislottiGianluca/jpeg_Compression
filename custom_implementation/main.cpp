#include <iostream>
#include <fftw3.h>
#include "timer.h"




void fastDCT2(int N, int M, double *in, double *out){
    fftw_plan plan = fftw_plan_r2r_2d(N, M, in, out,   FFTW_REDFT10,   FFTW_REDFT10, 0);
    fftw_execute(plan);
    fftw_cleanup();
}

void fastIDCT2(int N, int M, double *in, double *out) {
    fftw_plan plan = fftw_plan_r2r_2d(N, M, in, out, FFTW_REDFT01, FFTW_REDFT01, 0);
    fftw_execute(plan);
    fftw_cleanup();
}

void fastDCT(int N, double *in, double *out) {
    fftw_plan plan = fftw_plan_r2r_1d(N, in, out, FFTW_REDFT10, 0);
    fftw_execute(plan);
    fftw_cleanup();
}

void test();

int main() {
    Timer timer;
    /*
    for (int N = 10; N <= 5000; N += 10) {
        double * out = new double[N * N];
        double *in = new double[N * N];
        for (int i = 0; i < N * N; ++i) {
            in[i] = 10;
        }
        
        timer.tic();
        fastDCT2(N, in, out);
        timer.toc();
        std::cout << out[i] << std::endl;

        std::cout << "N = " << N <<  ". Elapsed " << timer.elapsedMilliseconds() << " ms" << std::endl;
        
    }*/

    test();
}


void test() {
        double testIn[64] = {231, 32, 233, 161, 24, 71, 140, 245,
                        247, 40, 248, 245, 124, 204, 36, 107,
                        234, 202, 245, 167, 9, 217, 239, 173,
                        193, 190, 100, 167, 43, 180, 8, 70,
                        11, 24, 210, 177, 81, 243, 8, 112,
                        97, 195, 203, 47, 125, 114, 165, 181,
                        193, 70, 174, 167, 41, 30, 127, 245,
                        87, 149, 57, 192, 65, 129, 178, 228};

    double *testOut = new double[64];
    double *idctOut = new double[64];
    fastDCT2(8, 8, &testIn[0], testOut);
   


     fastIDCT2(8, 8, testOut, idctOut);

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
           idctOut[8 * i + j] /= (4 * sqrt(8 / 2) * sqrt(8 / 2)) * (4 * sqrt(8 / 2) * sqrt(8 / 2));
            /*if (i == 0){
                idctOut[i * 8 + j] /= sqrt(2);
            }
            
            if (j == 0) {
                idctOut[i * 8 + j] /= sqrt(2);
            }*/
            std::cout << idctOut[8 * i + j] << " ";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            testOut[8 * i + j] /= (4 * sqrt(8 / 2) * sqrt(8 / 2));
            if (i == 0){
                testOut[i * 8 + j] /= sqrt(2);
            }
            
            if (j == 0) {
                testOut[i * 8 + j] /= sqrt(2);
            }
            std::cout << testOut[8 * i + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << std::endl;
    double arrTest[8] = {231, 32, 233, 161, 24, 71, 140, 245};
    double *outArr = new double[8];


    fastDCT(8, arrTest, outArr);
    outArr[0] /= sqrt(2);
    for (int i = 0; i < 7; ++i) {
        outArr[i] /= (2 * sqrt(8 / 2));
        std::cout << outArr[i] << ", ";
    }
    std::cout << outArr[7] << std::endl;
}