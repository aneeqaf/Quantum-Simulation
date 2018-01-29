//
//  state_autoconv.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 1/20/18.
//

#include "state_autoconv.h"

AdaptiveStateVector::
AdaptiveStateVector(const int qubits,
                    const SumOfTensorsProductsStateVector::SimType type,
                    const int cut_size) : full_state(nullptr), total_q(qubits)
{
    sumOfTensors = new SumOfTensorsProductsStateVector(qubits, type, cut_size);
    
    {
        string data = "";
        if (type == SumOfTensorsProductsStateVector::SimType::LosslessH)
             data += "Cut : horizontal ";
        else if (type == SumOfTensorsProductsStateVector::SimType::LosslessV)
            data += "Cut : vertical ";
        data += to_string(sumOfTensors -> GetStateANumQ()) + " + " + to_string(sumOfTensors -> GetStateBNumQ()) + "\n";
        log.push_back(data);
    }
    
    {
        string data = "Cycle\txCZ\tAddends\t  Memory\n";
        log.push_back(data);
    }
}

AdaptiveStateVector::
~AdaptiveStateVector()
{
    delete full_state;
    delete sumOfTensors;
}

void AdaptiveStateVector::
ApplyBlockOfDiagGates(const idx_size* __restrict CZ_bitmasks,
                      const idx_size __restrict T_bitmasks[2])
{
    
    
    if (full_state)
        full_state -> ApplyBlockOfDiagGates(CZ_bitmasks, T_bitmasks);
    else {
        static int cycle_count = 2;
        {
            string data = to_string(cycle_count) + "\t";
            log.push_back(data);
        }
        
        sumOfTensors -> ApplyBlockOfDiagGates(CZ_bitmasks, T_bitmasks);

        double memory = sumOfTensors -> GetMemUsage();
        ostringstream ss;
        ss << setprecision(3) << to_string(sumOfTensors -> GetNumAddends()) << "\t  ";
        if (memory >= 1e9) {
            ss << memory / 1e9 << " GB \n";
        }
        else if (memory >= 1e6) {
            ss << memory / 1e6 << " MB \n";
        }
        else if (memory >= 1e3) {
            ss << memory / 1e3 << " KB \n";
        }
        else
            ss << memory << " B \n";
        string data = ss.str();
        log.push_back(data);
       
        if (sumOfTensors -> GetNumAddends() > 10) {
            clock_t begin = clock();
            full_state = sumOfTensors -> ConvertSumOfTensorsToStateAVX(); //won't work for less than 4 q in a split
            clock_t end = clock();
            time_by_category.conversion += double(end - begin) / CLOCKS_PER_SEC;
            
            delete sumOfTensors;
            sumOfTensors = nullptr;
        }
        cycle_count += 2;
    }
    
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
ApplyXYRecursiveTransform(idx_size X_bitmask,
                          idx_size Y_bitmask,
                          const int th)
{
    if (full_state)
        full_state -> ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, th);
    else
        sumOfTensors -> ApplyXYRecursiveTransform(X_bitmask, Y_bitmask, th);
    
}

cmplx AdaptiveStateVector::
operator[](idx_size i) const
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

void AdaptiveStateVector::
ApplyGlobalICounter()
{
    if (full_state)
        return full_state -> ApplyGlobalICounter();
    else
        return sumOfTensors -> ApplyGlobalICounter();
}

void AdaptiveStateVector::
PrintStateVector(const string& outfile) const
{

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
PrintProbabilities(const string& out_file) const
{
    
}
