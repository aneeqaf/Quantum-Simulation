CC := clang++
#/usr/local/gcc7/bin/
# CC := clang --analyze # and comment out the linker last line for sanity
# SZPATH := /usr/local
SRCDIR := vector_state_sim
TARGETDIR := bin
OUTPUT := output/amp_vectors output/probabilities output/log output/reports output/misc
INPUT := input/random_circuits_google
TARGET := bin/rr
LIBS = -lm -lstdc++ -lsleef -lomp -latomic -lcusolver -lcudart -llapacke -llapack -lblas -lopenblas -L/usr/local/cuda/lib64 
# SZFLAG 	= $(SZPATH)/lib/libSZ.a $(SZPATH)/lib/libzlib.a $(SZPATH)/lib/libzstd.a
INC = -Ilibs -Isim -Isim/kernels -I/usr/local/cuda/include -I/usr/include/x86_64-linux-gnu/
CFLAGS = -O3 -Otime -Wall -std=c++17 -march=native -Ofast -fmerge-all-constants -m64 -mavx -mavx2 -march=core-avx-i -DRT -Xpreprocessor -fopenmp -mfma  -DParallel
# -DCosineSimilarity -DPorterThomas -DReportToFile -DCosineSimilarityDoubled -DFidelityDoubled  -ffast-math -DPrint 
# CFLAGS = -g  -fno-omit-frame-pointer -O0 -Wall -std=c++17 -m64 -mavx -mavx2 -mfma -march=core-avx-i -mtune=core-avx-i  -DParallel  
.PHONY: clean all

default: $(TARGET)
all: default

SIMOBJECTS = $(patsubst %.cpp, %.o, $(wildcard sim/*.cpp))
KERNELSOBJECTS = $(patsubst %.cpp, %.o, $(wildcard sim/kernels/*.cpp))
# LIBSOBJECTS = $(patsubst %.cpp, %.o, $(wildcard libs/*.cpp))
LIBSOBJECTS = $(filter-out libs/SZ_helper.cpp, $(patsubst %.cpp, %.o, $(wildcard libs/*.cpp)))
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

TEST_SRC = tests/test_state_tree_tensor.cpp
TEST_OBJ = tests/test_state_tree_tensor.o
TEST_BIN = bin/test_tree_tensor
$(TEST_OBJ): $(TEST_SRC) sim/state_tree_tensor.cpp sim/state_tree_tensor.h
	@echo "Building test obj..."
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(TARGETDIR): 
	mkdir -p $(TARGETDIR)

$(OUTPUT): 
	mkdir -p $(OUTPUT)

$(INPUT): 
	mkdir -p $(INPUT)

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(TARGETDIR) $(OUTPUT) $(INPUT) $(EXE) $(OBJECTS) 
			$(CC) $(EXE) $(OBJECTS) -Wall $(INC) $(LIBS) $(SZFLAG) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) $(filter-out sim/state_tree_tensor.o, $(OBJECTS))
	$(CC) $^ -Wall $(INC) $(LIBS) $(SZFLAG) -o $@

clean:
	-rm -f $(SRCDIR)/*.o sim/*.o libs/*.o sim/kernels/*.o benchmark/*.o tests/*.o bin/rr $(TEST_BIN)

# export LD_LIBRARY_PATH=/usr/local/gcc7/lib64
