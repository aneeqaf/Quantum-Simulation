//
//  compression.cpp
//  compression
//
//  Created by Aneeqa Fatima on 12/6/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#include "compression.h"

inline double radiusLogSpiral(double angle,
                              double error_bound)
{
    return A * exp(PT_B * angle);
}

inline double radiusUniformSpiral(double angle,
                                  double error_bound)
{
    return error_bound * angle;
}

cmplx DotProduct(const cmplx* vect_A,
                 const cmplx* vect_B,
                 idx_size size)
{
    
    cmplx product = 0;
    
    // Loop for calculate cot product
    for (int i = 0; i < size; i++)
        product +=  conj(vect_A[i]) * vect_B[i];
    return product;
}

double CalculateNorm(const cmplx* amp,
                     idx_size size)
{
    double norm_ = 0;
    
    for (idx_size i = 0; i < size; ++i)
        norm_ += norm(amp[i]);
    
    return sqrt(norm_);
}
    

int FindCInCThetaForRInLogSpiral(cmplx amp)
{
    float x = sqrt(norm(amp));
    int c = ((1/(PT_B * PI)) * log(x/A)) + 4;
    
    if (c % 2 != 0)
        c -= 1;
    
    return c;
}

int FindCInCThetaForRInUniformSpiral(cmplx amp,
                                     double error_bound)
{
    float x = sqrt(norm(amp));
    int c =  (x / (error_bound * PI)) + 4;
    
    if (c % 2 != 0)
        c -= 1;
    
    return c;
}

double CalculateFidelity(const cmplx* original,
                         const cmplx* processed,
                         idx_size size)
{
//    dotp_exact_approx = np.vdot(exact_amps, approx_amps) / (np.linalg.norm(exact_amps) * np.linalg.norm(approx_amps))
//    fidelity = round(pow(np.linalg.norm(dotp_exact_approx), 2), 5)

    double norm_product = (CalculateNorm(original, size) * CalculateNorm(processed, size));
    cmplx dotp_exact_approx = DotProduct(original, processed, size);
    return pow(norm(cmplx(dotp_exact_approx.real()/norm_product, dotp_exact_approx.imag()/norm_product)), 2);
}

