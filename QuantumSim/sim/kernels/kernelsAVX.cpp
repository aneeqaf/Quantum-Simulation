//
//  kernelsAVX.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/14/18.
//

#include <stdio.h>
#include <fstream>

#include "kernelsAVX.h"

__attribute__((always_inline)) inline void
GetTGatesCount(unsigned short *volatile gate_counts /*8*/,
               volatile bool &negate_Z,
               const volatile idx_size p_gray_code,
               const idx_size *volatile __restrict gray_codes /*8*/,
               const idx_size *volatile __restrict CZ_bitmasks,
               const idx_size *volatile __restrict T_bitmasks /*2*/)
{
    const idx_size volatile bit_idx[8] = {static_cast<idx_size>(__builtin_ctzl(gray_codes[0] ^ p_gray_code)), 0, 1, 0,
                                          static_cast<idx_size>(__builtin_ctzl(gray_codes[4] ^ gray_codes[3])), 0, 1, 0};

    // Number of T gates to be applied on an amp
    // If no T gate is being applied and amplitude has to be negated, then '-1' is wrapped in.
    for (int i = 0; i < 8; ++i)
    {
        gate_counts[i] = __builtin_popcountll(gray_codes[i] & T_bitmasks[0]) + __builtin_popcountll(gray_codes[i] & T_bitmasks[1]);
        if (__builtin_parityl(CZ_bitmasks[bit_idx[i]] & gray_codes[i]) == 1)
            negate_Z = !negate_Z;
        if (negate_Z)
            gate_counts[i] += 4;
        gate_counts[i] %= 8;
    }
    //    cout << "T gate counts : ";
    //    for (int i = 0; i < 8; ++i)
    //        cout << bit_idx[i] << " " << gray_codes[i] << " " << gate_counts[i] << " , ";
    //    cout << endl;
}

__attribute__((always_inline)) inline void
FirstGroupOf8GatesHelper(float *__restrict t_amp,
                         const unsigned short *volatile gate_counts /*8*/,
                         const idx_size *volatile __restrict gray_codes /*8*/)
{
    const __m256 temp_amp0 = _mm256_load_ps(&t_amp[2 * gray_codes[0]]);
    const __m256 temp_amp1 = _mm256_load_ps(&t_amp[2 * gray_codes[7]]);

    //    if (_mm256_movemask_ps(_mm256_cmp_ps(temp_amp0, kzeros, _CMP_EQ_OQ)) == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(temp_amp1, kzeros, _CMP_EQ_OQ))  == 255)
    //        return true;

    __m256 re_amps = _mm256_shuffle_ps(temp_amp0, temp_amp1, 0b10001000);
    __m256 im_amps = _mm256_shuffle_ps(temp_amp0, temp_amp1, 0b11011101);

    const __m256 re_Tgates = {kTGate_re[gate_counts[0]], kTGate_re[gate_counts[1]], kTGate_re[gate_counts[7]],
                              kTGate_re[gate_counts[6]], kTGate_re[gate_counts[3]], kTGate_re[gate_counts[2]], kTGate_re[gate_counts[4]],
                              kTGate_re[gate_counts[5]]};
    const __m256 im_Tgates = {kTGate_im[gate_counts[0]], kTGate_im[gate_counts[1]], kTGate_im[gate_counts[7]],
                              kTGate_im[gate_counts[6]], kTGate_im[gate_counts[3]], kTGate_im[gate_counts[2]], kTGate_im[gate_counts[4]],
                              kTGate_im[gate_counts[5]]};

    const __m256 tim_amps = _mm256_fmadd_ps(re_amps, im_Tgates, _mm256_mul_ps(im_amps, re_Tgates));
    const __m256 tre_amps = _mm256_fmsub_ps(re_amps, re_Tgates, _mm256_mul_ps(im_amps, im_Tgates));

    __m256 first_set = _mm256_shuffle_ps(tre_amps, tim_amps, 0b01000100);
    __m256 second_set = _mm256_shuffle_ps(tre_amps, tim_amps, 0b11101110);
    first_set = _mm256_permute_ps(first_set, 0b11011000);
    second_set = _mm256_permute_ps(second_set, 0b11011000);

    _mm256_store_ps(&t_amp[2 * gray_codes[0]], first_set);
    _mm256_store_ps(&t_amp[2 * gray_codes[7]], second_set);
}

