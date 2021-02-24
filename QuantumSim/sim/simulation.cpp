
//  simulation.cpp
//  Quantum Circuit
//
//  Created by Aneeqa Fatima on 12/12/17.
//

#include "simulation.h"

unordered_map<string, array<cmplx, 5>> SequentialSimulation::benchmark = {};

double circuit_loop_time = 0;

SequentialSimulation::
SequentialSimulation(Config* c): curr_gate(0), num_layers(0), adjustment_factor(1),
memory_usage(0), total_time(0), branch_time(0), prefix_time(0), XE_time(0), mmap_time(0), config(c)
{
    if (config -> norm_depth) {
        idx_size num_norms = config -> norm_depth ? 1ull << config -> norm_depth : 1ull << config -> ranges_bits;
        norms_CZ_paths.resize(num_norms, 0);
    }
    
    amps_of_interest.resize(config -> indices.size(), 0);
    
    if (config -> proc_prefix_bits)
        adjustment_factor = 2;
    
}

void SequentialSimulation::
PopulateBenchmarkMap()
{
    benchmark["inst_5_5_100_5_26"] = {cmplx(-1.131e-05,-7.93622e-06), cmplx(8.75183e-05,5.33763e-05),
        cmplx(1.07944e-05,4.87672e-05), cmplx(0.000104529,-0.000158536), cmplx(5.64697e-05,-0.000154015)};
    benchmark["inst_5_5_100_5_27"] = {cmplx(3.33539e-05,1.10289e-05), cmplx(-6.26463e-05,-5.86281e-05),
        cmplx(0.000114191,0.000189693), cmplx(0.000108496,-6.67671e-05), cmplx(-0.000189895,-5.00576e-05)};
    benchmark["inst_5_5_100_5_101"] = {cmplx(4.87169e-07,-0.000124961), cmplx(-3.05764e-05,-0.000204519),
        cmplx(-6.19013e-05,-8.43196e-05), cmplx(-0.00016258,-1.55531e-05), cmplx(-0.000111381,0.000135595)};
    benchmark["inst_6_5_100_5_26"] = {cmplx(-6.54596e-06,-2.83312e-05), cmplx(9.81011e-06,-1.3834e-05),
        cmplx(-1.328e-06,-5.20411e-05), cmplx(-2.88494e-05,-3.6528e-05), cmplx(2.02035e-05,-1.49718e-05)};
    benchmark["inst_6_5_100_5_101"] = {cmplx(-1.64932e-05,3.05875e-05), cmplx(-2.23461e-05,-2.09741e-05),
        cmplx(4.19768e-06,-3.64171e-05), cmplx(-9.25683e-06,5.20232e-05), cmplx(4.8687e-06,-1.32963e-05)};
    benchmark["inst_8_4_100_0_26"] = {cmplx(1.26873e-05,1.5801e-05), cmplx(-1.3945e-05,5.97119e-06),
        cmplx(-4.64556e-06,7.54662e-06), cmplx(-4.70546e-06,-3.98253e-06), cmplx(2.23744e-05,2.45329e-06)};
    benchmark["inst_8_4_100_0_31"] = {cmplx(-1.12582e-05,6.73938e-06), cmplx(5.84793e-06,-6.60197e-06),
        cmplx(-5.53118e-06,1.13912e-05), cmplx(1.60148e-05,-1.06845e-06), cmplx(1.77797e-05,9.72003e-06)};
    benchmark["inst_7_5_100_5_26"] = {cmplx(-3.07386e-06,2.53827e-06), cmplx(-6.51143e-06,-2.31236e-06),
        cmplx(-1.1414e-06,-7.2411e-06), cmplx(2.26241e-06,-2.0497e-06), cmplx(4.57272e-06,-8.47909e-07)};
    benchmark["inst_5x5_21_0_22"] = {cmplx(9.21374e-05, 6.59681e-05), cmplx(-1.35506e-05,0.000152743),
        cmplx(-2.26996e-05, -4.063e-06), cmplx(0.000146145, 3.12126e-06), cmplx(-8.68663e-06, -7.95167e-05)};
    benchmark["inst_5x6_21_0_22"] = {cmplx(-1.141978e-05, 8.101054e-06), cmplx(-2.860430e-05, -4.218435e-05),
        cmplx(-2.930546e-05, 4.713042e-06), cmplx(1.8098e-12, -1.160662e-06), cmplx(-1.208545e-05, -2.048368e-07)};
    benchmark["inst_8_8_26_0_chen_6"] = {cmplx(1.01709e-13, 9.58446e-13), cmplx(-2.3139e-12, 2.4555e-13),
        cmplx(1.01712e-13, 9.58448e-13), cmplx(1.204301e-10, 1.46254e-12), cmplx(-2.89576e-10, 3.58332e-10)};
    benchmark["inst_8_8_26_0_chen_11"] = {cmplx(1.904682e-10, 2.036684e-10), cmplx(-6.751987e-11, 2.830940e-11),
        cmplx(-6.978375e-11, -1.419068e-10), cmplx(1.204301e-10, 7.646200e-11), cmplx(-4.436820e-11, -5.569570e-11)};
    
//    benchmark["25_26"] = {cmplx(0.000104529,-0.000158536), cmplx(0.00011983,5.54614e-05), cmplx(0.000158796,6.78702e-05),
//        cmplx(-1.13099e-05,-7.9362e-06), cmplx(4.02591e-05, -6.56695e-05)};
//    benchmark["25_101"] = {cmplx(-0.000162581, -1.55533e-05), cmplx(-8.97293e-06, -1.42227e-05), cmplx(6.07801e-05, -4.70146e-05),
//        cmplx(4.87014e-07, -0.000124961), cmplx(0.000202284, 0.000114519)};
//    benchmark["30_26"] = {cmplx(-2.88494e-05, -3.6528e-05), cmplx(1.01401e-05, 6.81786e-06), cmplx(5.24095e-06, 4.47689e-05),
//        cmplx(-6.54596e-06, -2.83312e-05), cmplx(1.06338e-05, 3.37055e-05)};
//    benchmark["30_101"] = {cmplx(-9.25684e-06, 5.20232e-05), cmplx(-3.33498e-05, 2.11104e-05), cmplx(-4.62867e-05, -1.58411e-05),
//        cmplx(-1.64932e-05, 3.05876e-05), cmplx(-1.56987e-05, -7.06633e-06)};
}

void SequentialSimulation::
SetThreshold(int th)
{
    config -> th = th;
}

void SequentialSimulation::
CopyOrRead(bool file_back_up,
           bool branch,
           GenericQuantumState& amp,
           const GenericQuantumState& copy_amp)
{
    
    if (file_back_up) {
        Time file_io_time;
        file_io_time.StartTime();
        
        amp.ReadFromDisk(config -> temp_dir + "checkpoint" + to_string(branch));
        amp.CopyMemberVars(copy_amp);
        
        double time_file_io = file_io_time.GetElapsedTime();
        amp.time_by_category.copying += time_file_io;
        phase1_time += time_file_io;
        ++amp.count_of_category.copying;
    }
    else {
        Time c_time;
        c_time.StartTime();
        
        if (config -> compress) {
            amp.DecompressAndCopyAnotherState(copy_amp);
            
            double time = c_time.GetElapsedTime();
            amp.time_by_category.decompress += time;
            phase1_time += time;
            ++amp.count_of_category.decompress;
        }
        else {
            amp.CopyState(copy_amp);
            
            double time_copying = c_time.GetElapsedTime();
            amp.time_by_category.copying += time_copying;
            phase1_time += time_copying;
            ++amp.count_of_category.copying;
        }
    }
}

void SequentialSimulation::
MainLoopForRanges(GenericQuantumState& amp,
                  Circuit& circuit,
                  const GenericQuantumState& copy_amp)
{
    idx_size gate_num = curr_gate;
    
    Time time;
    time.StartTime();
    static const idx_size cz_paths_ex = config -> proc_prefix_bits ?
            1ull << config -> ranges_bits : 1ull << config -> norm_depth;
    int total_cz_bits = config -> proc_prefix_bits ?
            config -> ranges_bits : config -> norm_depth;
    
    if (config -> ranges_bits)
        config -> curr_mode = Config::SimMode::Ranges;
    
    prefix_time += time.GetElapsedTime();
    
    for (idx_size cz_p = 0; cz_p < cz_paths_ex; ++cz_p) {
        int remaining_cz_bits = total_cz_bits;
        idx_size cz_p_1 = cz_p;
        
        Phase1Simulation(amp, circuit, gate_num, remaining_cz_bits, cz_p_1, total_cz_bits);
        
        auto& idx = config -> indices;
        if (config -> dfs_length == 0) {
            if (amp.compressed) {
                Time decompress_time;
                decompress_time.StartTime();
                
                amp.DecompressStateVector();
                
                double time_decompress = decompress_time.GetElapsedTime();
                amp.time_by_category.decompress += time_decompress;
                phase1_time += time_decompress;
                ++amp.count_of_category.decompress;
            }
            
            Time amp_st_time;
            amp_st_time.StartTime();
            for(idx_size i = 0; i < idx.size(); ++i)
                amps_of_interest[i] += amp.GetGlobalAmpAtInterestingIdx(i);
            double time_storage = amp_st_time.GetElapsedTime();
            amp.time_by_category.amp_storage += time_storage;
            prefix_time += time_storage;
        }
        
        if (config -> norm_perc) {
            if (amp.compressed) {
                Time decompress_time;
                decompress_time.StartTime();
                
                amp.DecompressStateVector();
                
                double time_decompress = decompress_time.GetElapsedTime();
                amp.time_by_category.decompress += time_decompress;
                phase1_time += time_decompress;
                ++amp.count_of_category.decompress;
            }
            
            norms_CZ_paths[cz_p] = sqrt(amp.CalculateNormSquared());
        }
        amp.book_keep = false;
        
        config -> curr_mode = Config::SimMode::Ranges;
        
        if ((cz_p + 1) < cz_paths_ex)
            CopyOrRead(config -> save_cp_file > 0, false, amp, copy_amp);
    }
}

