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
                    const int hcut,
                    const int vcut) : full_state(nullptr), total_q(qubits)
{
    sumOfTensors = new SumOfTensorsProductsStateVector(qubits, type, hcut, vcut);
}

AdaptiveStateVector::
~AdaptiveStateVector()
{
    delete full_state;
    delete sumOfTensors;
}

bool AdaptiveStateVector::
ApplyBlockOfDiagGates(string& cz_bits,
                      const bitset<128>* __restrict CZ_bitmasks,
                      const bitset<128> __restrict T_bitmasks[2])
{
    bool terminate = false;
    if (full_state) {
        data_per_cycles.xCZ_H.push_back(0);
        data_per_cycles.xCZ_V.push_back(0);
        data_per_cycles.addends.push_back(0);
        data_per_cycles.memory.push_back(GetMemUsage());
        full_state -> ApplyBlockOfDiagGates(cz_bits, CZ_bitmasks, T_bitmasks);
    }
    else {
        terminate = sumOfTensors -> ApplyBlockOfDiagGates(cz_bits, CZ_bitmasks, T_bitmasks);
        
        if (sumOfTensors -> GetNumAddends() > 10) {
            clock_t begin = clock();
            
            //Add support for finding the cut type
            if (sumOfTensors -> GetStateANumQ() > 4 && sumOfTensors -> GetStateBNumQ() > 4
                && sumOfTensors -> GetSimType() != Config::SimType::LosslessV)
                full_state = sumOfTensors -> ConvertSumOfTensorsToStateAVX();
            else
                full_state = sumOfTensors -> ConvertSumOfTensorsToState();
            clock_t end = clock();
            time_by_category.conversion += double(end - begin) / CLOCKS_PER_SEC;
            
            delete sumOfTensors;
            sumOfTensors = nullptr;
        }
    }
    return terminate;
}

void AdaptiveStateVector::
ApplyNonCGate(const int gate_qubit,
              const Gate::Type gate_type,
              const Gate& g)
{
    if (full_state)
        full_state -> ApplyNonCGate(gate_qubit, gate_type);
    else
        sumOfTensors -> ApplyNonCGate(gate_qubit, gate_type);
}

void AdaptiveStateVector::
ApplyHGateOnAllAmps()
{
    if (full_state)
        full_state -> ApplyHGateOnAllAmps();
    else
        sumOfTensors -> ApplyHGateOnAllAmps();
}

//TODO
void AdaptiveStateVector::
ApplyCGate(const int num_controls,
           const vector<int>& gate_qubits,
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
                          const int th)
{
    if (full_state)
        full_state -> ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, th);
    else
        sumOfTensors -> ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, th);
    
}

cmplx AdaptiveStateVector::
operator[](bitset<128> i) const
{
    if (full_state)
        return (*full_state)[i];
    else
        return (*sumOfTensors)[i];
}

double AdaptiveStateVector::
GetMinProb() const
{
    if (full_state)
        return full_state -> GetMinProb();
    else
        return sumOfTensors -> GetMinProb();
}

double AdaptiveStateVector::
GetMaxProb() const
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


idx_size AdaptiveStateVector::
GetGlobalFactorPower() const
{
    if (full_state)
        return full_state -> GetGlobalFactorPower();
    else
        return sumOfTensors -> GetGlobalFactorPower();
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

idx_size AdaptiveStateVector::
GetNumAddends() const
{
    if (full_state)
        return 0;
    else
        return sumOfTensors -> GetNumAddends();
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

idx_size AdaptiveStateVector::
CountZeroAmp() const
{
    if (full_state)
        return full_state -> CountZeroAmp();
    else
        return sumOfTensors -> CountZeroAmp();
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
