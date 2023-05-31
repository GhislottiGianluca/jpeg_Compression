#include <iostream>
#include <fftw3.h>
#include "timer.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <cassert>


void dct(int N, double *in, double *out, int jump = 1) {

    double *f = new double[N];

    for (int i = 0; i < N; ++i) {
        f[i] = in[i * jump];
    }

    for (int i = 0; i < N; ++i) {

        double a_i = 0;

        for (int j = 0; j < N; ++j) {
            a_i +=  f[j] * cos(i * M_PI * (2 * j + 1) / (2 * N));
        }

        a_i /= (i == 0 ? N : N/2);
        out[i * jump] = a_i;
    }

    delete[] f;
}

void idct(int N, double *in, double *out, int jump = 1) {

    double *f = new double[N];

    for (int i = 0; i < N; ++i) {
        f[i] = in[i * jump];
    }

    for (int i = 0; i < N; ++i) {

        double c_i = 0;
        for (int j = 0; j < N; ++j) {
            c_i += f[j] * cos(j * M_PI * (2 * i + 1) / (2 * N));
        }

        out[i * jump] = c_i;
    }

    delete [] f;
}

void dct2(int N, int M, double *in, double *out) {
    for (int i = 0; i < N * M; ++i) {
        out[i] = 0;
    }

    // Rows
    for (int i = 0; i < N; ++i) {
        dct(M, in + i * M, out + i * M, 1);
    }

    // Columns
    for (int i = 0; i < M; ++i) {
        dct(N, out + i, out + i, M);
    }
}

void idct2(int N, int M, double *in, double *out) {

    for (int i = 0; i < N * M; ++i) {
        out[i] = 0;
    }

    // Rows
    for (int i = 0; i < N; ++i) {
        idct(M, in + i * M, out + i * M, 1);
    }

    // Columns
    for (int i = 0; i < M; ++i) {
        idct(N, out + i, out + i, M);
    }
}

void fastDCT2(int N, int M, double *in, double *out){
    fftw_plan plan = fftw_plan_r2r_2d(N, M, in, out,   FFTW_REDFT10,   FFTW_REDFT10, 0);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
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
    fftw_destroy_plan(plan);
    fftw_cleanup();
}

void test();
void compare();
void testIDCT();

int main() {
    test();
    compare();

    return 0;
}