__attribute__((always_inline)) inline void
SecondGroupOf8GatesHelper(float *__restrict t_amp,
                          const unsigned short *volatile gate_counts /*8*/,
                          const idx_size *volatile __restrict gray_codes /*8*/)
{
    const __m256 temp_amp0 = _mm256_load_ps(&t_amp[2 * gray_codes[7]]);
    const __m256 temp_amp1 = _mm256_load_ps(&t_amp[2 * gray_codes[0]]);

    //    if (_mm256_movemask_ps(_mm256_cmp_ps(temp_amp0, kzeros, _CMP_EQ_OQ)) == 255
    //        && _mm256_movemask_ps(_mm256_cmp_ps(temp_amp1, kzeros, _CMP_EQ_OQ))  == 255)
    //        return true;

    // The order of real and imag amps is 2 floats from 1st group of 4, 2 floats from 2nd group of 4, and so on.
    __m256 re_amps = _mm256_shuffle_ps(temp_amp0, temp_amp1, 0b10001000);
    __m256 im_amps = _mm256_shuffle_ps(temp_amp0, temp_amp1, 0b11011101);

    const __m256 re_Tgates = {kTGate_re[gate_counts[7]], kTGate_re[gate_counts[6]], kTGate_re[gate_counts[0]],
                              kTGate_re[gate_counts[1]], kTGate_re[gate_counts[4]], kTGate_re[gate_counts[5]], kTGate_re[gate_counts[3]],
                              kTGate_re[gate_counts[2]]};
    const __m256 im_Tgates = {kTGate_im[gate_counts[7]], kTGate_im[gate_counts[6]], kTGate_im[gate_counts[0]],
                              kTGate_im[gate_counts[1]], kTGate_im[gate_counts[4]], kTGate_im[gate_counts[5]], kTGate_im[gate_counts[3]],
                              kTGate_im[gate_counts[2]]};

    const __m256 tim_amps = _mm256_fmadd_ps(re_amps, im_Tgates, _mm256_mul_ps(im_amps, re_Tgates));
    const __m256 tre_amps = _mm256_fmsub_ps(re_amps, re_Tgates, _mm256_mul_ps(im_amps, im_Tgates));

    __m256 first_set = _mm256_shuffle_ps(tre_amps, tim_amps, 0b01000100);
    __m256 second_set = _mm256_shuffle_ps(tre_amps, tim_amps, 0b11101110);
    first_set = _mm256_permute_ps(first_set, 0b11011000);
    second_set = _mm256_permute_ps(second_set, 0b11011000);

    _mm256_store_ps(&t_amp[2 * gray_codes[7]], first_set);
    _mm256_store_ps(&t_amp[2 * gray_codes[0]], second_set);
}

