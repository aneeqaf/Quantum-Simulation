//
//  state_autoconv.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/20/18.
//

#include "state_autoconv.h"

AdaptiveStateVector::
AdaptiveStateVector(const int qubits,
                    const Config::SimType type,
                    const Config* config,
                    const int hcut,
                    const int vcut,
                    const bool row_major,
                    const bool first_part_small,
                    const int verb) : full_state(nullptr), total_q(qubits)
{
    sumOfTensors = new SumOfTensorsProductsStateVector(qubits, type, config,
                                                       hcut, vcut, row_major,
                                                       first_part_small, verb);
}

AdaptiveStateVector::
AdaptiveStateVector(const AdaptiveStateVector& rhs)
{
    if (rhs.full_state) {
        full_state = new FullAmpStateVector(*(rhs.full_state));
        sumOfTensors = nullptr;
    }
    else {
        full_state = nullptr;
        sumOfTensors = new SumOfTensorsProductsStateVector(*(rhs.sumOfTensors));
    }
    
    compressed = rhs.compressed;
    total_q = rhs.total_q;
}

AdaptiveStateVector& AdaptiveStateVector::
operator=(const AdaptiveStateVector& rhs)
{
    AdaptiveStateVector temp(rhs);
    if (rhs.full_state)
        swap(full_state, temp.full_state);
    else
        swap(sumOfTensors, temp.sumOfTensors);
    
    compressed = rhs.compressed;
    total_q = rhs.total_q;
    return *this;
}

AdaptiveStateVector::
~AdaptiveStateVector()
{
    if (full_state != nullptr) delete full_state;
    if (sumOfTensors != nullptr) delete sumOfTensors;
}

void AdaptiveStateVector::
ApplyNonCGate(const idx_size gate_qubit,
              const Gate::Type gate_type)
{
    if (full_state)
        full_state -> ApplyNonCGate(gate_qubit, gate_type);
    else
        sumOfTensors -> ApplyNonCGate(gate_qubit, gate_type);
}

void AdaptiveStateVector::
ApplyHGateOnAllAmps(bool cycle_0)
{
    if (full_state)
        full_state -> ApplyHGateOnAllAmps(cycle_0);
    else
        sumOfTensors -> ApplyHGateOnAllAmps(cycle_0);
}

//TODO
void AdaptiveStateVector::
ApplyCGate(const idx_size num_controls,
           const vector<idx_size>& gate_qubits,
           const Gate& g,
           const Gate::Type gate_type)
{
    
}

void AdaptiveStateVector::
ApplyMergedXYGate(const Gate& gate1,
                  const Gate& gate2)
{
    if (full_state)
        full_state -> ApplyMergedXYGate(gate1, gate2);
    else
        sumOfTensors -> ApplyMergedXYGate(gate1, gate2);
}

void AdaptiveStateVector::
ApplyXYRecursiveTransform(bitset<128> X_bitmask,
                          bitset<128> Y_bitmask,
                          int th)
{
    if (full_state)
        full_state -> ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, th);
    else
        sumOfTensors -> ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, th);
    
}

int AdaptiveStateVector::
ApplyLoXYHAndCZTInSamePass(int& remaining_cz_bits,
                           idx_size& cz_path,
                           const idx_size cz_path_len,
                           const idx_size suffix_size,
                           const bitset<128>& X_bitmask,
                           const bitset<128>& Y_bitmask,
                           const bitset<128>& H_bitmask,
                           const bitset<128>* __restrict CZ_bitmasks,
                           const bitset<128> T_bitmasks[2],
                           int th)
{
    int xCZ_applied_in_cycle = -1;
    if (full_state) {
        //        if (book_keep) {
        //            data_per_cycles.xCZ_H.push_back(0);
        //            data_per_cycles.xCZ_V.push_back(0);
        //            data_per_cycles.addends.push_back(0);
        //            data_per_cycles.memory.push_back(GetMemUsage());
        //        }
        full_state -> ApplyLoXYHAndCZTInSamePass(remaining_cz_bits, cz_path,
                                                 cz_path_len, suffix_size,
                                                 X_bitmask,Y_bitmask,
                                                 H_bitmask, CZ_bitmasks,
                                                 T_bitmasks, th);
    }
    else {
        if (cz_path_len == 0)
            sumOfTensors -> ApplyLoXYHAndCZTInSamePass(remaining_cz_bits, cz_path,
                                                       cz_path_len, suffix_size,
                                                       X_bitmask, Y_bitmask,
                                                       H_bitmask, CZ_bitmasks,
                                                       T_bitmasks, th);
        else
            xCZ_applied_in_cycle = sumOfTensors -> ApplyLoXYHAndCZTInSamePass(remaining_cz_bits, cz_path, cz_path_len,
                                                                              suffix_size, X_bitmask, Y_bitmask,
                                                                              H_bitmask, CZ_bitmasks,
                                                                              T_bitmasks, th);
        
        if (sumOfTensors -> GetNumAddends() > 10) {
            Time time;
            time.StartTime();
            
            full_state = sumOfTensors -> ConvertSumOfTensorsToState();
            
            time_by_category.conversion += time.GetElapsedTime();
            
            delete sumOfTensors;
            sumOfTensors = nullptr;
        }
    }
    return xCZ_applied_in_cycle;
}

