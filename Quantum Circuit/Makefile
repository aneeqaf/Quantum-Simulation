CC := g++-7 
#/usr/local/gcc7/bin/
# CC := clang --analyze # and comment out the linker last line for sanity
SRCDIR := vector_state_sim
BUILDDIR := build
TARGET := bin/vector_state_sim
LIBS = -lm -lstdc++
INC = -I libs -I sim -I sim/kernels
CFLAGS = -O3 -Wall -std=c++17 -march=native -Ofast -fmerge-all-constants -fmodulo-sched \
-fmodulo-sched-allow-regmoves -fsplit-ivs-in-unroller -ffast-math -fgcse-las -m64 \
-DManualMerging -DCPUname 
# CFLAGS = -g3 -S -Wall -std=c++17 -mavx -mavx2 -msse2avx -DManualMerging

.PHONY: clean all

default: $(TARGET)
all: default

SIMOBJECTS = $(patsubst %.cpp, %.o, $(wildcard sim/*.cpp))
KERNELSOBJECTS = $(patsubst %.cpp, %.o, $(wildcard sim/kernels/*.cpp))
LIBSOBJECTS = $(patsubst %.cpp, %.o, $(wildcard libs/*.cpp))
EXE = $(patsubst %.cpp, %.o, vector_state_sim/main.cpp)
BM = $(patsubst %.cpp, %.o, benchmark/XYmicroBM.cpp)

OBJECTS = $(SIMOBJECTS) $(KERNELSOBJECTS) $(LIBSOBJECTS)

$(OBJECTS) $(BM): %.o: %.cpp %.h
	@echo "Building dependencies..."
	@echo "$(CC) $(CFLAGS) -c $< -o $@"; $(CC) $(CFLAGS) $(INC) -c $< -o $@

$(EXE): %.o: %.cpp
	@echo "Building exe..."
	@echo "$(CC) $(CFLAGS) -c $< -o $@"; $(CC) $(CFLAGS) $(INC) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(EXE) $(OBJECTS)
	$(CC) $(EXE) $(OBJECTS) -Wall $(INC) $(LIBS) -o $@

clean:
	-rm -f $(SRCDIR)/*.o sim/*.o libs/*.o sim/kernels/*.o benchmark/*.o $(TARGET)

# Benchmark
bm: $(OBJECTS) $(BM)
	$(CC) $(CFLAGS) $(BM) $(OBJECTS) $(INC) $(LIBS) benchmark/main.cpp -o bin/bm


# export LD_LIBRARY_PATH=/usr/local/gcc7/lib64
