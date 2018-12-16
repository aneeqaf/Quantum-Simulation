//
//  compression.cpp
//  compression
//
//  Created by Aneeqa Fatima on 12/6/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "compression.h"

cmplx DotProduct(const cmplx* vect_A,
                 const cmplx* vect_B,
                 idx_size state_vector_size)
{
    
    cmplx product = 0;
    
    // Loop for calculate cot product
    for (int i = 0; i < state_vector_size; i++)
        product +=  conj(vect_A[i]) * vect_B[i];
    return product;
}

double CalculateNorm(const cmplx* amp,
                     idx_size state_vector_size)
{
    double norm_ = 0;
    
    for (idx_size i = 0; i < state_vector_size; ++i)
        norm_ += norm(amp[i]);
    
    return sqrt(norm_);
}
    

int FindCInCThetaForRInLogSpiral(cmplx amp)
{
    int c = CalculateCInCThetaGivenMagnitudeLogSpiral(abs(amp)) + 4;
    
    if (c % 2 != 0)
        c -= 1;
    
    return c;
}

int FindCInCThetaForRInUniformSpiral(cmplx amp,
                                     double error_bound)
{
    int c =  CalculateCInCThetaGivenMagnitudeUniformSpiral(abs(amp), error_bound) + 4;
    
    if (c % 2 != 0)
        c -= 1;
    
    return c;
}

