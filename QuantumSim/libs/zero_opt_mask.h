//
//  zero_opt_mask.h
//  QuantumSimProj
//
//  Created by Aneeqa Fatima on 4/2/18.
//  Copyright © 2018 Aneeqa Fatima. All rights reserved.
//

#ifndef zero_opt_mask_h
#define zero_opt_mask_h

using idx_size = size_t;

class ZeroOptMask {
private:
    idx_size nonzero_mask; // 1 represents all asterisks. Initialzed to all bits set.
    idx_size zero_mask; //1s represent odd zeros. Initialized to all bits set.
    idx_size num_qubits;
    
public:
    ZeroOptMask(): nonzero_mask(0), zero_mask(0), num_qubits(0) {}
    ZeroOptMask(idx_size s): nonzero_mask(0), zero_mask(0), num_qubits(s) {}
    
    void reset()
    {
        nonzero_mask = 0;
        zero_mask = 0;
    }
    
    idx_size size() const { return num_qubits; }
    
    char operator[](const idx_size q) const
    {
        if (!(nonzero_mask & (1ull << q)))
            return '*';
        else if (zero_mask & (1ull << q))
            return '1';
        else return '0';
    }
    
    void SetOddBit(idx_size q)
    {
        zero_mask |= 1ull << q;
        nonzero_mask |= 1ull << q;
    }
    
    void SetEvenBit(idx_size q)
    {
        zero_mask &= ~(1ull << q);
        nonzero_mask |= 1ull << q;
    }
    
    void SetNonZeroBit(idx_size q)
    {
        nonzero_mask &= ~(1ull << q);
    }
    
    string print() const
    {
        string mask = "";
        for (int i = (int)(num_qubits - 1); i >= 0; --i)
            mask += (*this)[i];
        
        return mask;
    }
    
    bool CheckIfBlockIsNotZero(idx_size idx,
                            idx_size block_size) const
    {
        return (idx & nonzero_mask) ? (zero_mask & block_size) ^ (idx & nonzero_mask) : true;
    }
};

#endif /* zero_opt_mask_h */