__attribute__((always_inline)) inline void
ApplyCZTGatesInABlock(float *t_amp,
                      Cramer *cramer,
                      const int num_qubits_amp,
                      const idx_size *volatile __restrict CZ_bitmasks,
                      const idx_size *volatile __restrict T_bitmasks /*2*/,
                      const int num_threads,
                      const idx_size block_begin,
                      const idx_size block_size,
                      const ZeroOptMask &zero_opt_mask)
{
    bool any_CZ = CZ_bitmasks[num_qubits_amp] == 1, any_T = T_bitmasks[0] || T_bitmasks[1];

    if (!any_CZ && !any_T)
        return;

    // Before starting a block compute `negate_Z`
    bool negate_Z = false;
    idx_size prev_gc = 0;

    const idx_size block_end = block_begin + block_size;

    // parity of gray code switches between odd and even
    if (block_begin)
    {
        prev_gc = (block_begin - 1) ^ ((block_begin - 1) >> 1);
        idx_size gate_count = 0;
        for (idx_size i = 0; i < (idx_size)num_qubits_amp; ++i)
        {
            if (((prev_gc & (1ull << i)) == (1ull << i)) && (prev_gc & CZ_bitmasks[i]))
                gate_count += __builtin_popcountll((prev_gc & CZ_bitmasks[i]));
        }
        if ((gate_count >> 1) % 2 == 1)
            negate_Z = true;
    }

    // Use `negate_Z` to enable a Gray-code optimized loop.
    for (idx_size count = block_begin; count + 15 < block_end; count += 16)
    {
        idx_size num_iters = count / 16;
        idx_size offset_idx = num_iters ^ (num_iters >> 1);

        idx_size gc0 = count ^ (count >> 1);
        idx_size gc4 = (count + 4) ^ ((count + 4) >> 1);
        const idx_size gc_first[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
        gc0 = (count + 8) ^ ((count + 8) >> 1);
        gc4 = (count + 12) ^ ((count + 12) >> 1);
        const idx_size gc_second[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};
        idx_size prev_gc1 = gc_first[7];

        unsigned short Tgate_count_1[8] = {0};
        GetTGatesCount(Tgate_count_1, negate_Z, prev_gc, gc_first, CZ_bitmasks, T_bitmasks);

        unsigned short Tgate_count_2[8] = {0};
        GetTGatesCount(Tgate_count_2, negate_Z, prev_gc1, gc_second, CZ_bitmasks, T_bitmasks);

        if (zero_opt_mask.CheckIfAllNonZeroes() || zero_opt_mask.CheckIfBlockIsNotZero(offset_idx * 16, 16))
        {
            if (cramer)
            {
                if ((gc_first[0] & 8) == 0)
                {
                    unsigned short Tgate_count[16] = {
                        Tgate_count_1[0], Tgate_count_1[1], Tgate_count_1[3], Tgate_count_1[2],
                        Tgate_count_1[7], Tgate_count_1[6], Tgate_count_1[4], Tgate_count_1[5],
                        Tgate_count_2[7], Tgate_count_2[6], Tgate_count_2[4], Tgate_count_2[5],
                        Tgate_count_2[0], Tgate_count_2[1], Tgate_count_2[3], Tgate_count_2[2]};
                    cramer->CramerBlockSectorSwitch((complex<float> *)t_amp, Tgate_count, gc_first[0], 16);
                }
                else
                {
                    unsigned short Tgate_count[16] = {
                        Tgate_count_2[7], Tgate_count_2[6], Tgate_count_2[4], Tgate_count_2[5],
                        Tgate_count_2[0], Tgate_count_2[1], Tgate_count_2[3], Tgate_count_2[2],
                        Tgate_count_1[0], Tgate_count_1[1], Tgate_count_1[3], Tgate_count_1[2],
                        Tgate_count_1[7], Tgate_count_1[6], Tgate_count_1[4], Tgate_count_1[5]};
                    cramer->CramerBlockSectorSwitch((complex<float> *)t_amp, Tgate_count, gc_second[7], 16);
                }
            }
            else
            {
                FirstGroupOf8GatesHelper(t_amp, Tgate_count_1, gc_first);
                FirstGroupOf8GatesHelper(t_amp, Tgate_count_2, gc_second);
            }
        }
        //        else {
        //            cout << "\nZero bm : " << zero_opt_mask.print() << ", Idx :" << offset_idx * 16
        //            << " (" << bitset<15>(offset_idx * 16).to_string() << ")" << endl;
        //
        //            for (int i = 0; i < 8; ++i) {
        ////                cout << gc_first[i] << ", " ;
        //                assert(t_amp[2 * gc_first[i]] == 0);
        //            }
        //
        //            for (int i = 0; i < 8; ++i) {
        ////                cout << gc_second[i] << ", " ;
        //                assert(t_amp[2 * gc_second[i]] == 0);
        //            }
        //        }
        prev_gc = gc_second[7];
    }
}

void ApplyBlockOfCZTGatesAVXSeq(cmplx *__restrict amp,
                                const int num_qubits_amp,
                                const idx_size *volatile __restrict CZ_bitmasks,
                                const idx_size *volatile __restrict T_bitmasks /*2*/)
{
    const idx_size amp_size = (1ull << num_qubits_amp);
    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    bool negate_Z = false;
    idx_size prev_gc = 0;

    for (idx_size count = 0; count + 15 < amp_size; count += 16)
    {

        idx_size gc0 = count ^ (count >> 1);
        idx_size gc4 = (count + 4) ^ ((count + 4) >> 1);
        const idx_size gc_first[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};

        unsigned short Tgate_count_1[8] = {0};
        GetTGatesCount(Tgate_count_1, negate_Z, prev_gc, gc_first, CZ_bitmasks, T_bitmasks);
        FirstGroupOf8GatesHelper(t_amp, Tgate_count_1, gc_first);
        prev_gc = gc_first[7];

        gc0 = (count + 8) ^ ((count + 8) >> 1);
        gc4 = (count + 12) ^ ((count + 12) >> 1);
        const idx_size gc_second[8] = {gc0, gc0 ^ 1, gc0 ^ 3, gc0 ^ 2, gc4, gc4 ^ 1, gc4 ^ 3, gc4 ^ 2};

        unsigned short Tgate_count_2[8] = {0};
        GetTGatesCount(Tgate_count_2, negate_Z, prev_gc, gc_second, CZ_bitmasks, T_bitmasks);
        FirstGroupOf8GatesHelper(t_amp, Tgate_count_2, gc_second);
        prev_gc = gc_second[7];
    }
}

// ofstream out("compressed.txt");

pair<idx_size, idx_size>
ApplyBlockOfCZTAndLowQXYHGatesAVX(cmplx *&amp,
                                  Cramer *cramer,
                                  const int num_qubits_amp,
                                  const idx_size *volatile __restrict CZ_bitmasks,
                                  const idx_size *volatile __restrict T_bitmasks,
                                  const idx_size lo_X_bitmask,
                                  const idx_size lo_Y_bitmask,
                                  const idx_size lo_H_bitmask,
                                  const int num_threads,
                                  const int num_high_qubits,
                                  const ZeroOptMask &zero_opt_mask)
{
    // out << " >>>>>>>>>>>>>>>>>>>>>>>>> ";
    const bool lo_H_bitmask_applicable = (lo_H_bitmask & (lo_X_bitmask | lo_Y_bitmask)) == (lo_X_bitmask | lo_Y_bitmask) && ((lo_X_bitmask | lo_Y_bitmask) != 0);
    const int bits_for_blk = num_qubits_amp - num_high_qubits;
    const idx_size amp_size = (1ull << num_qubits_amp);
    const idx_size block_size = amp_size > (1ull << bits_for_blk) ? (1ull << bits_for_blk) : amp_size;
    const idx_size new_lo_H_bitmask = lo_H_bitmask_applicable ? lo_H_bitmask ^ (lo_H_bitmask & (lo_X_bitmask | lo_Y_bitmask)) : lo_H_bitmask;
    const int block_bits = block_size != amp_size ? bits_for_blk : num_qubits_amp;

    pair<idx_size, idx_size> phases;
    cmplx *active_block_amps[num_threads];

    cmplx *decompressed_vector = nullptr;
    if (cramer)
    {
        for (size_t t = 0; t < num_threads; ++t)
        {
            if (posix_memalign((void **)&active_block_amps[t], 64, sizeof(complex<float>) * block_size) != 0)
                throw "Unable to allocate space for decompressed vector";

            memset(active_block_amps[t], 0, sizeof(complex<float>) * block_size);
        }

        if (posix_memalign((void **)&decompressed_vector, 64, sizeof(complex<float>) * amp_size) != 0)
            throw "Unable to allocate space for decompressed vector";
    }

#pragma omp parallel for schedule(guided) num_threads(num_threads)
    for (idx_size block_begin = 0; block_begin < amp_size; block_begin += block_size)
    {
        idx_size num_iters = block_begin / block_size;
        idx_size offset_idx = num_iters ^ (num_iters >> 1);
        idx_size curr_block_offset = offset_idx * block_size;

        if (zero_opt_mask.CheckIfAllNonZeroes() ||
            zero_opt_mask.CheckIfBlockIsNotZero(curr_block_offset, block_size))
        {

#pragma omp critical
            {
                ApplyCZTGatesInABlock((float *)__builtin_assume_aligned(amp, 64), cramer, num_qubits_amp, CZ_bitmasks, T_bitmasks,
                                      num_threads, block_begin, block_size, zero_opt_mask);
            }

            auto active_amp = cramer ? cramer->CramerBlockDecompress(
                                           active_block_amps[(block_begin / block_size) % num_threads],
                                           amp, curr_block_offset, block_size)
                                     : amp + curr_block_offset;

            // #pragma omp critical
            //             {
            //                 out << curr_block_offset << endl;
            //                 for (size_t i = 0; i < block_size; ++i)
            //                 {
            //                     float real = active_amp[i].real();
            //                     float imag = active_amp[i].imag();
            //                     if (abs(active_amp[i].real()) < 1.0e-10)
            //                     {
            //                         real = 0;
            //                     }
            //                     if (abs(active_amp[i].imag()) < 1.0e-10)
            //                     {
            //                         imag = 0;
            //                     }
            //                     out << complex<float>(real, imag) << "\n";
            //                 }
            //                 out << endl;
            //             }

            phases = XYFastTransformLowQ(active_amp,
                                         lo_X_bitmask, lo_Y_bitmask, lo_H_bitmask,
                                         block_bits, num_threads);

            if (new_lo_H_bitmask)
                ApplyHGatesIteratively(active_amp, block_bits, num_threads, new_lo_H_bitmask);

#pragma omp critical
            if (cramer)
            {
                // // This will probably not work ... not sure how the codeword updates work in parallel
                // cramer->CramerBlockCompress(amp, active_block_amps[(block_begin / block_size) % num_threads],
                //                             curr_block_offset, block_size);
                memcpy(decompressed_vector + curr_block_offset, active_amp, sizeof(cmplx) * block_size);
            }
        }
        //        else {
        //            cout << "\nZero bm : " << zero_opt_mask.print() << ", Idx :" << offset_idx * block_size
        //            << " (" << bitset<15>(offset_idx * block_size).to_string() << ")" << endl;
        //            for (idx_size i = offset_idx * block_size; i < (offset_idx * block_size) + block_size ; ++i) {
        ////                cout << amp[i] << ",";
        //                assert(amp[i] == cmplx(0,0));
        //            }
        //        }
    }

    phases.second += __builtin_popcountll(new_lo_H_bitmask);

    if (cramer)
    {
        for (idx_size i = 0; i < num_threads; ++i)
        {
            free(active_block_amps[i]);
        }
        free(amp);
        amp = decompressed_vector;
    }

    return pair<idx_size, idx_size>(phases.first, phases.second);
}

double
CalculateRzPhase(const idx_size idx,
                 const idx_size num_qubits,
                 const double *phases /*num qubits*/)
{
    double acc_phase = 0;

    for (idx_size i = 0; i < num_qubits; ++i)
    {
        if ((idx & (1ull << i)) == (1ull << i))
            acc_phase += phases[i];
        else
            acc_phase -= phases[i];
    }

    return acc_phase;
}

pair<double, double>
CalculateQftCRkPhase(const idx_size idx,
                     const idx_size num_qubits,
                     const idx_size target,
                     const idx_size *CRk_bitmasks)
{
    double control_phase = 0, target_phase = 0;

    for (idx_size i = target; i < num_qubits; ++i)
    {
        // The popcount checks whether there is a target on the index too.
        // Any target is fine since QFT CRk gate, target doesn't affect parameters
        if (i != target && ((idx & (1ull << i)) == (1ull << i)) && ((CRk_bitmasks[i] & (1ull << i)) == (1ull << i)))
            control_phase += M_PI / (1ull << (i - target));
    }

    if ((idx & (1ull << target)) == (1ull << target))
        target_phase = control_phase;

    return pair<double, double>{control_phase, target_phase};
}

void PrepareQFTCRkBitmask(idx_size *CRk_bitmasks,
                          const idx_size cycle_num,
                          const idx_size num_qubits)
{
    static idx_size gates_dropped = 0, crossing_gates = 0;
    QubitPartition qp(QubitPartition::Cuts::Horizontal, num_qubits);
    for (idx_size i = cycle_num; i < num_qubits; ++i)
    {
        if (qp.globalToBlock(i) == qp.globalToBlock(cycle_num) || ((i <= 10)))
        {
            CRk_bitmasks[i] = (1ull << i) | (1ull << cycle_num);
            if (qp.globalToBlock(i) != qp.globalToBlock(cycle_num))
                ++crossing_gates;
        }
        else
            ++gates_dropped;
    }

    //    cout << "gates_dropped: " << gates_dropped << endl;
    //    cout << "crossing_gates: " << crossing_gates << endl;
}

double
CalculateCRzPhaseAndPopulatePhasesMatrix(double *control_phase_sums /* num_qubits */,
                                         double *target_phase_sums /* num_qubits */,
                                         const idx_size idx,
                                         const idx_size num_qubits,
                                         const vector<vector<double>> &phases)
{
    double acc_phase = 0;

    for (idx_size i = 0; i < num_qubits; ++i)
    {
        double row_sum = 0, col_sum = 0;
        for (idx_size j = 0; j < num_qubits; ++j)
        {
            if ((idx & (1ull << j)) == (1ull << j))
            {
                // Calculate row sum assuming the control is set
                row_sum += phases[i][j];

                // Calculate column sum only for the control bits that are set
                if ((idx & (1ull << i)) == (1ull << i))
                    col_sum += phases[j][i];
                else
                    col_sum -= phases[j][i];
            }
            else
                row_sum -= phases[i][j];
        }
        if ((idx & (1ull << i)) == (1ull << i))
            acc_phase += row_sum;

        control_phase_sums[i] = row_sum;
        target_phase_sums[i] = col_sum;
    }

    return acc_phase;
}

pair<__m256, bool>
CalculatePhaseForRzInGroupsOf8(volatile double &acc_phase_first,
                               volatile idx_size &gray_code_idx,
                               const idx_size reg_idx,
                               const idx_size block_begin_idx,
                               const idx_size num_qubits,
                               const double *phases /*num qubits*/)
{
    static constexpr idx_size num_phases = 8;

    __m256 phases_per_idx = {0};
    idx_size i = 0;
    bool all_zeros = true;

    // The gray code idx ensures the continuity of gray codes for a certain number of bits
    // hence must calculate the beginning of block separately.
    if (gray_code_idx == 0)
    {
        phases_per_idx[0] = acc_phase_first;
        ++i;
        ++gray_code_idx;
    }

    for (; i < num_phases; ++i, ++gray_code_idx)
    {
        idx_size gc_idx = block_begin_idx + ((gray_code_idx) ^ (gray_code_idx >> 1));
        idx_size prev_gc_idx = block_begin_idx + (gray_code_idx - 1) ^ ((gray_code_idx - 1) >> 1);

        idx_size bit_flip_idx = __builtin_ctzl(gc_idx ^ prev_gc_idx);
        double phase_to_modify = phases[bit_flip_idx];
        if ((prev_gc_idx & (1ull << bit_flip_idx)) == (1ull << bit_flip_idx))
            acc_phase_first -= (2 * phase_to_modify);
        else
            acc_phase_first += (2 * phase_to_modify);

        if (acc_phase_first != 0)
            all_zeros = false;
        phases_per_idx[gc_idx - reg_idx] = acc_phase_first;
    }

    return pair<__m256, bool>{phases_per_idx, all_zeros};
    ;
}

pair<__m256, bool>
CalculatePhaseForCRzInGroupsOf8(double *volatile control_phase_sums,
                                double *volatile target_phase_sums,
                                /* base phase represent the angel for the index where the first 3 bits are 0 */
                                volatile double &base_phase,
                                const idx_size gray_code_idx,
                                /* begining idx in block of 8 */
                                const idx_size reg_idx,
                                /* begining idx of the larger block */
                                const idx_size block_begin_idx,
                                const idx_size num_qubits,
                                const vector<vector<double>> &phases)
{
    constexpr idx_size num_phases_reg = 8;

    __m256 phases_per_idx = {0};
    bool all_zeros = true;

    if (gray_code_idx != 0)
    {
        idx_size gc_idx = block_begin_idx + (gray_code_idx ^ (gray_code_idx >> 1));
        idx_size prev_gc_idx = gray_code_idx == 0 ? 0 : block_begin_idx + (gray_code_idx - 1) ^ ((gray_code_idx - 1) >> 1);
        idx_size bit_flip_idx = __builtin_ctzl(gc_idx ^ prev_gc_idx);

        for (idx_size j = 0; j < num_qubits; ++j)
        {

            // Fix the sign of phases in controls after a target bit flipped
            // Amend the targets after the flip of the control bit
            // Diagonal always has 0s.
            if ((reg_idx & (1ull << bit_flip_idx)) == (1ull << bit_flip_idx))
            {
                control_phase_sums[j] += (2 * phases[j][bit_flip_idx]);
                if ((reg_idx & (1ull << j)) == (1ull << j))
                    target_phase_sums[j] += phases[bit_flip_idx][j];
                else
                    target_phase_sums[j] -= phases[bit_flip_idx][j];
            }
            else
            {
                control_phase_sums[j] -= (2 * phases[j][bit_flip_idx]);
                if ((reg_idx & (1ull << j)) == (1ull << j))
                    target_phase_sums[j] -= phases[bit_flip_idx][j];
                else
                    target_phase_sums[j] += phases[bit_flip_idx][j];
            }
        }
        target_phase_sums[bit_flip_idx] = -target_phase_sums[bit_flip_idx];

        // Add the new control to base_phase and flip the signs of the target for the other controls
        if ((reg_idx & (1ull << bit_flip_idx)) == (1ull << bit_flip_idx))
            base_phase += control_phase_sums[bit_flip_idx] + (2 * target_phase_sums[bit_flip_idx]);
        else
            base_phase -= control_phase_sums[bit_flip_idx] - (2 * target_phase_sums[bit_flip_idx]);
    }

    phases_per_idx[0] = base_phase;
    for (idx_size i = 1; i < num_phases_reg; ++i)
    {
        double phase = base_phase;

        if ((i & 1) == 1)
        {
            phase += control_phase_sums[0] - (2 * target_phase_sums[0]);
            if (((i & 2) == 2))
                phase += (2 * phases[0][1]);
            if (((i & 4) == 4))
                phase += (2 * phases[0][2]);
        }
        if ((i & 2) == 2)
        {
            phase += control_phase_sums[1] - (2 * target_phase_sums[1]);
            if (((i & 1) == 1))
                phase += (2 * phases[1][0]);
            if (((i & 4) == 4))
                phase += (2 * phases[1][2]);
        }
        if ((i & 4) == 4)
        {
            phase += control_phase_sums[2] - (2 * target_phase_sums[2]);
            if (((i & 1) == 1))
                phase += (2 * phases[2][0]);
            if (((i & 2) == 2))
                phase += (2 * phases[2][1]);
        }

        phases_per_idx[i] = phase;
        if (phase != 0)
            all_zeros = false;
    }

    return pair<__m256, bool>{phases_per_idx, all_zeros};
}

pair<__m256, bool>
CalculatePhaseForQftCRkInGroupsOf8(double &control_phase,
                                   idx_size &gray_code_idx,
                                   /* begining idx in block of 8 */
                                   const idx_size reg_idx,
                                   /* begining idx of the larger block */
                                   const idx_size block_begin_idx,
                                   const idx_size num_qubits,
                                   const idx_size target,
                                   const idx_size *CRk_bitmasks)
{
    static constexpr idx_size num_phases = 8;

    __m256 phases_per_idx = {0};
    bool all_zeros = true;
    idx_size i = 0;

    // The gray code idx ensures the continuity of gray codes for a certain number of bits
    // hence must calculate the beginning of block separately.
    if (gray_code_idx == 0)
    {
        phases_per_idx[0] = (reg_idx & (1ull << target)) == (1ull << target) ? control_phase : 0;
        ++i;
        ++gray_code_idx;
    }

    for (; i < num_phases; ++i, ++gray_code_idx)
    {
        idx_size gc_idx = block_begin_idx + ((gray_code_idx) ^ (gray_code_idx >> 1));
        idx_size prev_gc_idx = gray_code_idx == 0 ? 0 : block_begin_idx + (gray_code_idx - 1) ^ ((gray_code_idx - 1) >> 1);

        idx_size bit_flip_idx = __builtin_ctzl(gc_idx ^ prev_gc_idx);
        if (bit_flip_idx != target)
        {
            // Phase only depends on the control
            // Only subtract if there was contribution from the prev_gc_idx
            if (((gc_idx & (1ull << bit_flip_idx)) == (1ull << bit_flip_idx)) && ((CRk_bitmasks[bit_flip_idx] & (1ull << bit_flip_idx)) == (1ull << bit_flip_idx)))
                control_phase += M_PI / (1ull << (bit_flip_idx - target));
            else if (((prev_gc_idx & (1ull << bit_flip_idx)) == (1ull << bit_flip_idx)) && ((CRk_bitmasks[bit_flip_idx] & (1ull << bit_flip_idx)) == (1ull << bit_flip_idx)))
                control_phase -= M_PI / (1ull << (bit_flip_idx - target));
        }

        double phase = (gc_idx & (1ull << target)) == (1ull << target) ? control_phase : 0;
        phases_per_idx[gc_idx - reg_idx] = phase;
        if (phase > 1e-8)
            all_zeros = false;
    }

    return pair<__m256, bool>{phases_per_idx, all_zeros};
}

void ApplyQftCRkGatesAVX(cmplx *__restrict amp,
                         const idx_size target,
                         const idx_size num_qubits,
                         /* difference between starting and end idx should be multiple of 8 */
                         const idx_size idx_begin,
                         const idx_size idx_end,
                         const idx_size *CRk_bitmasks /* num qubits */)
{
    static constexpr idx_size num_phases = 8;

    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);

    idx_size gray_code_idx = 0;
    idx_size reg_idx = (gray_code_idx) ^ (gray_code_idx >> 1);
    if ((reg_idx & (num_phases >> 1)) == (num_phases >> 1))
        reg_idx = reg_idx ^ (num_phases >> 1);
    reg_idx += idx_begin;

    auto acc_phase_first = CalculateQftCRkPhase(reg_idx, num_qubits, target, CRk_bitmasks);

    for (size_t i = idx_begin; i < idx_end; i += num_phases)
    {
        const auto phases_idx = CalculatePhaseForQftCRkInGroupsOf8(acc_phase_first.first,
                                                                   gray_code_idx,
                                                                   reg_idx,
                                                                   idx_begin,
                                                                   num_qubits,
                                                                   target,
                                                                   CRk_bitmasks);
        if (!phases_idx.second)
            ApplyRzPhasesAVX(t_amp, phases_idx.first, reg_idx);

        reg_idx = (gray_code_idx) ^ (gray_code_idx >> 1);
        if ((reg_idx & (num_phases >> 1)) == (num_phases >> 1))
            reg_idx = reg_idx ^ (num_phases >> 1);
        reg_idx += idx_begin;
    }
}