cmplx AdaptiveStateVector::
operator[](bitset<128> i)
{
    if (full_state)
        return (*full_state)[i];
    else
        return (*sumOfTensors)[i];
}

cmplx AdaptiveStateVector::
GetGlobalAmpAtInterestingIdx(idx_size i)
{
    if (full_state)
        return full_state -> GetGlobalAmpAtInterestingIdx(i);
    else
        return sumOfTensors -> GetGlobalAmpAtInterestingIdx(i);
    
}


double AdaptiveStateVector::
GetMinProb() 
{
    if (full_state)
        return full_state -> GetMinProb();
    else
        return sumOfTensors -> GetMinProb();
}

double AdaptiveStateVector::
GetMaxProb()
{
    if (full_state)
        return full_state -> GetMaxProb();
    else
        return sumOfTensors -> GetMaxProb();
}

double AdaptiveStateVector::
GetAvgProb() const
{
    if (full_state)
        return full_state -> GetAvgProb();
    else
        return sumOfTensors -> GetAvgProb();
}

double AdaptiveStateVector::
GetMemUsage() const
{
    if (full_state)
        return full_state -> GetMemUsage();
    else
        return sumOfTensors -> GetMemUsage();
}

idx_size AdaptiveStateVector::
GetSize() const
{
    if (full_state)
        return full_state -> GetSize();
    else
        return sumOfTensors -> GetSize();
}

idx_size AdaptiveStateVector::
GetFullStateVectorSize() const
{
    if (full_state)
        return full_state -> GetFullStateVectorSize();
    else
        return sumOfTensors -> GetFullStateVectorSize();
}

int AdaptiveStateVector::
GetNumQInBlock(idx_size block) const
{
    if (full_state)
        return full_state -> GetNumQInBlock(block);
    else
        return sumOfTensors -> GetNumQInBlock(block);
}

idx_size AdaptiveStateVector::
GetGlobalFactorPower() const
{
    if (full_state)
        return full_state -> GetGlobalFactorPower();
    else
        return sumOfTensors -> GetGlobalFactorPower();
}

idx_size AdaptiveStateVector::
GetNumAddends() const
{
    if (full_state)
        return 0;
    else
        return sumOfTensors -> GetNumAddends();
}

double AdaptiveStateVector::
CalculateNormSquared()
{
    if (full_state)
        return full_state -> CalculateNormSquared();
    else
        return sumOfTensors -> CalculateNormSquared();
}

double AdaptiveStateVector::
CalculateAverageInaccuracy(double norm) const
{
    if (full_state)
        return full_state -> CalculateAverageInaccuracy(norm);
    else
        return sumOfTensors -> CalculateAverageInaccuracy(norm);
}

double AdaptiveStateVector::
CalculateMeanEntropy() const
{
    if (full_state)
        return full_state -> CalculateMeanEntropy();
    else
        return sumOfTensors -> CalculateMeanEntropy();
}

void AdaptiveStateVector::
ResetAmpVector()
{
    if (full_state)
        return full_state -> ResetAmpVector();
    else
        return sumOfTensors -> ResetAmpVector();
}

void AdaptiveStateVector::
Normalize()
{
    if (full_state)
        full_state -> Normalize();
    else
        sumOfTensors -> Normalize();
}

double AdaptiveStateVector::
CalculateCrossEntropy(int range) const
{
    if (full_state)
        return full_state -> CalculateCrossEntropy(range);
    else
        return sumOfTensors -> CalculateCrossEntropy(range);
}

