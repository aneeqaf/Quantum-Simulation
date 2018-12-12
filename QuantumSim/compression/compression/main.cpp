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

#include "compression.h"

using namespace std;

int main(int argc, char * argv[]) {
    
    static const struct option longopts[] = {
        { "inputfile",    required_argument,       nullptr, 'i' },
        { "max_error",    required_argument,       nullptr, 'e' },
        { "k_largest",    required_argument,       nullptr, 'k' },
        { nullptr,  0,                 nullptr, '\0' }
    };
    
    string input_filename = "";
    double error_bound = 0;
    int c = 0, idx = 0, exponent = -4;
    idx_size k_largest = 16;

    while ((c = getopt_long(argc, argv, "i:e:k:", longopts, &idx)) != -1)
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
                exponent = stoi(temp);
                error_bound = pow(10, exponent);
                break;
            }
            case 'k': {
                string temp = string(optarg);
                k_largest = stoul(temp);
                break;
            }
            default: {
                cerr << "Unknown option " << c << '\n';
                exit(1);
                break;
            }
        }
    }
                
    
    size_t amp_size = 1ull << 16;
    cmplx* amp = nullptr;
    
    if (posix_memalign((void**)&amp, 64, sizeof(cmplx) * amp_size) != 0)
        throw "Unable to allocate";
    memset(amp, 0, amp_size * sizeof(amp));
    amp[0] = 1;
    
    ifstream infile;
    infile.open(input_filename);
    
    char extra = '\0';
//    getline(infile, cmplx_num);
    float real = 0, imag = 0;
    int i = 0;
    while (infile >> real >> imag >> extra) {
        assert(i < amp_size);
        amp[i++] = cmplx(real, imag);
    }
    infile.close();
    
    map<double, pair<double, double>> mag_phases_pitches;

    pair<amp_idx_t*, cmplx> k_largest_amps = ExtractFractionsOfAmpsFromState(amp, amp_size, k_largest);
    PlotLogSpiralAndAmpDensity("PT_" + input_filename + to_string(k_largest), amp, amp_size, k_largest_amps.second,
                               error_bound, amp_size/k_largest);
    
//    CompressDecompressStateVector(input_filename, amp, amp_size, exponent, k_largest);
    
    ApplyUniformTransformToStateVector(amp, amp_size);
//    ApplyPTTransformToStateVector(amp, amp_size);
    k_largest_amps = ExtractFractionsOfAmpsFromState(amp, amp_size, k_largest);
//    PlotLogSpiralAndAmpDensity("rePT_" +input_filename + to_string(k_largest), amp, amp_size, k_largest_amps.second, error_bound);
    
    PlotUniformSpiralAndAmpDensity("uniform_" + input_filename + to_string(k_largest), amp, amp_size,
                                   k_largest_amps.second, pow(10, exponent/2), amp_size/k_largest);
}