void ApplyRzGatesAVX(cmplx *__restrict amp,
                     const idx_size num_qubits,
                     /* difference between starting and end idx should be multiple of 8 */
                     const idx_size idx_begin,
                     const idx_size idx_end,
                     const double *phases /* num qubits */)
{
    static constexpr idx_size num_phases = 8;

    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);

    double acc_phase_first = CalculateRzPhase(idx_begin, num_qubits, phases);
    idx_size gray_code_idx = 0;

    for (size_t i = idx_begin; i < idx_end; i += num_phases)
    {
        idx_size reg_idx = (gray_code_idx) ^ (gray_code_idx >> 1);
        if ((reg_idx & (num_phases >> 1)) == (num_phases >> 1))
            reg_idx = reg_idx ^ (num_phases >> 1);
        reg_idx += idx_begin;

        const auto phases_idx = CalculatePhaseForRzInGroupsOf8(acc_phase_first,
                                                               gray_code_idx,
                                                               reg_idx,
                                                               idx_begin,
                                                               num_qubits,
                                                               phases);
        if (!phases_idx.second)
            ApplyRzPhasesAVX(t_amp, phases_idx.first, reg_idx);
    }
}

void ApplyCRzGatesAVX(cmplx *__restrict amp,
                      const idx_size num_qubits,
                      /* difference between starting and end idx should be multiple of 8 */
                      const idx_size idx_begin,
                      const idx_size idx_end,
                      const vector<vector<double>> &phases /* num qubits x num qubits */)
{
    static constexpr idx_size num_phases = 8;

    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    double control_phase_sums[num_qubits];
    double target_phase_sums[num_qubits];

    auto base_phase = CalculateCRzPhaseAndPopulatePhasesMatrix(control_phase_sums,
                                                               target_phase_sums,
                                                               idx_begin,
                                                               num_qubits,
                                                               phases);

    for (size_t i = 0; i + idx_begin < idx_end; i += num_phases)
    {
        idx_size reg_idx = i ^ (i >> 1);
        if ((reg_idx & (num_phases >> 1)) == (num_phases >> 1))
            reg_idx = reg_idx ^ (num_phases >> 1);
        reg_idx += idx_begin;

        const auto phases_idx = CalculatePhaseForCRzInGroupsOf8(control_phase_sums, target_phase_sums,
                                                                base_phase, i, reg_idx,
                                                                idx_begin, num_qubits, phases);

        if (!phases_idx.second)
            ApplyRzPhasesAVX(t_amp, phases_idx.first, reg_idx);
    }
}