void SequentialSimulation::
MainLoopForBranching(GenericQuantumState& amp,
                     Circuit& circuit,
                     const GenericQuantumState& copy_amp,
                     const idx_size gate_i)
{
    static int exec = 0; ++exec;
    static const idx_size num_CZ_paths = 1ull << config -> dfs_length,
    idxs_len = config -> indices.size();
    static const int block0_th = amp.GetNumQInBlock(0) >> 1, block1_th = amp.GetNumQInBlock(1) >> 1;
    
    config -> curr_mode = Config::SimMode::Branch;
    
    int remaining_cz_bits = config -> dfs_length, remaining_cz_bits_copy = remaining_cz_bits;

    for (idx_size i = 0; i < num_CZ_paths; ++i) {
        remaining_cz_bits = config -> dfs_length;
        remaining_cz_bits_copy = remaining_cz_bits;
        idx_size cz_p_1 = i, cz_p_2 = cz_p_1;
        
        config -> th = block0_th;
        amp.partition_to_sim = 'a';
        SimulationLoop(amp, circuit, remaining_cz_bits, cz_p_1, config -> dfs_length, 0, gate_i);
        
        config -> th = block1_th;
        amp.partition_to_sim = 'b';
        SimulationLoop(amp, circuit, remaining_cz_bits_copy, cz_p_2, config -> dfs_length, 0, gate_i);
        amp.partition_to_sim = 'x';
        
        auto& idx = config -> indices;
        for(idx_size i = 0; i < idxs_len; ++i) {
            assert(amp.compressed == false);
            Time amp_st_time;
            amp_st_time.StartTime();
            amps_of_interest[i] += amp.GetGlobalAmpAtInterestingIdx(i);
         amp.time_by_category.amp_storage += amp_st_time.GetElapsedTime();
        
        amp.book_keep = false;
        
        if ((i + 1) < num_CZ_paths)
            CopyOrRead(config -> save_cp_file > 1, true, amp, copy_amp);
    }
    
    if (exec == 1) {
        if (remaining_cz_bits == 0) {
            if (circuit.GetTotalNumGates() != curr_gate)
                cout << "DFS path exhausted early\n";
            else
                cout << "No xCZ gates left\n";
        }
        else
            cout << "Truncated DFS length : " << remaining_cz_bits << "\n";
    }
}

void SequentialSimulation::
CheckpointWithFile(bool branch,
                   GenericQuantumState& amp,
                   Circuit& circuit,
                   const idx_size gate_i)
{
    Time copy_time;
    copy_time.StartTime();
    amp.WriteAmpToDisk(config -> temp_dir + "checkpoint" + to_string(branch));
    SumOfTensorsProductsStateVector temp_amp;
    temp_amp.CopyMemberVars(amp);
    double time_copying = copy_time.GetElapsedTime();
    amp.time_by_category.copying += time_copying;
    ++amp.count_of_category.copying;
    prefix_time += time_copying;
    
    if (branch) {
        if (config -> count_zeros) {
            amp.count_of_category.zero_count_cp2_A += amp.CountZerosInBlock(0);
            amp.count_of_category.zero_count_cp2_B += amp.CountZerosInBlock(1);
        }
        if (amp.book_keep)
            memory_usage += amp.GetMemUsage();
        MainLoopForBranching(amp, circuit, temp_amp, gate_i);
    }
    else {
        if (config -> count_zeros) {
            amp.count_of_category.zero_count_cp1_A += amp.CountZerosInBlock(0);
            amp.count_of_category.zero_count_cp1_B += amp.CountZerosInBlock(1);
        }
        MainLoopForRanges(amp, circuit, temp_amp);
    }

    string command = "rm " + config -> temp_dir + "checkpoint" + to_string(branch) + "*";
    system(command.c_str());
}

void SequentialSimulation::
CheckpointWithoutFile(bool branch,
                      GenericQuantumState& amp,
                      Circuit& circuit,
                      const idx_size gate_i)
{
    SumOfTensorsProductsStateVector temp_amp;
    
    if (config -> compress) {
        Time compress_time, decompress_time;
        compress_time.StartTime();
        
        amp.CompressStateVector(config -> cramer_num_codewords, config -> cramer_p_rejection);
        
        double time_compress = compress_time.GetElapsedTime();
        amp.time_by_category.compress += time_compress;
        
        decompress_time.StartTime();
        
        temp_amp.DecompressAndCopyAnotherState(amp);
        
        double time_decompress = decompress_time.GetElapsedTime();
        amp.time_by_category.decompress += time_decompress;
        
        phase1_time += time_compress + time_decompress;
        ++amp.count_of_category.decompress;
        ++amp.count_of_category.compress;
        
        if (amp.book_keep)
            memory_usage += amp.GetMemUsage();
    }
    else {
        Time copy_time;
        copy_time.StartTime();
        
        temp_amp.CopyState(amp);
        temp_amp.CopyMemberVars(amp);
        
        double time_copying = copy_time.GetElapsedTime();
        amp.time_by_category.copying += time_copying;
        phase1_time += time_copying;
        ++amp.count_of_category.copying;
        
        if (amp.book_keep)
            memory_usage += amp.GetMemUsage();
    }
    
  if (branch) {
        if (config -> count_zeros) {
            amp.count_of_category.zero_count_cp2_A += amp.CountZerosInBlock(0);
            amp.count_of_category.zero_count_cp2_B += amp.CountZerosInBlock(1);
        }
        MainLoopForBranching(temp_amp, circuit, amp, gate_i);
    }
    else {
        if (config -> count_zeros) {
            amp.count_of_category.zero_count_cp1_A += amp.CountZerosInBlock(0);
            amp.count_of_category.zero_count_cp1_B += amp.CountZerosInBlock(1);
        }
        MainLoopForRanges(temp_amp, circuit, amp);
    }
    
    //Should not need to decompress vector. State vector is useless at this point.
//    assert(amp.compressed == false);
//    if (config -> SZ_compress)
//        amp.DecompressStateVector();
}

void SequentialSimulation::
CheckpointWithRanges(GenericQuantumState& amp,
                     Circuit& circuit)
{
    Time time;
    time.StartTime();
    int remaining_cz_bits = config -> proc_prefix_bits,
        remaining_cz_bits_copy = remaining_cz_bits;
    idx_size cz_p_1 = config -> cz_path, cz_p_2 = cz_p_1;
    
    if (amp.GetNumQInBlock(0) >= amp.GetNumQInBlock(1)) {
        config -> th = amp.GetNumQInBlock(0) >> 1;
        amp.partition_to_sim = 'a';
    }
    else {
        config -> th = amp.GetNumQInBlock(1) >> 1;
        amp.partition_to_sim = 'b';
    }
    
    SimulationLoop(amp, circuit, remaining_cz_bits, cz_p_1,
                   config -> proc_prefix_bits, config -> dfs_length, 0);

    if (amp.AreAllAmpsZero()) {
        if (config -> count_zeros) {
            if (amp.GetNumQInBlock(0) >= amp.GetNumQInBlock(1))
                amp.count_of_category.zero_count_cp1_A += amp.CountZerosInBlock(0);
            else
                amp.count_of_category.zero_count_cp1_B += amp.CountZerosInBlock(1);
        }
        adjustment_factor = 1;
        return;
    }
    
    if (amp.GetNumQInBlock(0) >= amp.GetNumQInBlock(1)) {
        config -> th = amp.GetNumQInBlock(1) >> 1;
        amp.partition_to_sim = 'b';
    }
    else {
        config -> th = amp.GetNumQInBlock(0) >> 1;
        amp.partition_to_sim = 'a';
    }
    
     SimulationLoop(amp, circuit, remaining_cz_bits_copy, cz_p_2,
                      config -> proc_prefix_bits, config -> dfs_length, 0);
    amp.partition_to_sim = 'x';
    
    if (amp.AreAllAmpsZero()) {
        if (config -> count_zeros) {
            amp.count_of_category.zero_count_cp1_A += amp.CountZerosInBlock(0);
            amp.count_of_category.zero_count_cp1_B += amp.CountZerosInBlock(1);
        }
        return;
    }
    
    amp.count_of_category.cycle_p = circuit.GetCycleNumForGateIdx(curr_gate);
    prefix_time += time.GetElapsedTime();
    
    if (config -> save_cp_file > 0) {
        CheckpointWithFile(false, amp, circuit);
        
        string command = "rm -rf " + config -> temp_dir ;
        system(command.c_str());
    }
    else CheckpointWithoutFile(false, amp, circuit);
    
}

void SequentialSimulation::
NoCheckpointWithRanges(GenericQuantumState& amp,
                       Circuit& circuit)
{
    amps_of_interest.resize(config -> indices.size(), 0);
    
    config -> cz_path <<= config -> ranges_bits;
    idx_size cz_paths_ex = config -> proc_prefix_bits ?
            1ull << config -> ranges_bits : 1ull << config -> norm_depth;
    int total_cz_bits = config -> proc_prefix_bits ?
            config -> proc_prefix_bits + config -> ranges_bits : config -> norm_depth;
    
    if (config -> ranges_bits)
        config -> curr_mode = Config::SimMode::Ranges;
    
    for (idx_size cz_p = 0; cz_p < cz_paths_ex; ++cz_p) {
        int remaining_cz_bits = total_cz_bits;
        idx_size cz_p_1 = config -> cz_path + cz_p;
        
        amp.ResetAmpVector();
        Phase1Simulation(amp, circuit, 0, remaining_cz_bits, cz_p_1, total_cz_bits);
        
        auto& idx = config -> indices;
        if (config -> dfs_length == 0) {
            Time amp_st_time;
            amp_st_time.StartTime();
            for(idx_size i = 0; i < idx.size(); ++i)
                amps_of_interest[i] += amp.GetGlobalAmpAtInterestingIdx(i);
            double time_storage = amp_st_time.GetElapsedTime();
            amp.time_by_category.amp_storage += time_storage;
            prefix_time += time_storage;
        }
        
        config -> curr_mode = Config::SimMode::Ranges;

        if (config -> norm_perc)
            norms_CZ_paths[cz_p] = sqrt(amp.CalculateNormSquared());
    }
}

void SequentialSimulation::
Simulate(GenericQuantumState& amp,
         Circuit& circuit)
{
//    if (config -> approx && config -> approx_epsilon)
//        amp.approx = config -> proc_prefix_bits + ceil(log2(config -> approx_epsilon));
//        
    PopulateBenchmarkMap();
    
    memory_usage += amp.GetMemUsage();
   
    pair<int, int> twoq_gate_count(0, 0);

    if (circuit.google) {
        if (!circuit.ClockCycleEmpty())
            circuit.GroupAlternateCycles();
        twoq_gate_count.first = circuit.GroupSimilarGates();
    }
    
    if (config -> sim_type != Config::SimType::FullState) {
        bool H_sims = config -> sim_type == Config::SimType::LosslessH ||
        config -> sim_type == Config::SimType::Approx1CutH || config -> sim_type == Config::SimType::ApproxCZPathH2011;
        
        QubitPartition qp = H_sims ?
        QubitPartition(QubitPartition::Cuts::Horizontal, circuit.GetNumQubits(),
                       config -> row_major, config -> hcut, config -> first_part_smaller) :
        QubitPartition(QubitPartition::Cuts::Vertical, circuit.GetNumQubits(), config -> row_major,
                       config -> vcut, config -> first_part_smaller) ;
//        qp.RenumberLocalQubits();
        
        twoq_gate_count = circuit.MovexCZGates(config -> proc_prefix_bits,
                                               config -> ranges_bits, config -> dfs_length,
                                               qp, config -> nearest_neighbors);
    }
    
    
    if (config -> verbose)
        PrintSimSpecReport(amp, circuit, twoq_gate_count);
    
    log << "Cycle \tRuntime \tMemory\t\tXEntropy\n";
    
    Time time;
    time.StartTime();
    if (config -> ranges_bits || config -> norm_perc) {
        if (config -> store_checkpoint_range)
            CheckpointWithRanges(amp, circuit);
        else
            NoCheckpointWithRanges(amp, circuit);
    }
    else if (config -> proc_prefix_bits) {
        int remaining_cz_bits = config -> proc_prefix_bits;
        Phase1Simulation(amp, circuit, curr_gate, remaining_cz_bits,
                         config -> cz_path, config -> proc_prefix_bits);
                
        auto& idx = config -> indices;
        if (config -> dfs_length == 0) {
            Time amp_st_time;
            amp_st_time.StartTime();
            for(idx_size i = 0; i < idx.size(); ++i)
                amps_of_interest[i] += amp.GetGlobalAmpAtInterestingIdx(i);
            double storage_time = amp_st_time.GetElapsedTime();
            amp.time_by_category.amp_storage += storage_time;
            mmap_time += storage_time;
        }
    }
    else {
        int remaining_cz_bits = -1;
        Phase1Simulation(amp, circuit, curr_gate, remaining_cz_bits, config -> cz_path, 0);
        auto& idx = config -> indices;
        Time amp_st_time;
        amp_st_time.StartTime();
        for(idx_size i = 0; i < idx.size(); ++i) {
            assert(amp.compressed == false);
            amps_of_interest[i] += amp[idx[i]];
        }
        amp.time_by_category.amp_storage += amp_st_time.GetElapsedTime();
    }

    if (config -> mmap_obj -> GetMapPtr()) {
        Time amp_st_time;
        amp_st_time.StartTime();
        for (idx_size i = 0; i < config -> indices.size(); ++i)
            *(*config -> mmap_obj)[i] += amps_of_interest[i];
        double storage_time = amp_st_time.GetElapsedTime();
        amp.time_by_category.amp_storage += storage_time;
        mmap_time += storage_time;
    }
    
    total_time += time.GetElapsedTime() - XE_time;
//    total_time = circuit_loop_time;
    
    ReportingAfterSim(amp, circuit);
}

void SequentialSimulation::
Phase1Simulation(GenericQuantumState& amp,
                 Circuit& circuit,
                 idx_size gate_i,
                 int& remaining_cz_bits,
                 idx_size& cz_path,
                 const idx_size cz_path_len)
{
    static int exec = 0; ++exec;
    static const idx_size total_cz_bits = config -> proc_prefix_bits +
       config -> ranges_bits + config -> dfs_length;
    
    Time cz_path_time;
    cz_path_time.StartTime();
    
    bool terminate = false;
    
    if (total_cz_bits != 0 && remaining_cz_bits > 0) {
        config -> th = amp.GetNumQInBlock(0) >> 1;
        amp.partition_to_sim = 'a';
        idx_size cz_path_copy = cz_path;
        int remaining_cz_bits_copy = remaining_cz_bits;
        terminate = SimulationLoop(amp, circuit, remaining_cz_bits, cz_path, cz_path_len,
                                   config -> dfs_length, gate_i);
        config -> th = amp.GetNumQInBlock(1) >> 1;
        amp.partition_to_sim = 'b';
        SimulationLoop(amp, circuit, remaining_cz_bits_copy, cz_path_copy, cz_path_len,
                       config -> dfs_length, gate_i);
        amp.partition_to_sim = 'x';
    }
    else if (remaining_cz_bits == -1)
        SimulationLoop(amp, circuit, remaining_cz_bits, cz_path, cz_path_len, config -> dfs_length, 0);
    
    if (exec == 1) {
        if (config -> ranges_bits != 0)
            amp.count_of_category.cycle_r = circuit.GetCycleNumForGateIdx(curr_gate);
        else if (config -> proc_prefix_bits != 0)
            amp.count_of_category.cycle_p = circuit.GetCycleNumForGateIdx(curr_gate);
    }
    prefix_time += cz_path_time.GetElapsedTime();

    if (terminate && config -> dfs_length != 0) {
        Phase2Simulation(amp, circuit, curr_gate);
        if (exec == 1)
            amp.count_of_category.cycle_d = circuit.GetCycleNumForGateIdx(curr_gate);
    }
    else if (!terminate &&  config -> dfs_length != 0)
        config -> dfs_length = 0;
    else if (exec == 1) {
        if (terminate && config -> dfs_length == 0 && remaining_cz_bits == 0 && total_cz_bits != 0) {
            cout << "Simulated " + to_string(curr_gate) + " gates, including "
            + to_string(amp.count_of_category.decomposed_CZ / adjustment_factor) + " xCZ gates. ";
            
            if (circuit.GetTotalNumGates() != curr_gate)
                cout << "CZpath exhausted early.\n";
            
            else {
                cout << "No xCZ gates left\n";
                cout << "Truncated CZ path : " << remaining_cz_bits << "\n";
            }
        }
        else if (!terminate && remaining_cz_bits == 0) {
            cout << "Simulated " + to_string(curr_gate) + " gates, including "
            + to_string(amp.count_of_category.decomposed_CZ / adjustment_factor) + " xCZ gates. No xCZ gates left.\n";
            if (config -> dfs_length != 0)
                cout << "Truncated DFS length : " << config -> dfs_length << "\n";
        }
        else if (!terminate && total_cz_bits != 0) {
            cout << "Truncated CZ path : " << remaining_cz_bits << "\n";
            if (config -> dfs_length != 0)
                cout << "Truncated DFS length : " << config -> dfs_length << "\n";
        }
    }
}

void SequentialSimulation::
Phase2Simulation(GenericQuantumState& amp,
                 Circuit& circuit,
                 const idx_size gate_i)
{
    Time phase2_time;
    phase2_time.StartTime();
    
    //    bool terminate = false;
//    ++amp.count_of_category.rescale;
//    amp.RescaleAndApplyGlobalICounter();
    
    if (config -> save_cp_file > 1 && config -> ranges_bits != 0)
        CheckpointWithFile(true, amp, circuit, gate_i);
    else if (config -> save_cp_file > 0 && config -> ranges_bits == 0) {
        CheckpointWithFile(true, amp, circuit, gate_i);
        
        string command = "rm -rf " + config -> temp_dir ;
        system(command.c_str());
    }
    else CheckpointWithoutFile(true, amp, circuit, gate_i);
    
    branch_time += phase2_time.GetElapsedTime();
}


bool SequentialSimulation::
SimulationLoop(GenericQuantumState &amp,
               Circuit &circuit,
               int& remaining_cz_bits,
               idx_size& cz_path,
               const idx_size cz_path_len,
               const idx_size suffix_size,
               const idx_size gate_i)
{
    Time loop_time;
    loop_time.StartTime();
    
    static const idx_size size = circuit.GetTotalNumGates();
    static const int total_circuit_qubits = circuit.GetNumQubits();
    static const auto gates = circuit.GetGates();
    
    bool terminate = false;
    int current_cycle = 0, last_layers_of_H = config -> last_layers_H;
    Time time, cycle_time;
    for (idx_size i = gate_i; i < size; ++i) {
        
        if (amp.book_keep)
            ++num_layers;
        if (config -> curr_mode == Config::SimMode::ProcPrefix)
            ++amp.count_of_category.proc_prefix_layers;
        else if (config -> curr_mode == Config::SimMode::Ranges)
            ++amp.count_of_category.ranges_layers;
        else if (config -> curr_mode == Config::SimMode::Branch)
            ++amp.count_of_category.branch_layers;
        
        if (amp.GetGlobalFactorPower() > 100) {
            ++amp.count_of_category.rescale;
            amp.Rescale();
        }
        
        curr_gate = i;
        Gate& current_gate = circuit.GetGateFromIndex(i);
        if(current_gate.ids.front() == Gate::Type::Control ||
           current_gate.ids.back() == Gate::Type::T) {

            double cycle_elapsed_t = time.GetElapsedTime();
            current_cycle = circuit.GetCycleNumForGateIdx(i);
            
//            if (amp.book_keep)
//                amp.data_per_cycles.cycles.push_back(current_cycle);
          
            if (current_gate.ids.back() == Gate::Type::T ||
                current_gate.ids.back() == Gate::Type::Z) {
                
                Time CZT_time;
                CZT_time.StartTime();
                
                double cross_entropy = 0;
                
                if (config -> verbose == 4 && amp.book_keep) {
                    Time misc_time;
                    misc_time.StartTime();
                    cross_entropy = amp.CalculateCrossEntropy(10);
                    XE_time += misc_time.GetElapsedTime();
                    
                    log <<  setprecision(3) ;
                    log << current_cycle << "\t" << cycle_elapsed_t << " s    \t";
                    double memory = amp.GetMemUsage();
                    if (memory >= (1 << 30)) {
                        log << memory / (1 << 30) << " GiB";
                    }
                    else if (memory >= (1 << 20)) {
                        log << memory / (1 << 20) << " MiB";
                    }
                    else if (memory >= (1 << 10)) {
                        log << memory / (1 << 10) << " KiB";
                    }
                    else
                        log << memory << " B";
                    
                    if (cross_entropy)
                        log << "\t\t" << cross_entropy ;
                    log << "\n";
                }
                
                cycle_time.StartTime();
                idx_size prev_i_CZT = i;
                bitset<128> T_bitmasks[2] = {0};
                bitset<128> CZ_bitmasks[total_circuit_qubits];
                amp.FormCZTGatesBitmask(CZ_bitmasks, T_bitmasks, i, gates, total_circuit_qubits);
                idx_size prev_i_XY = i;
                bitset<128> X_bitmask = amp.FormXYHGatesBitmask(i, gates, Gate::Type::X_1_2);
                bitset<128> Y_bitmask = amp.FormXYHGatesBitmask(i, gates, Gate::Type::Y_1_2);
                bitset<128> H_bitmask = 0;
                bool last_cycle = false;
                
                if (i < size && circuit.GetGateFromIndex(i).ids.back() == Gate::Type::Hadamard) {
                    last_cycle = true;
                    H_bitmask = amp.FormXYHGatesBitmask(i, gates, Gate::Type::Hadamard);
                    if (config -> last_layers_H)
                        last_layers_of_H--;
                    ++amp.count_of_category.H_layers;
                }

                int last_xCZ_idx = -1;
                if (X_bitmask != 0 || Y_bitmask != 0) {
                    last_xCZ_idx = amp.ApplyLoXYHAndCZTInSamePass(remaining_cz_bits, cz_path, cz_path_len,
                                                                  suffix_size, X_bitmask,
                                                                  Y_bitmask, H_bitmask, CZ_bitmasks,
                                                                  T_bitmasks, config -> th, last_cycle);
                    ++amp.count_of_category.CZT_layers;
                    ++amp.count_of_category.XY_layers;
                }
                else {
                    last_xCZ_idx = amp.ApplyBlockOfDiagGates(remaining_cz_bits, cz_path, cz_path_len,
                                                             suffix_size, CZ_bitmasks, T_bitmasks, H_bitmask,
                                                             last_cycle);
                    ++amp.count_of_category.CZT_layers;
                }
                
                terminate = last_xCZ_idx != -1 ? true : false;
                
                if (amp.book_keep) {
//                    if (config -> sim_type == Config::SimType::FullState) {
//                        amp.data_per_cycles.xCZ_H.push_back(0);
//                        amp.data_per_cycles.xCZ_V.push_back(0);
//                    }
                    
                    if (!terminate) {
                        amp.count_of_category.CZ_T += prev_i_XY - prev_i_CZT - amp.count_of_category.xCZ_not_applied;
//                        amp.data_per_cycles.T_gates.push_back(T_bitmasks[0].count() + T_bitmasks[1].count());
//                        amp.data_per_cycles.CZ_gates.push_back(prev_i_XY - prev_i_CZT -
//                                                               amp.data_per_cycles.T_gates.back() -
//                                                               amp.count_of_category.xCZ_not_applied);
                    }
                    else if (last_xCZ_idx) {
                        amp.count_of_category.CZ_T += last_xCZ_idx;
//                        amp.data_per_cycles.CZ_gates.push_back(last_xCZ_idx); //not accurate
                    }
//                    amp.data_per_cycles.XY_gates.push_back(i - prev_i_XY);
                }
                
                if (terminate) {
                    curr_gate = prev_i_CZT + last_xCZ_idx;
                    return terminate;
                }
                
                --i;
                
                CZ_T_top_time += CZT_time.GetElapsedTime();
            }
            else {
                amp.ApplyCGate(current_gate.num_controls, current_gate.qubits,
                               current_gate, (Gate::Type)current_gate.ids.back());
            }
        }
        else {
            if(circuit.google && current_gate.ids.back() == Gate::Type::Hadamard) {
                
                ++amp.count_of_category.H_layers;
                cycle_time.StartTime();
                amp.ApplyHGateOnAllAmps(i != 0);
                i += total_circuit_qubits - 1;
            }
        }
    }
    
    while (last_layers_of_H) {
        ++amp.count_of_category.H_layers;
        amp.ApplyHGateOnAllAmps(true);
        --last_layers_of_H;
    }
    
    curr_gate = circuit.GetTotalNumGates();
    
    circuit_loop_time += loop_time.GetElapsedTime();
    return terminate;
}


void SequentialSimulation::
ReportingAfterSim(GenericQuantumState& amp,
                  Circuit& circuit)
{
#ifdef Print
    amp.PrintStateVector();
#endif
#ifdef CosineSimilarity
    amp.PrintProbabilities(config -> prob_outfile, circuit.GetNumCycles() - 1);
#endif
#ifdef PorterThomas
    amp.PrintProbabilities(config -> prob_outfile, circuit.GetNumCycles() - 1);
#endif
#ifdef ReportToFile
    if (config -> verbose)
        PrintReportToFile(amp, circuit);
#else
    if (config -> verbose)
        PrintSimReport(amp, circuit);
#endif
    
    if (config -> proc_prefix_bits == 0)
        amp.RescaleAndApplyGlobalICounter();
    
    if (config -> print_idx)
        PrintIdxsToFile();
    
    if (config -> norm_perc) {
        ofstream norm_out;
        norm_out.open(config -> misc_outfile + "/" + config -> infile + "_" + to_string(config -> norm_depth) + ".norms");
        
        double sq_sum = 0;
        for (auto n : norms_CZ_paths){
            norm_out << -2 * log2(n) << "\n";
            sq_sum += (n * n);
        }

        cout << "Sum of squares: " << sq_sum << endl;
        norm_out.close();
    }
   
    if (config -> print_amp) {
      if (config -> proc_prefix_bits) {
          if (!config -> ascii) 
              config -> mmap_obj -> WriteToDisk();
         else
            WriteMmapToASCIIFile();
      }
      else
        WriteAmpToASCIIFile(amp);
    }    
}

void SequentialSimulation::
PrintIdxsToFile() const
{
    //        sort(config -> indices.begin(), config -> indices.end(), [](bitset<128>& first, bitset<128>& second) {
    //            for (int i = 127; i >= 0; i--) {
    //                if (first[i] ^ second[i]) return (bool)second[i];
    //            }
    //            return false;
    //        });
    
    string dir = "output/amp_vectors/" + config -> infile + "_" + to_string(config -> depth)
    + "_" + to_string(config -> proc_prefix_bits + config -> ranges_bits) + "_" + to_string(config -> num_threads);
    if (config -> approx)
        dir += "_approx_" + to_string(config -> approx_epsilon);
    string idx_outfile = dir + config -> amp_outfile.substr(config -> amp_outfile.find_last_of("/")) + ".idx";
    ofstream  idx_out;
    idx_out.open(idx_outfile);
    
    auto& idx_print = config -> indices;
    
    //print numbers larger than 64 bits?
    for (idx_size i = 0; i < idx_print.size(); ++i) {
        try {
            idx_out << config -> indices[i].to_ullong() << "\n";
        }
        catch (runtime_error) {
            string dec_binary = "";
            bitset<128> idx = config -> indices[i];
            __int128 temp_idx = ((((__int128)((idx >> 64)).to_ullong())) << 64) + ((idx << 64) >> 64).to_ullong();
            print_u128_u(temp_idx, dec_binary);
            idx_out << dec_binary << "\n";
        }
    }
    idx_out.close();
}

void SequentialSimulation::
 WriteMmapToASCIIFile() const
{
    string dir = "output/amp_vectors/" + config -> infile + "_" + to_string(config -> depth)
    + "_" + to_string(config -> proc_prefix_bits + config -> ranges_bits) + "_" + to_string(config -> num_threads);
    if (config -> approx)
        dir += "_approx_" + to_string(config -> approx_epsilon);
    string amp_outfile = dir + config -> amp_outfile.substr(config -> amp_outfile.find_last_of("/")) + "_ascii.amps";
    ofstream amp_out;
    amp_out.open(amp_outfile);
    
    auto& idx_print = config -> indices;
   
    for (idx_size i = 0; i < idx_print.size(); ++i) {
        amp_out << real(*(*config -> mmap_obj)[i]);
        if (imag(*(*config -> mmap_obj)[i]) < 0) amp_out << imag(*(*config -> mmap_obj)[i]) << "j";
        else amp_out << "+" << imag(*(*config -> mmap_obj)[i]) << "j";
        amp_out << "\n";
    }
    amp_out.close();
}

void SequentialSimulation::
WriteAmpToASCIIFile(GenericQuantumState& amp) const
{
    string dir = "output/amp_vectors/" + config -> infile + "_" + to_string(config -> depth)
    + "_" + to_string(config -> proc_prefix_bits + config -> ranges_bits) + "_" + to_string(config -> num_threads);
    if (config -> approx)
        dir += "_approx_" + to_string(config -> approx_epsilon);
    
    string command = "mkdir -p " + dir;
    system(command.c_str());
    auto time = to_string(clock());
    string amp_outfile = dir + config -> amp_outfile.substr(config -> amp_outfile.find_last_of("/"))  + ".amps";
    ofstream amp_out;
    amp_out.open(amp_outfile);
   
    auto& idx_print = config -> indices;
    
    for (idx_size i = 0; i < idx_print.size(); ++i) {
        amp_out << real(amp[idx_print[i]]);
        if (imag(amp[idx_print[i]]) < 0) amp_out << imag(amp[idx_print[i]]) << "j";
        else amp_out << "+" << imag(amp[idx_print[i]]) << "j";
        amp_out << "\n";
    }
    amp_out.close();
}

//void SequentialSimulation::
//AddToExternallyStoredAmps(const GenericQuantumState& amp) const
//{
//    auto does_file_exist = [](const char *fileName)
//    {
//        std::ifstream infile(fileName);
//        return infile.good();
//    };
//
//    string amp_outfile = config -> amp_outfile + "/output_" + to_string(config -> indices.size() - 5) + ".amps";
//    if (does_file_exist (amp_outfile.c_str())) {
//        idx_size size = config -> indices.size() * sizeof(cmplx);
//
//        int fd = open(amp_outfile.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
//        cmplx *map = (cmplx*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//        if (map == MAP_FAILED)
//        {
//            close(fd);
//            perror("Error mmapping the file");
//            exit(EXIT_FAILURE);
//        }
//
//        auto& idx_print = config -> indices;
//        for (idx_size i = 5; i < idx_print.size(); ++i)
//            map[i] += amp[idx_print[i]];
//
//        // Write it now to disk
//        if (msync(map, size, MS_SYNC) == -1)
//        {
//            perror("Could not sync the file to disk");
//        }
//
//    }
//    else {
//
//        string command = "mkdir -p " + config -> amp_outfile;
//        system(command.c_str());
//        string idx_outfile = config -> amp_outfile + "/output_" + to_string(config -> indices.size() - 5) + ".idx";
//        ofstream idx_out;
//
//        if (config -> print_idx)
//            idx_out.open(idx_outfile);
//
//        auto& idx_print = config -> indices;
//        cmplx amps_w[idx_print.size() - 5];
//        if (config -> print_idx)
//            for (idx_size i = 5; i < idx_print.size(); ++i)
//                idx_out << idx_print[i].to_ullong() << "\n";
//
//        FILE *amp_file;
//        amp_file = fopen(amp_outfile.c_str(), "wb");  // w for write, b for binary
//        fwrite(amps_w, sizeof(amps_w), 1, amp_file);
//
//        if (config -> print_idx)
//            idx_out.close();
//    }
//}

void SequentialSimulation::
PrintSystemReport() const
{
    cout << "\n(C) 2017, 2018  Regents of the University of Michigan\n";
    cout << "Rollright ver 2.3 - a quantum circuit simulator\n";
    cout << "Igor L. Markov and Aneeqa Fatima\n\n";
    
//    char hostname[30] = {};
//    gethostname(hostname, 30);
    cout << "Hostname : "; flush(cout);
    system("uname -a");
    
    if (config -> verbose != Config::Verbose::NCCV && config -> verbose != Config::Verbose::NCC) {
#ifdef __APPLE__
        cout << "CPU model name : "; flush(cout);
        system("sysctl -n machdep.cpu.brand_string");
        cout << "CPU cores : "; flush(cout);
        system("sysctl -n machdep.cpu.core_count");
        cout << "Hardware threads : "; flush(cout);
        system("sysctl -n machdep.cpu.thread_count");
#ifdef Parallel
        flush(cout);
        cout << "Max threads per process : " << config -> num_threads << "\n";
#endif
        cout << "L2 cache size : "; flush(cout);
        system("sysctl -n hw.l2cachesize");
        cout << "L3 cache size : "; flush(cout);
        system("sysctl -n hw.l3cachesize");
#endif
#ifndef __APPLE__
        cout << "CPU "; flush(cout);
        system("egrep CPU /proc/cpuinfo | head -1");
        system("egrep cores /proc/cpuinfo | head -1");
        cout << "Hardware threads : "; flush(cout);
        system("egrep cores  /proc/cpuinfo | wc -l");
        #ifdef Parallel
        cout << "Max threads per process : " << config -> num_threads << "\n";
        flush(cout);
        #endif
        system("grep \"MemTotal\" /proc/meminfo");
        cout << "L3 " ; flush(cout);
        system("egrep cache /proc/cpuinfo | head -1");
        system("grep \"Hugepagesize:\" /proc/meminfo");
        system("grep \"HugePages_Total\" /proc/meminfo");
        system("grep \"HugePages_Free\" /proc/meminfo");
#endif
        cout << "CPU instructions width :"
        << " popcnt:" << __builtin_cpu_supports("popcnt");
        
        if (__builtin_cpu_supports("sse4.2"))
            cout << ", sse4.2:" << __builtin_cpu_supports("sse4.2");
        else if (__builtin_cpu_supports("sse4.1"))
            cout << ", sse4.1:" << __builtin_cpu_supports("sse4.1");
        else if (__builtin_cpu_supports("sse4.1"))
            cout << ", sse4.1:" << __builtin_cpu_supports("sse4.1");
        else if (__builtin_cpu_supports("ssse3"))
            cout << ", ssse3:" << __builtin_cpu_supports("ssse3");
        else if (__builtin_cpu_supports("sse3"))
            cout << ", sse3:" << __builtin_cpu_supports("sse3");
        else if (__builtin_cpu_supports("sse2"))
            cout << ", sse2:" << __builtin_cpu_supports("sse2");
        else if (__builtin_cpu_supports("sse"))
            cout << ", sse:" << __builtin_cpu_supports("sse");
        
        cout << ", avx:" << __builtin_cpu_supports("avx")
        << ", avx2:" << __builtin_cpu_supports("avx2") << "\n";
        cout << "Using instructions : " << "AVX-2, popcnt\n\n";
        
        cout << "Compiler : gcc " << __GNUC__  << "." << __GNUC_MINOR__ << "."
        <<  __GNUC_PATCHLEVEL__<< "\n";
        
        cout << "Compiled on : " <<  __DATE__ << " " << __TIME__ << "\n";
        time_t t = time(0);
        struct tm * now = localtime (&t);
        cout << "Executed on : "
        << (now->tm_mon + 1) << "/"
        <<  now->tm_mday << "/"
        << (now->tm_year + 1900) << " "
        <<  now->tm_hour << ":" << now->tm_min << ":"
        << std::setw(2) << std::setfill('0') << now->tm_sec
        <<"\n";
        cout << "Size of complex : " << sizeof(cmplx) << " B\n\n";
        cout << "Verbosity : " << config -> verbose ;
    }
}

void SequentialSimulation::
PrintSimSpecReport(const GenericQuantumState& amp,
                   const Circuit& circuit,
                   const pair<int, int> twoq_gates)
{
    if (config -> google) {
        cout << "\nCircuit file : " + config -> infile + "\n";
        cout << "Circuit type : Google\n";
    }
    
    cout << "Qubits : " << circuit.GetNumQubits() << "  ";
    cout << "Gates : " << circuit.GetTotalNumGates() << " (" << twoq_gates.first << " two-q gates) " ;
    cout << "Cycles : " << circuit.GetNumCycles() << "\n";
    
    if (!amp.log.empty())
        cout << amp.log[amp.log.size() - 1]<< "\n";
    
    stringstream approx_type;
    if (config -> approx) {
        approx_type << "Requested end-to-end circuit fidelity: " << setprecision(3)
        << 1.0/float(config -> approx_epsilon) << "\nApproximation type : ";
        if (config -> proc_prefix_bits)
            approx_type << "pruned xCZ branches";
        if (config -> sim_type == Config::SimType::ApproxCZPathH2011
            || config -> sim_type == Config::SimType::ApproxCZPathV2011) {
            if (config -> ranges_bits == 0)
                approx_type << " / ";
            approx_type << "Approx2011";
        }
        approx_type << "\n";
    }
    
    idx_size log_count = 0;
    cout << "Simulation type : ";
    if (config -> sim_type == Config::SimType::FullState) {
        cout << "full state-vector  \n";
        cout << approx_type.str();
    }
    else if (config -> sim_type == Config::SimType::LosslessH
             || config -> sim_type == Config::SimType::ApproxCZPathH2011) {
        cout << "sum of tensor products / single cut\n";
        cout << amp.log[log_count++] << " (" << twoq_gates.second <<" xCZ gates)\n";
        cout << approx_type.str();
        if (config -> proc_prefix_bits)
            cout << "Simulating xCZ gates : using projection-based branches\n";
        else
            cout << "Simulating xCZ gates : exactly\n";
        
    }
    else if (config -> sim_type == Config::SimType::LosslessV
             || config -> sim_type == Config::SimType::ApproxCZPathV2011) {
        cout << "sum of tensor products / single cut\n";
        cout << amp.log[log_count++] << " (" << twoq_gates.second <<" xCZ gates)\n";
        cout << approx_type.str();
        if (config -> proc_prefix_bits)
            cout << "Simulating xCZ gates : using projection-based branches\n";
        else
            cout << "Simulating xCZ gates : exactly\n";    }
    else if (config -> sim_type == Config::SimType::Approx1CutH) {
        cout << "tensor products / approx single cut\n";
        cout << amp.log[log_count++];
        cout << approx_type.str();
        cout << "Simulating xCZ gates : ignored\n";
    }
    else if (config -> sim_type == Config::SimType::Approx1CutV) {
        cout << "tensor products / approx single cut\n";
        cout << amp.log[log_count++];
        cout << approx_type.str();
        cout << "Simulating xCZ gates : ignored\n";
    }
    else if (config -> sim_type == Config::SimType::Approx2011) {
        cout << "tensor products / approx2011 \n";
        cout << amp.log[log_count++];
        cout << approx_type.str();
        cout << "Simulating xCZ gates : approx\n";
    }
    else if (config -> sim_type == Config::SimType::Approx_i11i) {
        cout << "tensor products / approx-i11i \n";
        cout << amp.log[log_count++];
        cout << approx_type.str();
        cout << "Simulating xCZ gates : approx\n";
    }
    else if (config -> sim_type == Config::SimType::ApproxOWT) {
        cout << "sum of tensor products / approx 2 cuts \n";
        cout << amp.log[log_count++] << "\n";
        cout << amp.log[log_count++];
        cout << approx_type.str();
        cout << "Simulating xCZ gates : approx\n";
       
    }
    else if (config -> sim_type == Config::SimType::Approx2011OWT) {
        cout << "sum of tensor products / approx 2 cuts (2011) \n";
        cout << amp.log[log_count++] << "\n";
        cout << amp.log[log_count++];
        cout << approx_type.str();
        cout << "Simulating xCZ gates : approx\n";
        
    }
    else if (config -> sim_type == Config::SimType::Approx_i11iOWT) {
        cout << "sum of tensor products / approx 2 cuts (-i11i) \n";
        cout << amp.log[log_count++] << "\n";
        cout << amp.log[log_count++];
        cout << approx_type.str();
        cout << "Simulating xCZ gates : approx\n";
        
    }
    
    if (config -> sim_type == Config::SimType::FullState)
        cout << "Low-value qubits : " << amp.GetNumQInBlock(0) - config -> th << " q\n";
    else {
        cout << "Low-value qubits : "
        << amp.GetNumQInBlock(0) - (amp.GetNumQInBlock(0) / 2 < 18 ? amp.GetNumQInBlock(0) / 2 : 15) << " q, "
        << amp.GetNumQInBlock(1) - (amp.GetNumQInBlock(1) / 2 < 18 ? amp.GetNumQInBlock(1) / 2 : 15) << " q\n";
    }
    
    if (config -> proc_prefix_bits) {
        cout << "xCZ path breakdown : " << config -> proc_prefix_bits << "p" ;
        if (config -> ranges_bits)
            cout << " + " << config -> ranges_bits << "r";
        if (config -> dfs_length)
            cout << " + " << config -> dfs_length << "b";
        cout << "\n";
        if (config -> ranges_bits) {
            cout << "xCZ paths: ";
            
            const int total_cz_bits = config -> proc_prefix_bits + config -> ranges_bits + config -> dfs_length;
            const idx_size czb_v = config -> cz_path << (config -> ranges_bits + config -> dfs_length);
            const idx_size cze_v = czb_v + (1ull << (config -> ranges_bits + config -> dfs_length)) - 1;
            const string czb = bitset<128>(czb_v).to_string();
            const string cze = bitset<128>(cze_v).to_string();
            cout << "[" << czb.substr(czb.size() - total_cz_bits) << " (" << czb_v << "), "
            << cze.substr(cze.size() - total_cz_bits) << " (" << cze_v << "))";
        }
        else {
            const string czp = bitset<128>(config -> cz_path).to_string();
            cout << "CZ path : " << czp.substr(czp.size() - config -> proc_prefix_bits)
            << " (" << config -> proc_prefix_bits << ")";
        }
        
        cout << "\n";
        config -> verbose = Config::Verbose::NCC;
    }
    
//    else
//       cout << "None\n";
}

void SequentialSimulation::
PrintSimReport(GenericQuantumState& amp,
               const Circuit& circuit) const
{
    if (config -> sim_type == Config::SimType::ApproxOWT || config -> sim_type == Config::SimType::Approx2011OWT
        || config -> sim_type == Config::SimType::Approx_i11iOWT)
        amp.Normalize();
    
    idx_size temp_amp_size = amp.GetFullStateVectorSize();
    
    if (config -> proc_prefix_bits) {
        cout << "Cycle breakdown : ";
        if (amp.count_of_category.cycle_p)
            cout << amp.count_of_category.cycle_p << "p";
        if (amp.count_of_category.cycle_r && amp.count_of_category.cycle_p == 0 )
            cout << amp.count_of_category.cycle_r << "p & r";
        else if (amp.count_of_category.cycle_r )
            cout << " + " << amp.count_of_category.cycle_r - amp.count_of_category.cycle_p << "r";
        if (amp.count_of_category.cycle_d && amp.count_of_category.cycle_r)
            cout << " + " << amp.count_of_category.cycle_d - amp.count_of_category.cycle_r << "b";
        else if (amp.count_of_category.cycle_d)
            cout << " + " << amp.count_of_category.cycle_d - amp.count_of_category.cycle_p << "b";
            
        cout << "\n";
    }
    
    cout << "Layers simulated :";
    cout << " H (" << amp.count_of_category.H_layers/adjustment_factor << "), " ;
    cout << "CZ & T (" << amp.count_of_category.CZT_layers/adjustment_factor << "), " ;
    if (amp.count_of_category.last_H)
        cout << "X & Y & H (" << amp.count_of_category.XY_layers/adjustment_factor << ")\n" ;
    else
        cout << "X & Y (" << amp.count_of_category.XY_layers/adjustment_factor << ")\n" ;

    if (config -> proc_prefix_bits) {
        cout << "Layers breakdown : ";
        if (amp.count_of_category.proc_prefix_layers)
            cout << amp.count_of_category.proc_prefix_layers / adjustment_factor << "p";
        if (amp.count_of_category.cycle_r && amp.count_of_category.cycle_p == 0)
            cout << amp.count_of_category.ranges_layers / adjustment_factor << "p & r";
        else if (amp.count_of_category.cycle_r )
            cout << " + " << amp.count_of_category.ranges_layers / adjustment_factor << "r";
        if (amp.count_of_category.cycle_d)
            cout << " + " << amp.count_of_category.branch_layers / adjustment_factor << "b";
        
        cout << "\n";
    }
    if (config -> print_amp)
        cout << "Requested num amps : " << config -> indices.size() - 5 << "\n";
    
    cout << "\n";
    
    {
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);

        double memory =  sizeof(cmplx) * amp.GetSize();
        ss << "State representation size : ";

        if (memory >= (1 << 30)) {
            ss << memory / (1 << 30) << " GiB \n";
        }
        else if (memory >= (1 << 20)) {
            ss << memory / (1 << 20) << " MiB \n";
        }
        else if (memory >= (1 << 10)) {
            ss << memory / (1 << 10) << " KiB \n";
        }
        else
            ss << memory << " B \n";
        
        if (config -> count_zeros) {
            if (config -> proc_prefix_bits && (config -> ranges_bits || config -> dfs_length)) {
                ss << "Zero count \n";
                ss << "\t1st Checkpoint : ";
                if (config -> ranges_bits) {
                    if (config -> store_checkpoint_range)
                    ss << "A = " << amp.count_of_category.zero_count_cp1_A << " ("
                    << ((double)amp.count_of_category.zero_count_cp1_A /
                        (double)(1ull << amp.GetNumQInBlock(0))) * 100.0 << "%), ";
                    ss << "B = " << amp.count_of_category.zero_count_cp1_B << " ("
                    << ((double)amp.count_of_category.zero_count_cp1_B /
                        (double)(1ull << amp.GetNumQInBlock(1))) * 100.0 << "%)\n";
                    ss << "\t2nd Checkpoint : ";
                }
                if (config -> dfs_length) {
                    idx_size A_avg_0s = amp.count_of_category.zero_count_cp2_A /
                    ((1ull << config -> ranges_bits) * (1ull << config -> dfs_length));
                    idx_size B_avg_0s = amp.count_of_category.zero_count_cp2_B /
                    ((1ull << config -> ranges_bits) * (1ull << config -> dfs_length));
                    ss << "A = " << A_avg_0s << " (" << ((double)A_avg_0s /
                                                         (double)(1ull << amp.GetNumQInBlock(0))) * 100.0 << "%), ";
                    ss << "B = " << B_avg_0s << " (" << ((double)B_avg_0s /
                                                         (double)(1ull << amp.GetNumQInBlock(1))) * 100.0 << "%)\n";
                }
            }
        }
        
        
        if (config -> verbose >= Config::Verbose::Default && config -> proc_prefix_bits == 0) {
            double norm = amp.CalculateNormSquared();
            ss << "Norm";
            if (amp.GetNumAddends() > 1)
                ss << "(assuming orthogonal addends)";
            ss << " : " << sqrt(norm) << "\n";
            
            double avg_inacc = amp.CalculateAverageInaccuracy(norm);

            double min = amp.GetMinProb();
            if (sqrt(norm) > 0.9) {
                ss << "Mean entropy : " <<  amp.CalculateMeanEntropy() << " ";
                ss << "Cross entropy : " <<  amp.CalculateCrossEntropy(SAMPLING_FACTOR) << "\n";
            }
            ss << "Probabilities : " << amp.GetMinProb() << "(min), "
                 << amp.GetMaxProb() << "(max), "
                 << amp.GetAvgProb() << "(avg)\n";
            
            if (min)
               ss << "Log_2 (max / min) = " << log2(real(amp.GetMaxProb())/real(amp.GetMinProb())) << "\n" ;
            if (norm >= 0.9)
                ss << "Avg inaccuracy per probability > " << avg_inacc << " ("<< (avg_inacc/amp.GetAvgProb()) * 100 << "%)\n";
            
            double zero_amps_A = amp.CountZeroAmpPercentage();
            double zero_amps_B = 0;
            if (amp.GetNumAddends())
                zero_amps_B = amp.CountZeroAmpPercentage();
            
            if (zero_amps_A || zero_amps_B) {
                ss << "Stored zero amplitudes : ";
                if (amp.GetNumAddends()) {
                    ss << " 1st block = ";
                    ss << zero_amps_A << "% 2nd block = " ;
                    ss << zero_amps_B << "%\n";
                }
                else {
                   ss << zero_amps_A << "%\n";
                }
            }
        }
        ss << "\n";
        cout << ss.str();
    }
    
    if (config -> proc_prefix_bits == 0) {
        
        string key = config -> infile.substr(0, config -> infile.find_first_of('.')) + "_" + to_string(circuit.GetNumCycles());
        cout << "Correctness check : ";
    
        if (benchmark.count(key)) {
            if (real(amp[3]) - real(benchmark[key][0]) < 1e-9
                && imag(amp[3]) - imag(benchmark[key][0]) < 1e-9
                && real(amp[temp_amp_size/4]) - real(benchmark[key][1]) < 1e-9
                && imag(amp[temp_amp_size/4]) - imag(benchmark[key][1]) < 1e-9
                && real(amp[temp_amp_size/2]) - real(benchmark[key][2]) < 1e-9
                && imag(amp[temp_amp_size/2]) - imag(benchmark[key][2]) < 1e-9
                && real(amp[3 * temp_amp_size/4]) -  real(benchmark[key][3]) < 1e-9
                && imag(amp[3 * temp_amp_size/4]) -  imag(benchmark[key][3]) < 1e-9
                && real(amp[temp_amp_size - 3]) - real(benchmark[key][4]) < 1e-9
                && imag(amp[temp_amp_size - 3]) - imag(benchmark[key][4]) < 1e-9)
            {
                cout << "passed\n";
            }
            else {
                cout << "failed\nCorrect results: \n";
                if (config -> verbose != Config::Verbose::NCCV) {
                    string imag0 = to_string(imag(benchmark[key][0])), imag1 = to_string(imag(benchmark[key][1])),
                    imag2 = to_string(imag(benchmark[key][2])), imag3 = to_string(imag(benchmark[key][3])),
                    imag4 = to_string(imag(benchmark[key][4]));
                    
                    cout << "amp[3]  \t= "  << real(benchmark[key][0]) ;
                    if (imag(benchmark[key][0]) < 0)
                        cout << " - " << abs(imag(benchmark[key][0])) << "j\n";
                    else
                        cout << " + " << imag(benchmark[key][0]) << "j\n";
                    
                    cout << "amp[1/4]\t= " << real(benchmark[key][1]);
                    if (imag(benchmark[key][1]) < 0)
                        cout << " - " << abs(imag(benchmark[key][1])) << "j\n";
                    else
                        cout << " + " << imag(benchmark[key][1]) << "j\n";
                    
                    cout << "amp[1/2]\t= " << real(benchmark[key][2]);
                    if (imag(benchmark[key][2]) < 0)
                        cout << " - " << abs(imag(benchmark[key][2])) << "j\n";
                    else
                        cout << " + " << imag(benchmark[key][2]) << "j\n";
                    
                    cout << "amp[3/4]\t= " << real(benchmark[key][3]);
                    if (imag(benchmark[key][3]) < 0)
                        cout << " - " << abs(imag(benchmark[key][3])) << "j\n";
                    else
                        cout << " + " << imag(benchmark[key][3]) << "j\n";
                    
                    cout << "amp[-3] \t= "  << real(benchmark[key][4]);
                    if (imag(benchmark[key][4]) < 0)
                        cout << " - " << abs(imag(benchmark[key][4])) << "j\n";
                    else
                        cout << " + " << imag(benchmark[key][4]) << "j\n";
                    
                    cout << "Incorrect results: \n";
                }
            }
        }
        else
            cout << "no data available\n";
    }
    
    if (config -> verbose != Config::Verbose::NCCV)
    {
        if (!amps_of_interest.empty()) {
            auto amp3 = amps_of_interest[0];
            cout << "amp[3]  \t= " << real(amp3);
            if (imag(amp3) < 0)
                cout << " - " << abs(imag(amp3)) << "j\n";
            else
                cout << " + " << imag(amp3) << "j\n";
            
            auto amp14 = amps_of_interest[1];
            cout << "amp[1/4]\t= " << real(amp14);
            if (imag(amp14) < 0)
                cout << " - " << abs(imag(amp14)) << "j\n";
            else
                cout << " + " << imag(amp14) << "j\n";
            
            auto amp12 = amps_of_interest[2];
            cout << "amp[1/2]\t= " << real(amp12);
            if (imag(amp12) < 0)
                cout << " - " << abs(imag(amp12)) << "j\n";
            else
                cout << " + " << imag(amp12) << "j\n";
            
            auto amp34 = amps_of_interest[3];
            cout << "amp[3/4]\t= " << real(amp34);
            if (imag(amp34) < 0)
                cout << " - " << abs(imag(amp34)) << "j\n";
            else
                cout << " + " << imag(amp34) << "j\n";
            
            auto amp_3 = amps_of_interest[4];
            cout << "amp[-3] \t= " << real(amp_3);
            if (imag(amp_3) < 0)
                cout << " - " << abs(imag(amp_3)) << "j\n";
            else
                cout << " + " << imag(amp_3) << "j\n";
            
            cout << "\n";
        }
        else if (config -> proc_prefix_bits == 0) {
            cout << "amp[3]  \t= " << real(amp[3]);
            if (imag(amp[3]) < 0)
                cout << " - " << abs(imag(amp[3])) << "j\n";
            else
                cout << " + " << imag(amp[3]) << "j\n";
            
            cout << "amp[1/4]\t= " << real(amp[temp_amp_size/4]);
            if (imag(amp[temp_amp_size/4]) < 0)
                cout << " - " << abs(imag(amp[temp_amp_size/4])) << "j\n";
            else
                cout << " + " << imag(amp[temp_amp_size/4]) << "j\n";
            
            cout << "amp[1/2]\t= " << real(amp[temp_amp_size/2]);
            if (imag(amp[temp_amp_size/2]) < 0)
                cout << " - " << abs(imag(amp[temp_amp_size/2])) << "j\n";
            else
                cout << " + " << imag(amp[temp_amp_size/2]) << "j\n";
            
            cout << "amp[3/4]\t= " << real(amp[3 * temp_amp_size/4]);
            if (imag(amp[3 * temp_amp_size/4]) < 0)
                cout << " - " << abs(imag(amp[3 * temp_amp_size/4])) << "j\n";
            else
                cout << " + " << imag(amp[3 * temp_amp_size/4]) << "j\n";
            
            cout << "amp[-3] \t= " << real(amp[temp_amp_size - 3]);
            if (imag(amp[temp_amp_size - 3]) < 0)
                cout << " - " << abs(imag(amp[temp_amp_size - 3])) << "j\n";
            else
                cout << " + " << imag(amp[temp_amp_size - 3]) << "j\n";
            
            cout << "\n";
        }
        else {
            cerr << "Can't print state vector without amp_of_interest vector for xCZ path simulation\n";
            exit(1);
        }
    }
    
    {
        int width = 43;
        double sum_percen = (amp.time_by_category.initial_H/total_time) * 100;
        ostringstream ss (ostringstream::ate);
        ss << setprecision(3);
        ss << "Runtime (" << total_time << " s total) by category \n";
        
        string H_s = "\tInitial H (" + to_string(amp.count_of_category.initial_H) +  ")";
        ss << H_s << setw(width - (int)H_s.size()) << right << ": " << amp.time_by_category.initial_H
        << " s  \t\t  =  " << (amp.time_by_category.initial_H/total_time) * 100 << "%\n";
        
        if (amp.count_of_category.last_H) {
            string H_s = "\tLast H (" + to_string(amp.count_of_category.last_H) +  ")";
            ss << H_s << setw(width - (int)H_s.size()) << right << ": " << amp.time_by_category.last_H
            << " s  \t\t  =  " << (amp.time_by_category.last_H/total_time) * 100 << "%\n";
            sum_percen = (amp.time_by_category.last_H/total_time) * 100;
        }

        if(amp.count_of_category.decomposed_CZ) {
            string CZ_s = "\txCZ (" + to_string(amp.count_of_category.decomposed_CZ/adjustment_factor) + ")" ;
            ss << CZ_s << setw(width - (int)CZ_s.size()) << right << ": "
            << amp.time_by_category.decomposed_CZ << " s  \t\t  =  "
            << (amp.time_by_category.decomposed_CZ/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.decomposed_CZ/(total_time)) * 100;
        }
        
        if(amp.count_of_category.CZ_T - amp.count_of_category.decomposed_CZ || amp.count_of_category.low_q_XY1_2) {
            string CZ_T_s = "\tCZ & T (" +
            to_string((amp.count_of_category.CZ_T - amp.count_of_category.decomposed_CZ)/adjustment_factor)
            + "), Low X & Y (" + to_string(amp.count_of_category.low_q_XY1_2) + ")";
            if (amp.count_of_category.H_merged_lo)
                CZ_T_s += " & H (" + to_string(amp.count_of_category.H_merged_lo) +  ")";
            
            ss << CZ_T_s << setw(width - (int)CZ_T_s.size()) << right << ": "
            << amp.time_by_category.low_q_XY_CZT << " s  \t\t  =  "
            << (amp.time_by_category.low_q_XY_CZT/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.low_q_XY_CZT/(total_time)) * 100;
        }
        
        if (amp.count_of_category.X1_2 || amp.count_of_category.Y1_2) {
            string XY_s = "\tSingle X (" + to_string(amp.count_of_category.X1_2)
            + ") & Y (" + to_string(amp.count_of_category.Y1_2) + ")";
            ss << XY_s << setw(width - (int)XY_s.size()) << right << ": "
            << (amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2) << " s  \t\t  =  "
            << ((amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2)/(total_time)) * 100 << "%\n";
            sum_percen += ((amp.time_by_category.X1_2 +  amp.time_by_category.Y1_2)/(total_time)) * 100;
        }
        
        if (amp.count_of_category.merged_XY1_2) {
            string X_Y_s = "\tMerged X & Y ("
            + to_string(amp.count_of_category.merged_XY1_2) + ")";
            ss << X_Y_s << setw(width - (int)X_Y_s.size()) << right << ": " << amp.time_by_category.merged_XY1_2
            << " s  \t\t  =  " << (amp.time_by_category.merged_XY1_2/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.merged_XY1_2/(total_time)) * 100;
        }
        
        if(amp.count_of_category.high_q_XY1_2) {
            string xy_s = "\tHigh X & Y (" + to_string(amp.count_of_category.high_q_XY1_2) + ")";
            if (amp.count_of_category.H_merged_hi)
                xy_s += " & H (" + to_string(amp.count_of_category.H_merged_hi) +  ")";
            ss << xy_s << setw(width - (int)xy_s.size()) << right << ": "
            << amp.time_by_category.high_q_XY1_2 << " s  \t\t  =  "
            << (amp.time_by_category.high_q_XY1_2/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.high_q_XY1_2/(total_time)) * 100;
        }
        
        if (amp.count_of_category.rescale) {
            string RP_s = "\tRescaling passes (" + to_string(amp.count_of_category.rescale/adjustment_factor) + ")";
            ss <<  RP_s << setw(width - (int)RP_s.size()) << right << ": " << (amp.time_by_category.rescale)
            << " s\t\t  =  " << (amp.time_by_category.rescale/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.rescale/(total_time)) * 100;
        }
        
        if (amp.time_by_category.conversion) {
            string RP_s = "\tConversion ";
            ss <<  RP_s << setw(width - (int)RP_s.size()) << right << ": "
            << amp.time_by_category.conversion<< " s  \t\t  =  "
            << (amp.time_by_category.conversion/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.conversion/(total_time)) * 100;
        }
        
        if (amp.time_by_category.copying) {
            string RP_s = "\tCopying (" + to_string(amp.count_of_category.copying) + ")";
            ss <<  RP_s << setw(width - (int)RP_s.size()) << right << ": "
            << amp.time_by_category.copying << " s  \t\t  =  "
            << (amp.time_by_category.copying/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.copying/(total_time)) * 100;
        }
        
        if (amp.time_by_category.compress) {
            string RP_s = "\tCompression (" + to_string(amp.count_of_category.compress) + ")";
            ss <<  RP_s << setw(width - (int)RP_s.size()) << right << ": "
            << amp.time_by_category.compress << " s  \t\t  =  "
            << (amp.time_by_category.compress/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.compress/(total_time)) * 100;
        }
        
        if (amp.time_by_category.decompress) {
            string RP_s = "\tDecompression (" + to_string(amp.count_of_category.decompress) + ")";
            ss <<  RP_s << setw(width - (int)RP_s.size()) << right << ": "
            << amp.time_by_category.decompress << " s  \t\t  =  "
            << (amp.time_by_category.decompress/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.decompress/(total_time)) * 100;
        }
        
        if(amp.time_by_category.norm && config -> norm_depth) {
            string RP_n = "\tNorm ";
            ss <<  RP_n << setw(width - (int)RP_n.size()) << right << ": "
            << amp.time_by_category.norm << " s  \t\t  =  "
            << (amp.time_by_category.norm/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.norm/(total_time)) * 100;
        }
        
        if(amp.time_by_category.amp_storage) {
            string RP_s = "\tStoring amps ";
            ss <<  RP_s << setw(width - (int)RP_s.size()) << right << ": "
            << amp.time_by_category.amp_storage << " s  \t\t  =  "
            << (amp.time_by_category.amp_storage/(total_time)) * 100 << "%\n";
            sum_percen += (amp.time_by_category.amp_storage/(total_time)) * 100;
        }
        
        idx_size factor1 = (1ull << config -> ranges_bits) * (1ull << config -> dfs_length);
          
        ss << "\t\t\t\t\t\t\t\t\t    -------\n";
        ss << "\tTotal \t\t\t\t\t\t\t\t    " << sum_percen << "%\n";
        
        ss << "\nAverage time per gate : " << total_time/(factor1 * circuit.GetTotalNumGates()) << " s\n";
        
        if (config -> proc_prefix_bits) {
            ss << "Simulation runtime breakdown : \n\tPrefix    \t\t : " << prefix_time << " s = "
             << (prefix_time/(total_time)) * 100 << "%\n";
        }
        if (config -> dfs_length != 0) {
            ss << "\tBranches  \t\t : " << branch_time << " s = "
            << (branch_time/(total_time)) * 100 << "%\n";
        }
        if (mmap_time != 0) {
            ss << "\tMemory mapped I/O : " << mmap_time << " s = "
            << (mmap_time/(total_time)) * 100 << "%\n";
        }
        
        ss << "Simulation memory usage : " ;
        if (memory_usage >= (1 << 30)) {
            ss << memory_usage / (1 << 30) << " GiB \n";
        }
        else if (memory_usage >= (1 << 20)) {
            ss << memory_usage / (1 << 20) << " MiB \n";
        }
        else if (memory_usage >= (1 << 10)) {
            ss << memory_usage / (1 << 10) << " KiB \n";
        }
        else
            ss << memory_usage << " B \n";
        
        cout << ss.str() << "\n";
    }
    
    
//    if (config -> verbose == Config::Verbose::Cycles) {
//        cout << log.str() << "\n";
//        cout << "Cycle\t xCZ\t Addends\t  Memory\n";
//        for (idx_size c = 0; c < amp.data_per_cycles.memory.size(); ++c) {
//            cout << amp.data_per_cycles.cycles[c] << "\t "
//            << amp.data_per_cycles.xCZ_H[c] << "," << amp.data_per_cycles.xCZ_V[c] << "\t "
//            << amp.data_per_cycles.addends[c] << "\t  " ;
//            if (amp.data_per_cycles.memory[c] >= (1 << 30)) {
//                cout << amp.data_per_cycles.memory[c] / (1 << 30) << " GiB \n";
//            }
//            else if (amp.data_per_cycles.memory[c] >= (1 << 20)) {
//                cout << amp.data_per_cycles.memory[c] / (1 << 20) << " MiB \n";
//            }
//            else if (amp.data_per_cycles.memory[c] >= (1 << 10)) {
//                cout << amp.data_per_cycles.memory[c] / (1 << 10) << " KiB \n";
//            }
//            else
//                cout << amp.data_per_cycles.memory[c] << " B \n";
//        }
//    }
    
    cout << "¯\\_(ツ)_/¯ \n\n";
}

void SequentialSimulation::
PrintReportToFile(GenericQuantumState& amp,
                  const Circuit& circuit) const
{
}


