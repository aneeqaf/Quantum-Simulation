//
//  mmap.cpp
//  vector_state_sim
//
//  Created by Aneeqa Fatima on 4/7/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#include "mmap.h"

MMapContent::
MMapContent(string file_n,
            size_t size,
            bool initialize): map_ptr(nullptr), size(size), filename(file_n)
{
    if (initialize)
        InitializeMemMap(filename, size);
}

MMapContent::
MMapContent(const MMapContent& rhs)
{
    size = rhs.size;
    fd = rhs.fd;
    filename = rhs.filename;
    InitializeMemMap(filename, size);
}

MMapContent& MMapContent::
operator=(const MMapContent& rhs)
{
    MMapContent temp(rhs);
    swap(map_ptr, temp.map_ptr);
    swap(size, temp.size);
    swap(fd, temp.fd);
    return *this;
}

MMapContent::
~MMapContent()
{
    if (map_ptr) {
        if (munmap(map_ptr, size) == -1)
        {
            close(fd);
            perror("Error un-mmapping the file");
            exit(EXIT_FAILURE);
        }
        
        // Un-mmaping doesn't close the file, so we still need to do that.
        close(fd);
    }
    map_ptr = nullptr;
}

void MMapContent::
InitializeMemMap(const string file_n,
                 size_t size)
{
    auto does_file_exist = [](const char *fileName)
    {
        ifstream infile(fileName);
        return (bool)infile.good();
    };
    
    auto is_file_empty = [] (const char *fileName)
    {
        ifstream infile(fileName);
        return infile.peek() == std::ifstream::traits_type::eof();
    };
    
    bool new_file = !does_file_exist(filename.c_str()) || is_file_empty(filename.c_str());
    fd = open(filename.c_str(), O_RDWR | O_CREAT , (mode_t)0600);
    map_ptr = (cmplx*)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_ptr == MAP_FAILED)
    {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }
    
    if (new_file) {
        
        if (lseek(fd, size - 1, SEEK_SET) == -1)
        {
            close(fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }
        
        if (write(fd, "", 1) == -1)
        {
            close(fd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }
        for (size_t i = 0; i < size/sizeof(cmplx); ++i)
            map_ptr[i] = 0;
    }
}

cmplx* MMapContent::
operator[](size_t i)
{
    return &map_ptr[i];
}


void MMapContent::
WriteToDisk()
{
    if (msync(map_ptr, size, MS_SYNC) == -1)
    {
        perror("Could not sync the file to disk");
    }
}
