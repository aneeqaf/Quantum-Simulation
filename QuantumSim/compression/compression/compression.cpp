//
//  compression.cpp
//  compression
//
//  Created by Aneeqa Fatima on 12/6/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "compression.h"

cmplxd DotProduct(const cmplxd* vect_A,
                 const cmplxd* vect_B,
                 idx_size state_vector_size)
{
    
    cmplxd product = 0;
    
    // Loop for calculate cot product
    for (int i = 0; i < state_vector_size; i++)
        product +=  conj(vect_A[i]) * vect_B[i];
    return product;
}

double CalculateNorm(const cmplxd* amp,
                     idx_size state_vector_size)
{
    double norm_ = 0;
    
    for (idx_size i = 0; i < state_vector_size; ++i)
        norm_ += norm(amp[i]);
    
    return sqrt(norm_);
}

double CalculateFidelity(const cmplxd* original,
                         const cmplxd* processed,
                         idx_size state_vector_size)
{
    double norm_product = (CalculateNorm(original, state_vector_size) * CalculateNorm(processed, state_vector_size));
    cmplxd dotp_exact_approx = DotProduct(original, processed, state_vector_size);
    return pow(norm(cmplxd(dotp_exact_approx.real()/norm_product, dotp_exact_approx.imag()/norm_product)), 2);
}

cmplx FindAmpWithMaxMagnitude(const cmplx* state_vector,
                              idx_size amp_size)
{
    float amp_real = 0, amp_imag = 0;
    
    for (idx_size i = 0; i < amp_size; ++i) {
        if (abs(amp_real) > abs(amp_imag)) {
            if (abs(amp_real) < abs(state_vector[i].real()) || abs(amp_real) < abs(state_vector[i].imag())) {
                amp_real = state_vector[i].real();
                amp_imag = state_vector[i].imag();
            }
        }
        else {
            if (abs(amp_imag) < abs(state_vector[i].real()) || abs(amp_imag) < abs(state_vector[i].imag())) {
                amp_real = state_vector[i].real();
                amp_imag = state_vector[i].imag();
            }
        }
    }
    
    if (amp_real < 0)
        amp_real *= -1;
    if (amp_imag < 0)
        amp_imag *= -1;
    
    return cmplx(amp_real, amp_imag);
}

pair<amp_idx_t*, cmplx> ExtractFractionsOfAmpsFromState(const cmplx* state_vector,
                                                    idx_size amp_size,
                                                    idx_size kth_fraction_of_amps)
{
    const idx_size fraction_starting_amps = amp_size/(kth_fraction_of_amps << 4);
    const idx_size fraction_amps = amp_size/kth_fraction_of_amps;
    
    auto comp_amp = [](const amp_idx_t first, const amp_idx_t second) {
        return norm(first.second) > norm(second.second);
    };
    
    auto comp_idx = [](const amp_idx_t first, const amp_idx_t second) {
        return first.first < second.first;
    };
    
    vector<amp_idx_t> heap_amps;
    for (idx_size i = 0; i < fraction_starting_amps; ++i)
        heap_amps.push_back(make_pair(i, state_vector[i]));
    
    priority_queue<amp_idx_t, state_vector_t, decltype(comp_amp)> pq_k_largest_amps (heap_amps.begin(), heap_amps.end(), comp_amp);
    heap_amps.clear();
    
    for (idx_size i = fraction_starting_amps; i < amp_size; ++i) {
        if (norm(state_vector[i]) > norm(pq_k_largest_amps.top().second)) {
            if (pq_k_largest_amps.size() == fraction_amps)
                pq_k_largest_amps.pop();
            
            pq_k_largest_amps.push(make_pair(i, state_vector[i]));
        }
    }
    
    amp_idx_t* k_amps = nullptr;
    
    if (posix_memalign((void**)&k_amps, 64, sizeof(amp_idx_t) * fraction_amps) != 0)
        throw "Unable to allocate";
    memset(k_amps, 0, fraction_amps * sizeof(amp_idx_t));
    cmplx minimum = pq_k_largest_amps.top().second;
    
    for (idx_size x = 0; x < fraction_amps; ++x) {
        k_amps[x] = pq_k_largest_amps.top();
        
//        if (x == fraction_amps/2)
//            minimum = k_amps[x].second;
        
        pq_k_largest_amps.pop();
    }
    sort(k_amps, k_amps + fraction_amps, comp_idx);
//    for (idx_size x = 0; x < fraction_amps; ++x) {
//        k_amps[x] = temp_k_amps[x].second;
////        cout << k_amps[x] << " ";
//    }
////    cout << endl;
    
    float amp_real = minimum.real(), amp_imag = minimum.imag();
    if (amp_real < 0)
        amp_real *= -1;
    if (amp_imag < 0)
        amp_imag *= -1;
    
    return make_pair(k_amps, cmplx(amp_real, amp_imag));
}