void compare() {
    std::vector<int> dim;
    std::vector<double> timeSlow;
    std::vector<double> timeFast;

    Timer timer;
    for (int N = 10; N <= 1500; N += 100) {
        double * out = new double[N * N];
        double *in = new double[N * N];
        for (int i = 0; i < N * N; ++i) {
            in[i] = random() * 100;
        }

        timer.tic();
        fastDCT2(N, N, in, out);
        timer.toc();


        std::cout << "Fast. N = " << N <<  ". Elapsed " << timer.elapsedMilliseconds() << " ms" << std::endl;
        timeFast.push_back(timer.elapsedMilliseconds());
        timer.tic();
        dct2(N, N, in, out);
        timer.toc();
        std::cout << "Classic. N = " << N <<  ". Elapsed " << timer.elapsedMilliseconds() << " ms" << std::endl;
        timeSlow.push_back(timer.elapsedMilliseconds());
        dim.push_back(N);


        std::ofstream file("results.csv");
        file << "N,fast,slow\n";
        for (int i = 0; i < dim.size(); ++i) {
            file << dim[i] << "," << timeFast[i] << "," << timeSlow[i] << "\n";
        }
    }
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

    double testAfterDct[64] = {
            1118.750,  44.0221926230179,  75.9190503260943, -138.572410997073,  3.50000000000011,  122.078055188031,   195.04386762363, -101.604905927954,
            77.1900790187901,   114.86820590697, -21.8014421145654,  41.3641350570362,  8.77720597607917,  99.0829620423164,  138.171515732119,  10.9092795340779,
            44.8351536536101, -62.7524463827356,  111.614114216838, -76.3789657981438,  124.422159680707,  95.5984193638183,   -39.82879694885,  58.5237669941392,
            -69.9836647476835, -40.2408944802794, -23.4970508345704, -76.7320593698257,  26.6457749504529, -36.8328289535662,  66.1891484548533,  125.429730617991,
            -109, -43.3430856605738, -55.5436907886514,  8.17347082832753,    30.250,  -28.660243733699,  2.44149822336149, -94.1437025464986,
            -5.38783590517084,  56.6345008912568,   173.02151904115, -35.4234493871322,  32.3878249236356,  33.4576727521862, -58.1167863718725,  19.0225614870112,
            78.8439693119085,  -64.592409552383,   118.67120305115, -15.0904839758581, -137.316927872672, -30.6196662811987, -105.114114216838,  39.8130497068829,
            19.7882438284889, -78.1813408994881,  0.972311859835136,  -72.346418009607, -21.5781632503609,  81.2999035477882,  63.7103782052763,  5.90618071066943
    };

    double arrTest[8] = {231, 32, 233, 161, 24, 71, 140, 245};
    double arrAfterDct[8] = {401.990205104552, 6.60001990553251, 109.167365444296, -112.785578571751, 65.4073772597556, 121.831398036668, 116.656488554865, 28.8004072178305};
    double *outArr = new double[8];

    double *testOut = new double[64];
    double *idctOut = new double[64];
    fastDCT2(8, 8, &testIn[0], testOut);

    fastIDCT2(8, 8, testOut, idctOut);

    double epsilon = 1e-12;

    std::cout << "---- Test FFTW ---- " << std::endl;

    std::cout << "Test DCT2 on matrix: " << std::endl;
    std::cout << "   - DCT2 and IDCT2, same result as input: ";
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            idctOut[8 * i + j] /= 64 * 4;
            assert(std::abs(idctOut[8 * i + j] - testIn[8 * i + j]) < epsilon);
        }
    }
    std::cout<< "passed" << std::endl;

    std::cout << "   - same DCT coefficient as mathlab: " << std::flush;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            testOut[8 * i + j] /= (4 * sqrt(8 / 2) * sqrt(8 / 2));
            if (i == 0){
                testOut[i * 8 + j] /= sqrt(2);
            }

            if (j == 0) {
                testOut[i * 8 + j] /= sqrt(2);
            }
            assert(std::abs(testOut[8 * i + j] - testAfterDct[8 * i + j]) < epsilon);
        }
    }
    std::cout << "passed" << std::endl;



    std::cout << "\nTest DCT on single array: " << std::endl;
    std::cout << "   - right values for DCT transformation: " << std::flush;
    fastDCT(8, arrTest, outArr);
    outArr[0] /= sqrt(2);
    for (int i = 0; i < 8; ++i) {
        outArr[i] /= (2 * sqrt(8 / 2));
        assert(std::abs(outArr[i] - arrAfterDct[i]) < epsilon);
    }
    std::cout << "passed" << std::endl;


    std::cout << "\n\n---- Test Custom DCT and IDCT ---- " << std::endl;
    std::cout << "   - Test DCT2 and IDCT2: " << std::flush;
    dct2(8, 8, testIn, testOut);
    idct2(8, 8, testOut, idctOut);
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            assert(std::abs(idctOut[8 * i + j] - testIn[8 * i + j]) < epsilon);
        }
    }
    std::cout << "passed" << std::endl;

    std::cout << "   - Test DCT and IDCT on single array: " << std::flush;
    double *outArr2 = new double[8];
    dct(8, arrTest, outArr2);
    idct(8, outArr2, outArr2);
    for (int i = 0; i < 7; ++i) {
        assert(std::abs(outArr2[i] - arrTest[i]) < epsilon);
    }
    std::cout << "passed" << std::endl;
}