double CalculateFidelity(const cmplx* original,
                         const cmplx* processed,
                         idx_size state_vector_size)
{
//    dotp_exact_approx = np.vdot(exact_amps, approx_amps) / (np.linalg.norm(exact_amps) * np.linalg.norm(approx_amps))
//    fidelity = round(pow(np.linalg.norm(dotp_exact_approx), 2), 5)

    double norm_product = (CalculateNorm(original, state_vector_size) * CalculateNorm(processed, state_vector_size));
    cmplx dotp_exact_approx = DotProduct(original, processed, state_vector_size);
    return pow(norm(cmplx(dotp_exact_approx.real()/norm_product, dotp_exact_approx.imag()/norm_product)), 2);
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

template<typename function>
void FindCodewordsForPlotting(const function& radius_func,
                              vector<tuple<float, float>>& codewords,
                              int r,
                              int R,
                              int num_codewords,
                              double error_bound)
{
    double c_dist = (double)(R - r)/(double)num_codewords;
    
    for (int i = 0; i < num_codewords; ++i) {
        double angle = (r + (i * c_dist)) * PI;
        codewords.push_back(make_tuple(radius_func(angle, error_bound) * cos(angle),
                             radius_func(angle, error_bound) * sin(angle)));
    }
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
    
//    make_heap(heap_amps.begin(), heap_amps.end(), comp_amp);

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

void PlotLogSpiralAndAmpDensity(const string& filename,
                                const cmplx* state_vector,
                                idx_size state_vector_size,
                                cmplx smallest_amp_in_k_largest,
                                double error_bound,
                                idx_size k_size,
                                int num_codewords,
                                idx_size multiplicant,
                                double fidelity,
                                idx_size num_zero_amps)
{
#ifdef GP
    Gnuplot gp;
#endif
    
    vector<tuple<float, float>> codewords;
    
    vector<pair<float, float>> xy_pts;
    for(size_t i = 0; i < state_vector_size; ++i)
        xy_pts.push_back(make_pair(state_vector[i].real(), state_vector[i].imag()));
    
    
    int r = FindCInCThetaForRInLogSpiral(cmplx(error_bound, error_bound));
    int R = FindCInCThetaForRInLogSpiral(smallest_amp_in_k_largest);
    cmplx max_amp = FindAmpWithMaxMagnitude(state_vector, state_vector_size);
    int max_R = FindCInCThetaForRInLogSpiral(max_amp);
    FindCodewordsForPlotting(RadiusLogSpiral, codewords, r, R, num_codewords, error_bound);
    
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
    //    gp << "set xl 'x(t)' enhanced font 'Latin Modern Math, 20'\n";
    //    gp << "set yl 'y(t)' enhanced font 'Latin Modern Math, 20'\n";
    gp << "set linetype 1 linecolor rgb \"blue\"\n";
    gp << "set linetype 2 linecolor rgb \"light-blue\"\n";
    gp << "a=" << A << "\nb=" << PT_B << "\ncw=" << codewords.size() << "\n q=" << log2(state_vector_size) << "\n";
    gp << "r=" << r << "\n R=" << R << "\n maxR=" << max_R << "\n maxMag=" << abs(max_amp) << "\n k=" << k_size << "\n";
    
    if (fidelity == 0) {
        gp << "title(a, b) = sprintf(\"\\nPT distribution and log spirals (%iq) \\n\\na=%.3f  b=%.3f \\n\\n"
        << "r=%.10f  R=%.4f  maxR/R=%.5f\\n\\n codewords=%i  largest amps=%i" << "\\n\\n \""
        << ",q, a, b, a * exp( b * r * pi), a * exp( b * R * pi), maxMag/(a * exp(b*R*pi)), cw, k) \n";
        
        gp << "set multiplot layout 2,2 columnsfirst title title(a, b) font 'Latin Modern Math,"
        << "20' margins screen MP_LEFT, MP_RIGHT, MP_BOTTOM, MP_TOP spacing screen MP_GAP \n";
    }
    else {
        
        gp << "f=" << fidelity << "\n z=" << num_zero_amps << "\n qs=" << state_vector_size << "\n";
        gp << "title(a, b) = sprintf(\"\\nResults after compression-decompression\\n\\nPT distribution and log spirals (%iq) \\n\\na=%.3f  b=%.3f \\n\\n"
        << "r=%.10f  R=%.4f  maxR/R=%.5f\\n\\n codewords=%i  largest amps=%i \\n\\n fidelity=%.5f  fraction zero amps=%i/%i\""
        << ",q, a, b, a * exp( b * r * pi), a * exp( b * R * pi), maxMag/(a * exp(b*R*pi)), cw, k, f, z, qs) \n";
        
        gp << "set multiplot layout 1,2 title title(a, b) font 'Latin Modern Math, 20'\n";
    }
    
    gp << "x(t) = a*exp(b*t)*cos(t)\n";
    gp << "y(t) = a*exp(b*t)*sin(t)\n";
    //    gp << "x(t) = a*exp(b*t)*(1-(t**2))/(1+(t**2))\n";
    //    gp << "y(t) = a*exp(b*t)*(2*t)/(1+(t**2))\n";
    
    gp << setprecision(2);
    gp << "set title \"Zoom out \"\n";
    gp << "L = " << A * exp(PT_B * max_R * PI) << "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
    gp << "plot " << gp.file1d(xy_pts, "file.dat")
    << " with points pt 7 ps 0.2 lc 'grey'\n";
    
    if (fidelity == 0) {
        gp << "L = " << A * exp(PT_B * max_R * PI) << "\n";
        gp << "set xr[-L:L]\nset yr[-L:L]\n";
        gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
        gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat")
        << " with points pt 7 ps 0.2 lc 'grey', (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
        << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
        << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    }
    
    gp << "set title \"Zoom in \"\n";
    gp << "L = " << error_bound * 5 << "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', "
    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2\n";
    
    if (fidelity == 0) {
        gp << "L = " << error_bound * 5 << "\n";
        gp << "set xr[-L:L]\nset yr[-L:L]\n";
        gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
        gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', "
        << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
        << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
        << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    }
    
#endif
}

void PlotUniformSpiralAndAmpDensity(const string& filename,
                                    const cmplx* state_vector,
                                    idx_size state_vector_size,
                                    cmplx smallest_amp_in_k_largest,
                                    double error_bound,
                                    idx_size k_size,
                                    int num_codewords,
                                    idx_size multiplicant)
{
#ifdef GP
    Gnuplot gp;
#endif
    
    vector<tuple<float, float>> codewords;
    
    vector<pair<float, float>> xy_pts;
    for(size_t i = 0; i < state_vector_size; ++i)
        xy_pts.push_back(make_pair(state_vector[i].real(), state_vector[i].imag()));
    
    int r = FindCInCThetaForRInUniformSpiral(cmplx(error_bound * multiplicant, error_bound * multiplicant), error_bound);//-584;
    int R = FindCInCThetaForRInUniformSpiral(smallest_amp_in_k_largest, error_bound); //30
    cmplx max_amp = FindAmpWithMaxMagnitude(state_vector, state_vector_size);
    int max_R = FindCInCThetaForRInUniformSpiral(max_amp, error_bound);
    FindCodewordsForPlotting(RadiusUniformSpiral, codewords, r, R, num_codewords, error_bound);
    
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
    //    gp << "set xl 'x(t)' enhanced font 'Latin Modern Math, 20'\n";
    //    gp << "set yl 'y(t)' enhanced font 'Latin Modern Math, 20'\n";
    gp << "set linetype 1 linecolor rgb \"blue\"\n";
    gp << "set linetype 2 linecolor rgb \"light-blue\"\n";
    gp << "eb=" << error_bound << "\ncw=" << codewords.size() << "\n q=" << log2(state_vector_size) << "\n";
    gp << "r=" << r << "\n R=" << R << "\n maxR=" << max_R << "\n maxMag=" << abs(max_amp) << "\n k=" << k_size << "\n";
    gp << "title(eb) = sprintf(\"\\nUniform distribution and uniform spirals (%iq)\\n\\n r=%.4f  R=%.4f  maxR/R=%.5f\\n\\n codewords=%i  largest amps=%i\""
    << ", q, eb * r * pi, eb * R * pi, maxMag/(eb*R*pi), cw, k) \n";
    gp << "set multiplot layout 2,2 columnsfirst title title(eb) font 'Latin Modern Math, 20' margins screen MP_LEFT, MP_RIGHT, MP_BOTTOM, MP_TOP spacing screen MP_GAP \n";
    
    gp << "x(t) = " << error_bound << "*t*cos(t)\n";
    gp << "y(t) = " << error_bound << "*t*sin(t)\n";
    //    gp << "x(t) = a*exp(b*t)*(1-(t**2))/(1+(t**2))\n";
    //    gp << "y(t) = a*exp(b*t)*(2*t)/(1+(t**2))\n";
    
    gp << setprecision(2);
    gp << "set title \"Zoom out \"\n";
    gp << "L = " << error_bound * max_R * PI << "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
    gp << "plot " << gp.file1d(xy_pts, "file.dat")
    << " with points pt 7 ps 0.2 lc 'grey'\n";
    
    gp << "L = " << error_bound * max_R * PI << "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
    gp << "plot [r*pi:maxR*pi]" << gp.file1d(xy_pts, "file.dat")
    << " with points pt 7 ps 0.2 lc 'grey', (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
    
    gp << "set title \"Zoom in \"\n";
    gp << "L = " << error_bound * (3 * 100)<< "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat")  << " with points pt 7 ps 0.2 lc 'grey', "
    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2\n";
    
    gp << "L = " << error_bound * (3 * 100)<< "\n";
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
    cout << endl;
}

unsigned short MapAmpToCodeword(const amp_idx_t* k_largest_amps,
                                cmplx amp,
                                idx_size amp_idx,
                                idx_size k_largest_size,
                                int r,
                                int R,
                                int num_codewords,
                                double error_bound,
                                double& near_zero_c,
                                idx_size& num_near_zero_amps)
{
    static idx_size idx_k_largest = 0;
    double magnitude = abs(amp);
    double min_radius = RadiusUniformSpiral((r + 2) * PI, error_bound);
    
    assert(idx_k_largest <= k_largest_size);
    
    if (magnitude < min_radius) {
        near_zero_c += CalculateCInCThetaGivenMagnitudeUniformSpiral(magnitude, error_bound);
        ++num_near_zero_amps;
        return 0;
    }
    else if (idx_k_largest < k_largest_size && amp_idx ==  k_largest_amps[idx_k_largest].first) {
        ++idx_k_largest;
        return num_codewords;
    }
    else
        return CalculateNearestCodewordToAmp(amp, r, (double)(R - r)/(double)num_codewords, error_bound);
}

idx_size CompressStateVector(const amp_idx_t* k_largest_amps,
                         int num_codewords,
                         cmplx*& state_vector,
                         idx_size state_vector_size,
                         idx_size k_largest_size,
                         double error_bound,
                         double& near_zero_c,
                         int r,
                         int R)
{
    
    cmplx* compressed_amp = nullptr;
    if (posix_memalign((void**)&compressed_amp, 64, sizeof(unsigned short) * state_vector_size) != 0)
        throw "Unable to allocate";

    memset(compressed_amp, 0, sizeof(unsigned short) * state_vector_size);
    
    size_t i = 0;
    
    idx_size num_near_zero_amps = 0;
    
    for (idx_size c_idx = 0; c_idx < state_vector_size/8; ++c_idx) {
        Packed8ShortArray temp = {0};
        temp[0] = MapAmpToCodeword(k_largest_amps, state_vector[i], i, k_largest_size, r, R, num_codewords,
                                   error_bound, near_zero_c, num_near_zero_amps);
        temp[1] = MapAmpToCodeword(k_largest_amps, state_vector[i + 1], i + 1, k_largest_size, r, R, num_codewords,
                                    error_bound, near_zero_c, num_near_zero_amps);
        temp[2] = MapAmpToCodeword(k_largest_amps, state_vector[i + 2], i + 2, k_largest_size, r, R, num_codewords,
                                    error_bound, near_zero_c, num_near_zero_amps);
        temp[3] = MapAmpToCodeword(k_largest_amps, state_vector[i + 3], i + 3, k_largest_size, r, R, num_codewords,
                                    error_bound, near_zero_c, num_near_zero_amps);
        temp[4] = MapAmpToCodeword(k_largest_amps, state_vector[i + 4], i + 4, k_largest_size, r, R, num_codewords,
                                   error_bound, near_zero_c, num_near_zero_amps);
        temp[5] = MapAmpToCodeword(k_largest_amps, state_vector[i + 5], i + 5, k_largest_size, r, R, num_codewords,
                                   error_bound, near_zero_c, num_near_zero_amps);
        temp[6] = MapAmpToCodeword(k_largest_amps, state_vector[i + 6], i + 6, k_largest_size, r, R, num_codewords,
                                   error_bound, near_zero_c, num_near_zero_amps);
        temp[7] = MapAmpToCodeword(k_largest_amps, state_vector[i + 7], i + 7, k_largest_size, r, R, num_codewords,
                                   error_bound, near_zero_c, num_near_zero_amps);
        
        
        compressed_amp[c_idx] = *(cmplx*)temp;
        i += 8;
    }

    near_zero_c /= num_near_zero_amps;
    
    free(state_vector);
    state_vector = nullptr;
    state_vector = compressed_amp;
    
    return num_near_zero_amps;
}

void DecompressStateVector(const amp_idx_t* k_largest_amps,
                           cmplx*& state_vector,
                           idx_size k_largest_size,
                           idx_size state_vector_size,
                           idx_size num_codewords,
                           int r,
                           int R,
                           double error_bound,
                           double near_zero_c)
{
    cmplx* decompressed_amp = nullptr;
    if (posix_memalign((void**)&decompressed_amp, 64, sizeof(cmplx) * state_vector_size) != 0)
        throw "Unable to allocate";
    memset(decompressed_amp, 0, sizeof(cmplx) * state_vector_size);
    
    unsigned short * __restrict compressed_t_amp = (unsigned short *)state_vector;
    
    idx_size large_amps_idx = 0;
    double cw_dist = (double)(R - r)/(double)num_codewords;
    
    for (idx_size i = 0; i < state_vector_size; ++i) {
        
        if (compressed_t_amp[i] == 0) {
            double magnitude = RadiusUniformSpiral(near_zero_c * PI, error_bound);
            
            decompressed_amp[i] = cmplx(magnitude * cos(near_zero_c * PI), magnitude * sin(near_zero_c * PI));
        }
        else if (compressed_t_amp[i] != num_codewords) {
            double c = CalculateCforCodewordInCTheta(compressed_t_amp[i], r, cw_dist);
            double magnitude = RadiusUniformSpiral(c * PI, error_bound);
            
            decompressed_amp[i] = cmplx(magnitude * cos(c * PI), magnitude * sin(c * PI));
        }
        else
            decompressed_amp[i] = k_largest_amps[large_amps_idx++].second;

//        assert(large_amps_idx < k_largest_size);
    }

    free(state_vector);
    state_vector = nullptr;
    state_vector = decompressed_amp;
}

void CompressDecompressStateVector(const string& filename,
                                   cmplx* state_vector,
                                   idx_size state_vector_size,
                                   int error_exponent,
                                   idx_size kth_fraction_of_amps,
                                   int num_codewords)
{
    double error_bound = pow(10, error_exponent);

    vector<cmplx> copy_state_vector(state_vector_size);
    for (idx_size i = 0; i < state_vector_size; ++i)
        copy_state_vector[i] = state_vector[i];
    
    ApplyUniformTransformToStateVector(state_vector, state_vector_size);
    pair<amp_idx_t*, cmplx> k_largest_amps = ExtractFractionsOfAmpsFromState(state_vector, state_vector_size, kth_fraction_of_amps);
    
//    PlotUniformSpiralAndAmpDensity("uniform_" + filename + to_string(kth_fraction_of_amps), state_vector, state_vector_size,
//                                   k_largest_amps.second, pow(10, error_exponent), state_vector_size/kth_fraction_of_amps,
//                                   num_codewords, 90);

    int r = FindCInCThetaForRInUniformSpiral(cmplx(error_bound * 300, error_bound * 300), error_bound);
    int R = FindCInCThetaForRInUniformSpiral(FindAmpWithMaxMagnitude(state_vector, state_vector_size), error_bound);
    
    double near_zero_c = 0;
    
    idx_size num_zero_amps = CompressStateVector(k_largest_amps.first, num_codewords, state_vector, state_vector_size,
                                                 state_vector_size/kth_fraction_of_amps, error_bound, near_zero_c, r, R);

    
    DecompressStateVector(k_largest_amps.first, state_vector, state_vector_size/kth_fraction_of_amps,
                          state_vector_size, num_codewords, r, R, error_bound, near_zero_c);
    ApplyPTTransformToStateVector(state_vector, state_vector_size);
    
    double fidelity = CalculateFidelity(copy_state_vector.data(), state_vector, state_vector_size);
    cout << setprecision(3);
    cout << "Results after compression-decompression of " << log2(state_vector_size) << "q\n";
    cout << "R max = " << FindCInCThetaForRInUniformSpiral(FindAmpWithMaxMagnitude(state_vector, state_vector_size), error_bound) * error_bound * PI << endl;
    cout << "Fraction of amps mapped to zero : " << num_zero_amps << "/" << state_vector_size
    << " (" << (double)num_zero_amps/(double)state_vector_size << ")" << endl;
    cout << "Largest stored amps : " << state_vector_size/kth_fraction_of_amps << endl;
    cout << "Number of codewords : " << num_codewords << endl;
    cout << "Fidelity of Compression : " << fidelity << endl;
    cout << "Compression ratio : " << (double)((sizeof(complex<float>) * (double)state_vector_size) + (sizeof(complex<float>)
                                    * ((double)state_vector_size/(double)kth_fraction_of_amps))
                                    + (sizeof(idx_size) * ((double)state_vector_size/(double)kth_fraction_of_amps)))
                                    /(double)((log2(num_codewords)/8) * (double)state_vector_size) << endl;
   
//    k_largest_amps = ExtractFractionsOfAmpsFromState(state_vector, state_vector_size, kth_fraction_of_amps);
//    PlotLogSpiralAndAmpDensity("rePT_" + filename + to_string(kth_fraction_of_amps), state_vector,
//                               state_vector_size, k_largest_amps.second, pow(10, error_exponent), state_vector_size/kth_fraction_of_amps,
//                               num_codewords, 90, fidelity, num_zero_amps);
}