cmplx FindAmpWithMaxMagnitude(const cmplx* state_vector,
                              idx_size amp_size)
{
    float amp_real = 0, amp_imag = 0;
    
    for (idx_size i = 0; i < amp_size; ++i) {
        if (amp_real > amp_imag) {
            if (amp_real < state_vector[i].real() || amp_real < state_vector[i].imag()) {
                amp_real = state_vector[i].real();
                amp_imag = state_vector[i].imag();
            }
        }
        else {
            if (amp_imag < state_vector[i].real() || amp_imag < state_vector[i].imag()) {
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
pair<double, double> FindCodewordsForPlotting(const function& radius_func,
                                              vector<tuple<float, float, double>>& codewords,
                                              int r,
                                              int R,
                                              double error_bound)
{
    double angle = r * PI;
    double radius = radius_func(angle, error_bound) * cos(angle);
    double prev_radius = 0;
    bool move_next = false;
    double max_error = 1e-12, min_error = 100000;
    
    while (angle < R * PI) {
        double error = 0;
        error = radius - prev_radius; //> max_error ? max_error : radius - prev_radius;
        
        if (error > max_error)
            max_error = error;
        
        if (error < min_error)
            min_error = error;
        
        double t_angle = error / radius;
        if (t_angle > (r + 0.5) * PI)
            angle += 0.01 * PI;
        else
            angle += t_angle;
        
        if (angle < (r + 0.5) * PI)
            codewords.push_back(make_tuple(radius_func(angle, error_bound) * cos(angle), radius_func(angle, error_bound) * sin(angle), error));
        else move_next = true;
        
        if (move_next) {
            r += 2;
            angle = r * PI;
            prev_radius = radius;
            radius = radius_func(angle, error_bound) * cos(angle);
            move_next = false;
        }
    }
    
    return make_pair(min_error, max_error);
}

template<typename function>
void FindCodewordsForCompression(const function& radius_func,
                                 vector<float>& codewords,
                                 int r,
                                 int R,
                                 double error_bound)
{
    float angle = r * PI;
    double radius = radius_func(angle, error_bound) * cos(angle);
    double prev_radius = 0;
    bool move_next = false;
    double max_error = 1e-12, min_error = 100000;
    
    codewords.push_back(0);
    
    while (angle < R * PI) {
        double error = 0;
        error = radius - prev_radius; //> max_error ? max_error : radius - prev_radius;
       
        double t_angle = error / radius;
        if (t_angle > (r + 0.5) * PI)
            angle += 0.01 * PI;
        else
            angle += t_angle;
        
        if (angle < (r + 0.5) * PI)
            codewords.push_back(angle/PI);
        else move_next = true;
        
        if (move_next) {
            r += 2;
            angle = r * PI;
            prev_radius = radius;
            radius = radius_func(angle, error_bound) * cos(angle);
            move_next = false;
        }
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
    memset(k_amps, 0, fraction_amps * sizeof(k_amps));
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
                                idx_size size,
                                cmplx smallest_amp_in_k_largest,
                                double error_bound,
                                idx_size k_size)
{
#ifdef GP
    Gnuplot gp;
#endif
    
    vector<tuple<float, float, double>> codewords;
    
    vector<pair<float, float>> xy_pts;
    for(size_t i = 0; i < size; ++i) {
        float amp_real = state_vector[i].real(), amp_imag = state_vector[i].imag();
        if (amp_real < 0)
            amp_real *= -1;
        if (amp_imag < 0)
            amp_imag *= -1;
        
        xy_pts.push_back(make_pair(amp_real, amp_imag));
    }
    
    int r = FindCInCThetaForRInLogSpiral(cmplx(error_bound, error_bound));//-584;
    int R = FindCInCThetaForRInLogSpiral(smallest_amp_in_k_largest); //30
    cmplx max_amp = FindAmpWithMaxMagnitude(state_vector, size);
    int max_R = FindCInCThetaForRInLogSpiral(max_amp);
    pair<double, double> error = FindCodewordsForPlotting(radiusLogSpiral, codewords, r, R, error_bound);
    
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
    gp << "a=" << A << "\nb=" << PT_B << "\ncw=" << codewords.size() << "\n e=" << error.second << "\n m=" << error.first << "\n";
    gp << "r=" << r << "\n R=" << R << "\n maxR=" << max_R << "\n maxMag=" << sqrt(norm(max_amp)) << "\n k=" << k_size << "\n";
    gp << "title(a, b) = sprintf(\"\\na=%.3f  b=%.3f \\n\\n r=%.10f  R=%.4f  maxR/R=%.5f\\n\\n codewords=%i  largest amps=%i"
    << "\\n\\n max error=%.6f  min error=%.12f \""
    << ",a, b, a * exp( b * r * pi), a * exp( b * R * pi), maxMag/(a * exp(b*R*pi)), cw, k, e, m) \n";
    gp << "set multiplot layout 2,2 columnsfirst title title(a, b) font 'Latin Modern Math, 20' margins screen MP_LEFT, MP_RIGHT, MP_BOTTOM, MP_TOP spacing screen MP_GAP \n";
    
    gp << "x(t) = a*exp(b*t)*cos(t)\n";
    gp << "y(t) = a*exp(b*t)*sin(t)\n";
    //    gp << "x(t) = a*exp(b*t)*(1-(t**2))/(1+(t**2))\n";
    //    gp << "y(t) = a*exp(b*t)*(2*t)/(1+(t**2))\n";
    
    gp << setprecision(2);
    gp << "set title \"Zoom out \"\n";
    gp << "L = " << A * exp(PT_B * max_R * PI) << "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2, L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat")
    << " with points pt 7 ps 0.2 lc 'grey', (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
    gp << "set title \"Zoom in \"\n";
    gp << "L = " << error_bound * 10 << "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', "
    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
    gp << "set title \"Zoom out: positive \"\n";
    gp << "L = " <<  A * exp(PT_B * max_R * PI) << " \n";
    gp << "set xr[0:L]\nset yr[0:L]\n";
    gp << "set xtics 0 , L/2 , L\nset ytics 0, L/2, L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey',"
    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
    gp << "set title \"Zoom in: positive\"\n";
    gp << "L = " << error_bound * 10 << "\n";
    gp << "set xr[0:L]\nset yr[0:L]\n";
    gp << "set xtics 0 , L/2 , L\nset ytics 0, L/2 , L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', "
    << "(t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'";
#endif
}

void PlotUniformSpiralAndAmpDensity(const string& filename,
                                    const cmplx* state_vector,
                                    idx_size size,
                                    cmplx smallest_amp_in_k_largest,
                                    double error_bound,
                                    idx_size k_size)
{
#ifdef GP
    Gnuplot gp;
#endif
    
    vector<tuple<float, float, double>> codewords;
    
    vector<pair<float, float>> xy_pts;
    for(size_t i = 0; i < size; ++i) {
        float amp_real = state_vector[i].real(), amp_imag = state_vector[i].imag();
        if (amp_real < 0)
            amp_real *= -1;
        if (amp_imag < 0)
            amp_imag *= -1;
        
        xy_pts.push_back(make_pair(amp_real, amp_imag));
    }
    
    int r = FindCInCThetaForRInUniformSpiral(cmplx(error_bound, error_bound), error_bound);//-584;
    int R = FindCInCThetaForRInUniformSpiral(smallest_amp_in_k_largest, error_bound); //30
    cmplx max_amp = FindAmpWithMaxMagnitude(state_vector, size);
    int max_R = FindCInCThetaForRInUniformSpiral(max_amp, error_bound);
    pair<double, double> err = FindCodewordsForPlotting(radiusUniformSpiral, codewords, r, R, error_bound);
    
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
    gp << "eb_real=" << err.first << "\neb=" << error_bound << "\ncw=" << codewords.size() << "\n";
    gp << "r=" << r << "\n R=" << R << "\n maxR=" << max_R << "\n maxMag=" << sqrt(norm(max_amp)) << "\n k=" << k_size << "\n";
    gp << "title(eb) = sprintf(\"\\nerror bound=%.8f \\n\\n r=%.4f  R=%.4f  maxR/R=%.5f\\n\\n codewords=%i  largest amps=%i\""
    << ",eb_real, eb * r * pi, eb * R * pi, maxMag/(eb*R*pi), cw, k) \n";
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
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat")
    << " with points pt 7 ps 0.2 lc 'grey', (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
    gp << "set title \"Zoom in \"\n";
    gp << "L = " << error_bound * 100<< "\n";
    gp << "set xr[-L:L]\nset yr[-L:L]\n";
    gp << "set xtics -L , L/2 , L\nset ytics -L, L/2 , L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', "
    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
    gp << "set title \"Zoom out: positive \"\n";
    gp << "L = " <<  error_bound * max_R * PI << " \n";
    gp << "set xr[0:L]\nset yr[0:L]\n";
    gp << "set xtics 0 , L/2 , L\nset ytics 0, L/2, L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey',"
    << " (t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'\n";
    
    gp << "set title \"Zoom in: positive\"\n";
    gp << "L = " << error_bound * 100 << "\n";
    gp << "set xr[0:L]\nset yr[0:L]\n";
    gp << "set xtics 0 , L/2 , L\nset ytics 0, L/2 , L\nset grid\n";
    gp << "plot [r*pi:maxR*pi] " << gp.file1d(xy_pts, "file.dat") << " with points pt 7 ps 0.2 lc 'grey', "
    << "(t <= R * pi ? x(t): 1/0), (t <= R * pi ? y(t): 1/0) ls 1, "
    << "(t > R * pi ? x(t): 1/0), (t > R * pi ? y(t): 1/0) ls 2, "
    << gp.file1d(codewords, "file.txt") << " using 1:2 with points pt 7 ps 0.4 lc 'red'";
#endif
}

void ApplyUniformTransformToStateVector(cmplx* state_vector,
                                        idx_size size)
{    
    for (idx_size i = 0; i < size; ++i) {
        float PT_probability = norm(state_vector[i]);
        float uniform_probability = 1.0 - exp(-PT_probability * size);
        
        float c = ((1/(PT_B * PI)) * log(sqrt(uniform_probability)/A));
        float uniform_mag = A * exp(PT_B * c * PI);
        state_vector[i] = cmplx(uniform_mag * cos(c * PI), uniform_mag * sin(c * PI));
    }
}

void ApplyPTTransformToStateVector(cmplx* state_vector,
                                   idx_size size)
{
    for (idx_size i = 0; i < size; ++i) {
        float uniform_probability = norm(state_vector[i]);
        float PT_probability = -log(abs(1.0 - uniform_probability)) / size;
        
        float c = ((1/(PT_B * PI)) * log(sqrt(PT_probability)/A));
        float PT_mag = A * exp(PT_B * c * PI);
        state_vector[i] = cmplx(PT_mag * cos(c * PI), PT_mag * sin(c * PI));
        cout << state_vector[i] << endl;
    }
}

unsigned short BinarySearchForClosestRadiusToAmp(const vector<float>& codewords,
                                                 double magnitude,
                                                 double error_bound)
{
    double space_radii = (error_bound * (codewords[1] + 2) * PI) - (error_bound * codewords[1] * PI);
    
    int min = 0;
    idx_size max = codewords.size() - 1; // last index value
    unsigned short guess;
    
    
    while (min <= max){
        guess = (unsigned short)(((max + min) / 2) + 0.5);
        if (guess == 0 || guess == codewords.size())
            throw "Incorrect 0 mapping to codewords";
        
        double codeword_magnitude = error_bound * codewords[guess] * PI;
        double diff_mag = codeword_magnitude - magnitude;
        if (diff_mag >= 0 && diff_mag <= space_radii)
            return guess;
        else if (magnitude > codeword_magnitude)
            min = guess + 1;
        else
            max = guess - 1;
    }
    
    throw "Unable to map amp to codeword within error bound";
}

unsigned short FindClosestCodewordUsingAngle(const vector<float>& codewords,
                                             cmplx amp,
                                             double magnitude,
                                             unsigned short closest_outer_radius,
                                             double error_bound)
{
    double phase = asin(amp.imag()/magnitude);
    unsigned short closest_cw = 0;
    double closest_phase = 100000;
    
    double space_radii = (error_bound * (codewords[1] + 2) * PI) - (error_bound * codewords[1] * PI);
    
    for (idx_size i = closest_outer_radius; i > 0; --i) {
        double codeword_magnitude = error_bound * codewords[i] * PI;
        double phase_diff = abs(phase - (fmod(codewords[i], 2.0)* PI));
                                          
        if (phase_diff < closest_phase && abs(codeword_magnitude - magnitude) < space_radii/2) {
            closest_cw = i;
            closest_phase = phase_diff;
        }
        else if (abs(codeword_magnitude - magnitude) > space_radii)
            break;
    }
    
    return closest_cw;
}

unsigned short MapAmpToCodeword(const amp_idx_t* k_largest_amps,
                                vector<float>& codewords,
                                cmplx amp,
                                idx_size amp_idx,
                                idx_size k_largest_size,
                                int r,
                                double error_bound,
                                idx_size& num_near_zero_amps)
{
    static idx_size idx_k_largest = 0;
    double magnitude = sqrt(norm(amp));
    double min_radius = r * PI * error_bound;
    
    assert(idx_k_largest <= k_largest_size);
    
    if (magnitude < min_radius) {
        codewords[0] += ((1/(PT_B * PI)) * log(magnitude/A));
        ++num_near_zero_amps;
        return 0;
    }
    else if (idx_k_largest < k_largest_size && amp_idx ==  k_largest_amps[idx_k_largest].first) {
        ++idx_k_largest;
        return codewords.size();
    }
    else {
        float amp_real = amp.real(), amp_imag = amp.imag();
        bool neg_real = false, neg_imag = false;
        if (amp_real < 0) {
            amp_real *= -1;
            neg_real = true;
        }
        if (amp_imag < 0) {
            amp_imag *= -1;
            neg_imag = true;
        }
        amp = cmplx(amp_real, amp_imag);
        
        unsigned short cw = FindClosestCodewordUsingAngle(codewords, amp, magnitude,
                                                          BinarySearchForClosestRadiusToAmp(codewords, magnitude, error_bound), error_bound);
        
        if (neg_real)
            cw |= 1 << 15;
        if (neg_imag)
            cw |= 1 << 14;
        
        return cw;
    }
}

void CompressStateVector(const amp_idx_t* k_largest_amps,
                         vector<float>& codewords,
                         cmplx* state_vector,
                         idx_size size,
                         idx_size k_largest_size,
                         double error_bound,
                         int r)
{
    
    cmplx* compressed_amp = nullptr;
    if (int err = posix_memalign((void**)&compressed_amp, 64, sizeof(unsigned short) * size) != 0) {
        idx_size memory = sizeof(unsigned short) * size;
        cerr << "Memory requirement exceeds availiable memory for aligned storage. Requested ";
        if (memory >= (1 << 30)) {
            cerr << memory / (1 << 30) << " GiB \n";
        }
        else if (memory >= (1 << 20)) {
            cerr << memory / (1 << 20) << " MiB \n";
        }
        else if (memory >= (1 << 10)) {
            cerr << memory / (1 << 10) << " KiB \n";
        }
        else
            cerr << memory << " B \n";
        exit(err);
    }
    memset(compressed_amp, 0, sizeof(unsigned short) * size);
    
    size_t i = 0;
    
    idx_size num_near_zero_amps = 0;
    
    cout << codewords.size();
    
    for (size_t c_idx = 0; c_idx < size/4; ++c_idx) {
        Packed4ShortArray temp = {0};
        temp[0] = MapAmpToCodeword(k_largest_amps, codewords, state_vector[i], i, k_largest_size, r, error_bound, num_near_zero_amps);
        temp[1] = MapAmpToCodeword(k_largest_amps, codewords, state_vector[i + 1], i + 1, k_largest_size, r, error_bound, num_near_zero_amps);
        temp[2] = MapAmpToCodeword(k_largest_amps, codewords, state_vector[i + 2], i + 2, k_largest_size, r, error_bound, num_near_zero_amps);
        temp[3] = MapAmpToCodeword(k_largest_amps, codewords, state_vector[i + 3], i + 3, k_largest_size, r, error_bound, num_near_zero_amps);
        
        compressed_amp[c_idx] = *(cmplx*)temp;
        i += 4;
    }
    
    codewords[0] /= num_near_zero_amps;
    
    free(state_vector);
    state_vector = compressed_amp;
}

void DecompressStateVector(const amp_idx_t* k_largest_amps,
                           vector<float>& codewords,
                           cmplx* state_vector,
                           idx_size k_largest_size,
                           idx_size size)
{
    cmplx* decompressed_amp = nullptr;
    if (posix_memalign((void**)&decompressed_amp, 64, sizeof(cmplx) * size) != 0)
        throw "Unable to allocate";
    
    unsigned short * __restrict compressed_t_amp = (unsigned short*)state_vector;
    
    idx_size large_amps_idx = 0;
    
    for (idx_size i = 0; i < size; ++i) {
        
        unsigned short cw = compressed_t_amp[i] & MAX_14BITS;
        
        if (cw != codewords.size()) {
            double magnitude = A * exp(PT_B * codewords[cw] * PI);
            
            float amp_real = compressed_t_amp[i] & (1 << 15) ? -magnitude * cos(codewords[cw] * PI) : magnitude * cos(codewords[cw] * PI);
            float amp_imag = compressed_t_amp[i] & (1 << 14) ? -magnitude * sin(codewords[cw] * PI) : magnitude * sin(codewords[cw] * PI);
            
            decompressed_amp[i] = cmplx(amp_real, amp_imag);
        }
        else
            decompressed_amp[i] = k_largest_amps[large_amps_idx++].second;
        
        assert(large_amps_idx < k_largest_size);
    }

    free(state_vector);
    state_vector = decompressed_amp;
}

void CompressDecompressStateVector(const string& filename,
                                   cmplx* state_vector,
                                   idx_size size,
                                   int error_exponent,
                                   idx_size kth_fraction_of_amps)
{
    vector<cmplx> copy_state_vector(size);
    for (idx_size i = 0; i < size; ++i)
        copy_state_vector[i] = state_vector[i];
    
    ApplyUniformTransformToStateVector(state_vector, size);
    pair<amp_idx_t*, cmplx> k_largest_amps = ExtractFractionsOfAmpsFromState(state_vector, size, kth_fraction_of_amps);
    
    PlotUniformSpiralAndAmpDensity("uniform" + filename + to_string(kth_fraction_of_amps), state_vector, size,
                                   k_largest_amps.second, error_exponent/2, size/kth_fraction_of_amps);
    
    double error_bound = pow(10, error_exponent/2);
    int r = FindCInCThetaForRInUniformSpiral(cmplx(error_bound, error_bound), error_bound);
    int R = FindCInCThetaForRInUniformSpiral(k_largest_amps.second, error_bound);
    
    vector<float> codewords;
    FindCodewordsForCompression(radiusUniformSpiral, codewords, r, R, error_bound);
    
    CompressStateVector(k_largest_amps.first, codewords, state_vector, size, size/kth_fraction_of_amps, error_bound, r);
    DecompressStateVector(k_largest_amps.first, codewords, state_vector, size/kth_fraction_of_amps, size);
    ApplyPTTransformToStateVector(state_vector, size);
    
    cout << endl << "Fidelity of Compression : " << CalculateFidelity(copy_state_vector.data(), state_vector, size) << endl;
   
    PlotLogSpiralAndAmpDensity("rePT_" + filename + to_string(kth_fraction_of_amps), state_vector,
                               size, k_largest_amps.second, pow(10, error_exponent), size/kth_fraction_of_amps);
}