void AdaptiveStateVector::
Rescale()
{
    if (full_state)
        return full_state -> Rescale();
    else
        return sumOfTensors -> Rescale();
}

void AdaptiveStateVector::
RescaleAndApplyGlobalICounter()
{
    if (full_state)
        return full_state -> RescaleAndApplyGlobalICounter();
    else
        return sumOfTensors -> RescaleAndApplyGlobalICounter();
}

double AdaptiveStateVector::
CountZeroAmpPercentage() const
{
    if (full_state)
        return full_state -> CountZeroAmpPercentage();
    else
        return sumOfTensors -> CountZeroAmpPercentage();
}

idx_size AdaptiveStateVector::
CountZerosInBlock(int block) const
{
    if (full_state)
        return full_state -> CountZerosInBlock(block);
    else
        return sumOfTensors -> CountZerosInBlock(block);
}

bool AdaptiveStateVector::
AreAllAmpsZero() const
{
    if (full_state)
        return full_state -> AreAllAmpsZero();
    else
        return sumOfTensors -> AreAllAmpsZero();
}

void AdaptiveStateVector::
ApplyGlobalICounter()
{
    if (full_state)
        return full_state -> ApplyGlobalICounter();
    else
        return sumOfTensors -> ApplyGlobalICounter();
}

void AdaptiveStateVector::
PrintStateVector(const string& outfile,
                 const int cycle_num)
{
    if (sumOfTensors)
        sumOfTensors -> PrintStateVector(outfile, cycle_num);
    else
        full_state -> PrintStateVector(outfile, cycle_num);
}

void AdaptiveStateVector::
PrintStateVector() 
{
    if (sumOfTensors)
        sumOfTensors -> PrintStateVector();
    else
        full_state -> PrintStateVector();
}

void AdaptiveStateVector::
PrintProbabilities(const string& out_file,
                   const int cycle_num)
{
    if (sumOfTensors)
        sumOfTensors -> PrintProbabilities(out_file, cycle_num);
    else
        full_state -> PrintProbabilities(out_file, cycle_num);
}

void AdaptiveStateVector::
WriteAmpToDisk(const string& filename)
{
    if (full_state)
        full_state -> WriteAmpToDisk(filename);
    else
        sumOfTensors -> WriteAmpToDisk(filename);
}

void AdaptiveStateVector::
ReadFromDisk(const string& filename)
{
    if (full_state)
        full_state -> ReadFromDisk(filename);
    else
        sumOfTensors -> ReadFromDisk(filename);
}

void AdaptiveStateVector::
CopyState(const GenericQuantumState& rhs)
{
    const AdaptiveStateVector& t_rhs = (const AdaptiveStateVector&)rhs;
    if (t_rhs.full_state) {
        full_state -> CopyState(*t_rhs.full_state);
        sumOfTensors = nullptr;
    }
    else {
        full_state = nullptr;
        sumOfTensors -> CopyState(*(t_rhs.sumOfTensors));
    }
    
    total_q = t_rhs.total_q;
}

void AdaptiveStateVector::
CopyMemberVars(const GenericQuantumState& rhs)
{
    const AdaptiveStateVector& t_rhs = (const AdaptiveStateVector&)rhs;
    total_q = t_rhs.total_q;
    compressed = rhs.compressed;
    
    if (full_state)
        full_state -> CopyMemberVars(*t_rhs.full_state);
    else
        sumOfTensors -> CopyMemberVars(*t_rhs.sumOfTensors);
}

void AdaptiveStateVector::
CompressStateVector(idx_size num_codewords,
                    double p_rejection)
{
    if (full_state)
        full_state -> CompressStateVector(num_codewords, p_rejection);
    else
        sumOfTensors -> CompressStateVector(num_codewords, p_rejection);
    
    compressed = true;
}

void AdaptiveStateVector::
DecompressStateVector()
{
    if (full_state)
        full_state -> DecompressStateVector();
    else
        sumOfTensors -> DecompressStateVector();
    
    compressed = false;
}

void AdaptiveStateVector::
DecompressAndCopyAnotherState(const GenericQuantumState& rhs)
{
    const AdaptiveStateVector& t_rhs = (const AdaptiveStateVector&)rhs;
    total_q = t_rhs.total_q;
    
    if (full_state)
        full_state -> DecompressAndCopyAnotherState(*t_rhs.full_state);
    else
        sumOfTensors -> DecompressAndCopyAnotherState(*t_rhs.sumOfTensors);
    
    compressed = false;
}
