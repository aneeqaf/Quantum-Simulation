//
//  mmap.h
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 4/7/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#ifndef mmap_h
#define mmap_h

#include <stdio.h>
#include <complex>
#include <fstream>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <vector>
#include <utility>
#include <dirent.h>

using namespace std;

using cmplx = complex<float>;

class MMapContent {
    cmplx* map_ptr;
    size_t size;
    int fd;
    string filename;
    
public:
    void InitializeMemMap(const string filename,
                          size_t size);
    cmplx* operator[](size_t i);
    void WriteToDisk();
    
    cmplx* GetMapPtr() { return map_ptr; }
    
    MMapContent(): map_ptr(nullptr), size(0), fd(0), filename("") {};
    MMapContent(const string filename,
                const size_t size,
                bool initialize = false);
    MMapContent(const MMapContent& rhs);
    MMapContent& operator=(const MMapContent& rhs);
    ~MMapContent();
};

#endif /* mmap_h*/
