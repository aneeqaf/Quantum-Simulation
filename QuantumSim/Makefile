CC := g++-7
#/usr/local/gcc7/bin/
# CC := clang --analyze # and comment out the linker last line for sanity
SRCDIR := vector_state_sim
TARGETDIR := bin
OUTPUT := output/amp_vectors output/probabilities output/qpro_scripts output/reports
INPUT := input/random_circuits_aneeqa
TARGET := bin/rollright_sim
LIBS = -lm -lstdc++
INC = -I libs -I sim -I sim/kernels
CFLAGS = -O3 -Wall -std=c++17 -march=native -Ofast -fmerge-all-constants -fmodulo-sched \
-fmodulo-sched-allow-regmoves -fsplit-ivs-in-unroller -ffast-math -fgcse-las -m64 -mavx -mavx2 -msse2avx \
-march=core-avx-i -mtune=core-avx-i  -DRT -fopenmp -DParallel  -DCosineSimilarityDoubled -DFidelityDoubled
# -DCosineSimilarity -DPorterThomas -DReportToFile -DCosineSimilarityDoubled -DFidelityDoubled
# CFLAGS = -g3 -S -Wall -std=c++17 -mavx -mavx2 -msse2avx -DManualMerging

.PHONY: clean all

default: $(TARGET)
all: default

SIMOBJECTS = $(patsubst %.cpp, %.o, $(wildcard sim/*.cpp))
KERNELSOBJECTS = $(patsubst %.cpp, %.o, $(wildcard sim/kernels/*.cpp))
LIBSOBJECTS = $(patsubst %.cpp, %.o, $(wildcard libs/*.cpp))
EXE = $(patsubst %.cpp, %.o, vector_state_sim/main.cpp)
BM = $(patsubst %.cpp, %.o, benchmark/XYmicroBM.cpp)
KERNELHEADERS = $(wildcard sim/kernels/*.h)

OBJECTS = $(SIMOBJECTS) $(KERNELSOBJECTS) $(LIBSOBJECTS)

$(OBJECTS) $(BM): %.o: %.cpp %.h 
	@echo "Building dependencies..."
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(EXE): %.o: %.cpp
	@echo "Building exe..."
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(TARGETDIR): 
	mkdir -p $(TARGETDIR)

$(OUTPUT): 
	mkdir -p $(OUTPUT)

$(INPUT): 
	mkdir -p $(INPUT)

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(TARGETDIR) $(OUTPUT) $(INPUT) $(EXE) $(OBJECTS) 
	$(CC) $(EXE) $(OBJECTS) -Wall $(INC) $(LIBS) -fopenmp -o $@

clean:
	-rm -f $(SRCDIR)/*.o sim/*.o libs/*.o sim/kernels/*.o benchmark/*.o bin/rollright_sim bin/bm

# Benchmark
bm: $(OBJECTS) $(BM) $(KERNELHEADERS)
	$(CC) $(CFLAGS) $(BM) $(OBJECTS) $(INC) $(LIBS) benchmark/main.cpp -o bin/bm


# export LD_LIBRARY_PATH=/usr/local/gcc7/lib64