void RescaleAndApplyGlobalICounter(cmplx *&__restrict amp,
                                   idx_size &global_factor_power,
                                   idx_size &global_i_counter,
                                   const size_t amp_size,
                                   const size_t num_threads)
{
    float rescaling_factor = 1.0 / pow(2, (global_factor_power / 2));
    if ((global_factor_power % 2) == 1)
        rescaling_factor *= 1.0 / sqrt(2.0);
    global_factor_power = 0;

    const auto i_multiplier = cmplx(pow(ki, global_i_counter));
    global_i_counter = 0;

    //    for (idx_size i = 0; i < amp_size; ++i)
    //        amp[i] *= rescaling_factor * i_multiplier;

    float *__restrict t_amp = (float *)__builtin_assume_aligned(amp, 64);
    const __m256 rescaling = {rescaling_factor, rescaling_factor, rescaling_factor, rescaling_factor,
                              rescaling_factor, rescaling_factor, rescaling_factor, rescaling_factor};

#pragma omp parallel for num_threads(num_threads)
    for (idx_size i = 0; i < amp_size; i += 4)
    {
        amp[i] *= i_multiplier;
        amp[i + 1] *= i_multiplier;
        amp[i + 2] *= i_multiplier;
        amp[i + 3] *= i_multiplier;
        __m256 t = _mm256_load_ps(t_amp + (2 * i));
        t = _mm256_mul_ps(t, rescaling);
        _mm256_store_ps(t_amp + (2 * i), t);
    }
}