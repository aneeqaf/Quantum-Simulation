//
//  main.cpp
//  Practice
//
//  Created by Aneeqa Fatima on 9/9/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "graphing.h"
#include "compression.h"

using namespace std;

int main(int argc, char * argv[]) {
    
    static const struct option longopts[] = {
        { "inputfile",    required_argument,       nullptr, 'i' },
        { "max_error",    required_argument,       nullptr, 'e' },
        { "num_codewords",    required_argument,       nullptr, 'n' },
        { "num_codewords",    required_argument,       nullptr, 'q' },
        { "probability",    required_argument,       nullptr, 'p' },
        { nullptr,  0,                 nullptr, '\0' }
    };
    
    string input_filename = "";
    double error_bound = 1e-3;
    int c = 0, idx = 0;
    size_t num_codewords = (1 << 13) - 2;
    size_t num_q = 0;
    double probability = 0;

    while ((c = getopt_long(argc, argv, "i:e:n:q:p:", longopts, &idx)) != -1)
    {
        switch (c) {
            case 'i': {
                if (argc < 2) {
                    cerr << "Please enter filename\n";
                    exit(1);
                }
                input_filename = string(optarg);
                break;
            }
            case 'e': {
                string temp = string(optarg);
                error_bound = stod(temp);
                break;
            }
            case 'p': {
                string temp = string(optarg);
                probability = stod(temp);
                break;
            }
            case 'n': {
                string temp = string(optarg);
                num_codewords = (1 << stoul(temp)) - 1;
                break;
            }
            case 'q': {
                string temp = string(optarg);
                num_q = stoul(temp);
                break;
            }
            default: {
                cerr << "Unknown option " << c << '\n';
                exit(1);
                break;
            }
        }
    }
    
    size_t amp_size = 1ull << (num_q);
    complex<float>* amp = nullptr;
    
    if (posix_memalign((void**)&amp, 64, sizeof(complex<float>) * amp_size) != 0)
        throw "Unable to allocate";
    memset(amp, 0, amp_size * sizeof(complex<float>));
    
    ifstream infile;
    infile.open(input_filename);
    
    char extra = '\0';
//    getline(infile, cmplx_num);
    float real = 0, imag = 0;
    int i = 0;
    while (infile >> real >> imag >> extra) {
        assert(i < amp_size);
        amp[i++] = complex<float>(real, imag);
        if (i == amp_size)
            break;
    }
    infile.close();
    
        PlotCDF(amp, num_q);
//    PlotPT(amp, num_q);
    PlotPointsOnComplexPlane(amp, amp_size);

//    Cramer cramer(amp_size, num_codewords, 4, probability, true);
    
//    ApplyUniformTransformToStateVector(amp, amp_size);
//    PlotUniformSpiralAndAmpDensity("uniform_plot" + to_string(num_codewords), amp, amp_size, cramer);
//    ApplyPTTransformToStateVector(amp, amp_size);
//    CompressDecompressStateVector(amp, amp_size, cramer);
}