void PlotUniformSpiralAndAmpDensity(const string& filename,
                                    const cmplx* state_vector,
                                    idx_size state_vector_size,
                                    const Cramer& cramer)
{
#ifdef GP
    Gnuplot gp;
#endif
    
    vector<pair<float, float>> codewords;
    
    vector<pair<float, float>> xy_pts;
    for(size_t i = 0; i < state_vector_size; ++i)
        xy_pts.push_back(make_pair(state_vector[i].real(), state_vector[i].imag()));
    

    cmplx max_amp = FindAmpWithMaxMagnitude(state_vector, state_vector_size);
    int max_R = abs(max_amp)/(cramer.GetFactorOfDistBetweenTurns() * PI);
    cramer.GetCodewordsForPlotting(codewords);
    
#ifdef GP
    gp << "if (!exists(\"MP_LEFT\"))   MP_LEFT = .1\n";
    gp <<  "if (!exists(\"MP_RIGHT\"))  MP_RIGHT = .95\n";
    gp << "if (!exists(\"MP_BOTTOM\")) MP_BOTTOM = .1\n";
    gp << "if (!exists(\"MP_TOP\"))    MP_TOP = .8\n";
    gp << "if (!exists(\"MP_GAP\"))    MP_GAP = 0.1\n";
    
    gp << "reset\nset nokey\n";
    gp << "set termoption enhanced\n";
    gp << "set term png size 1000,1000\n";
    gp << "set output '" << filename << ".png'\n";
    gp << "set parametric\nset size ratio -1\nset samples 1e5\n";
    gp << "set tics font 'Times New Roman,18'\n";
    gp << "set linetype 1 linecolor rgb \"blue\"\n";
    gp << "set linetype 2 linecolor rgb \"light-blue\"\n";
    gp << "eb=" << cramer.GetFactorOfDistBetweenTurns() << "\ncw=" << codewords.size() << "\n q=" << log2(state_vector_size) << "\n";
    gp << "r=" << cramer.GetMinInnerRadius() << "\n R=" << cramer.GetMaxOuterRadius()
    << "\n maxR=" << max_R << "\n k=" << cramer.GetNumOfLargestVals() << "\n";
    gp << "title(eb) = sprintf(\"\\nUniform distribution and uniform spirals (%iq)\\n\\n r=%.4f  R=%.4f  maxR/R=%.5f\\n\\n codewords=%i  largest amps=%i\""
    << ", q, eb * r * pi, eb * R * pi, maxMag/(eb*R*pi), cw, k) \n";
    gp << "set multiplot layout 2,2 columnsfirst title title(eb) font 'Latin Modern Math, 20' margins screen MP_LEFT, MP_RIGHT, MP_BOTTOM, MP_TOP spacing screen MP_GAP \n";
    
    gp << "x(t) = " << cramer.GetFactorOfDistBetweenTurns() << "*t*cos(t)\n";
    gp << "y(t) = " << cramer.GetFactorOfDistBetweenTurns() << "*t*sin(t)\n";
    
    gp << setprecision(2);
    gp << "set title \"Zoom out \"\n";
    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * max_R * PI << "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
    gp << "plot " << gp.file1d(xy_pts, "file.dat")
    << " with points pt 7 ps 0.2 lc 'grey'\n";
    
    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * max_R * PI << "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
    gp << "plot [r*pi:maxR*pi]" << gp.file1d(xy_pts, "file.dat")
    << " with points pt 7 ps 0.2 lc 'grey', (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
    
    gp << "set title \"Zoom in \"\n";
    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * (3 * 100)<< "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat")  << " with points pt 7 ps 0.2 lc 'grey', "
    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2\n";
    
    gp << "L = " << cramer.GetFactorOfDistBetweenTurns() * (3 * 100)<< "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', "
    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
#endif
}

void ApplyUniformTransformToStateVector(cmplx* state_vector,
                                        idx_size state_vector_size)
{    
    for (idx_size i = 0; i < state_vector_size; ++i) {
        double original_phase = atan2((double)state_vector[i].imag(), (double)state_vector[i].real());
        original_phase = original_phase < 0 ? original_phase + ( 2.0 * PI) : original_phase;
        double PT_probability = norm(complex<double>(state_vector[i]));
        double uniform_probability = 1.0 - exp(-PT_probability * (double)state_vector_size);
        
        double uniform_mag = sqrt(uniform_probability);
        state_vector[i] = complex<double>(uniform_mag * cos(original_phase), uniform_mag * sin(original_phase));
//        cout << state_vector[i] << "; ";
    }
}

void ApplyPTTransformToStateVector(cmplx* state_vector,
                                   idx_size state_vector_size)
{
    for (idx_size i = 0; i < state_vector_size; ++i) {
        double original_phase = atan2((double)state_vector[i].imag(), (double)state_vector[i].real());
        original_phase = original_phase < 0 ? original_phase + ( 2.0 * PI) : original_phase;
        double uniform_probability = norm(complex<double>(state_vector[i]));
        double PT_probability = -log(abs(1.0 - uniform_probability)) / (double)state_vector_size;
        
        double PT_mag = sqrt(PT_probability);
        state_vector[i] = complex<double>(PT_mag * cos(original_phase), PT_mag * sin(original_phase));
    }
}

void CompressDecompressStateVector(cmplx* state_vector,
                                   idx_size state_vector_size,
                                   Cramer& cramer)
{
    vector<cmplxd> copy_state_vector(state_vector_size);
    for (idx_size i = 0; i < state_vector_size; ++i)
        copy_state_vector[i] = state_vector[i];
    
    
    cmplx* comp_vector = cramer.CramerCompress(state_vector);
    free(state_vector);
    state_vector = nullptr;

    state_vector = cramer.CramerDecompress(comp_vector);
    
    vector<cmplxd> new_state_vector(state_vector_size);
    for (idx_size i = 0; i < state_vector_size; ++i)
        new_state_vector[i] = state_vector[i];
    
    double fidelity = CalculateFidelity(copy_state_vector.data(), new_state_vector.data(), state_vector_size);

    cout << "Results after compression-decompression of " << log2(state_vector_size) << "q\n";
    cout << "r = " << cramer.GetMinInnerRadius() << ", R = " << cramer.GetMaxOuterRadius() << endl;;
    cout << "Fraction of amps mapped to zero : " << cramer.GetNumValsMappedToZero() << "/" << state_vector_size
    << " (" << (double)cramer.GetNumValsMappedToZero()/(double)state_vector_size << ")" << endl;
    cout << "Largest stored amps : " << cramer.GetNumOfLargestVals()
    << " (" << (double)cramer.GetNumOfLargestVals()/(double)state_vector_size << ")" << endl;
    cout << "Number of codewords : " << cramer.GetNumOfCodewords() << endl;
    cout << "Fidelity of Compression : " << fidelity << endl;
    cout << "Compression ratio : " << (double)((64 * (double)state_vector_size))
                                    /(double)(((log2(cramer.GetNumOfCodewords() + 2)) * (double)state_vector_size) + (64 * cramer.GetNumOfLargestVals()))
                                               << endl;
}
