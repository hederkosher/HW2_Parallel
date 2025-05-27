# Makefile for macOS - Parallel Matrix Submatrix Finder
# Customized for Homebrew GCC and your file names

# Compilers and paths for macOS with Homebrew
CC = /opt/homebrew/bin/gcc-14
MPICC = /opt/homebrew/bin/mpicc
CFLAGS = -Wall -Wextra -O2 -std=c99
OPENMP_FLAGS = -fopenmp
INCLUDES = -I/opt/homebrew/opt/libomp/include -I/opt/homebrew/opt/open-mpi/include
LIBS = -L/opt/homebrew/opt/libomp/lib -lomp -lm

# Executable names
SINGLE_TARGET = matrix_solver
MPI_TARGET = matrix_solver_mpi

# Source files (adjusted to your actual file names)
SINGLE_SOURCE = matrix_solver_1.c
MPI_SOURCE = matrix_solver_mpi.c

# Default target - build single version (since you're working on that part)
all: $(SINGLE_TARGET)

# Build single computer version (OpenMP only)
$(SINGLE_TARGET): $(SINGLE_SOURCE)
	@echo "Building single computer version..."
	$(CC) $(CFLAGS) $(OPENMP_FLAGS) $(INCLUDES) -o $(SINGLE_TARGET) $(SINGLE_SOURCE) $(LIBS)
	@echo "✅ Build successful!"

# Build MPI version (for when you and your friend work together)
$(MPI_TARGET): $(MPI_SOURCE)
	@echo "Building MPI version..."
	$(MPICC) $(CFLAGS) $(OPENMP_FLAGS) $(INCLUDES) -o $(MPI_TARGET) $(MPI_SOURCE) $(LIBS)
	@echo "✅ MPI version build successful!"

# Build only single computer version
single: $(SINGLE_TARGET)

# Build only MPI version  
mpi: $(MPI_TARGET)

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -f $(SINGLE_TARGET) $(MPI_TARGET) *.o
	@echo "✅ Clean complete!"

# Test single computer version with different matrix sizes
test: $(SINGLE_TARGET)
	@echo "🧪 Testing single computer version..."
	@echo "Small matrix (100x100, K=5):"
	echo "100 100 5" | ./$(SINGLE_TARGET)

test-small: $(SINGLE_TARGET)
	@echo "🧪 Small matrix test (50x50, K=3):"
	echo "50 50 3" | ./$(SINGLE_TARGET)

test-medium: $(SINGLE_TARGET)
	@echo "🧪 Medium matrix test (200x200, K=8):"
	echo "200 200 8" | ./$(SINGLE_TARGET)

test-large: $(SINGLE_TARGET)
	@echo "🧪 Large matrix test (500x500, K=15):"
	echo "500 500 15" | ./$(SINGLE_TARGET)

# Performance measurements for your results.doc
perf-measurements: $(SINGLE_TARGET)
	@echo "📊 Performance Measurements for results.doc"
	@echo "==========================================="
	@echo ""
	@echo "Test 1: Small Matrix (100x100, K=5)"
	@echo "100 100 5" | time ./$(SINGLE_TARGET)
	@echo ""
	@echo "Test 2: Medium Matrix (300x300, K=10)"
	@echo "300 300 10" | time ./$(SINGLE_TARGET)
	@echo ""
	@echo "Test 3: Large Matrix (500x500, K=15)"
	@echo "500 500 15" | time ./$(SINGLE_TARGET)
	@echo ""
	@echo "📝 Copy these times to your results.doc file"

# Run multiple times for average (like the assignment asks)
perf-average: $(SINGLE_TARGET)
	@echo "📊 Running 3 times for average (as required by assignment)"
	@echo "========================================================="
	@echo ""
	@echo "Small Matrix (100x100, K=5) - Run 1:"
	@echo "100 100 5" | time ./$(SINGLE_TARGET) 2>&1 | grep real
	@echo "Small Matrix (100x100, K=5) - Run 2:"
	@echo "100 100 5" | time ./$(SINGLE_TARGET) 2>&1 | grep real
	@echo "Small Matrix (100x100, K=5) - Run 3:"
	@echo "100 100 5" | time ./$(SINGLE_TARGET) 2>&1 | grep real

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(SINGLE_TARGET)
	@echo "🐛 Debug version built successfully!"

# Check if all dependencies are available
check-deps:
	@echo "🔍 Checking dependencies..."
	@echo -n "GCC-14: "
	@which $(CC) > /dev/null && echo "✅ Found" || echo "❌ Not found"
	@echo -n "OpenMP: "
	@test -f /opt/homebrew/opt/libomp/include/omp.h && echo "✅ Found" || echo "❌ Not found"
	@echo -n "MPI: "
	@which $(MPICC) > /dev/null && echo "✅ Found" || echo "❌ Not found"

# Show your system info (useful for the assignment report)
system-info:
	@echo "=== System Information for Assignment ==="
	@echo "OS: $(shell uname -s)"
	@echo "Kernel: $(shell uname -r)"
	@echo "Architecture: $(shell uname -m)"
	@echo "CPU cores: $(shell sysctl -n hw.ncpu)"
	@echo "Physical CPU cores: $(shell sysctl -n hw.physicalcpu)"
	@echo "Memory: $(shell sysctl -n hw.memsize | awk '{print int($$1/1024/1024/1024) "GB"}')"
	@echo "GCC version: $(shell $(CC) --version | head -1)"
	@echo "OpenMP version: $(shell $(CC) -fopenmp -dM -E - < /dev/null | grep _OPENMP | awk '{print $$3}')"

# Create the required README.TXT file
create-readme:
	@echo "📝 Creating README.TXT file..."
	@echo "PARALLEL MATRIX SUBMATRIX FINDER - macOS VERSION" > README.TXT
	@echo "==============================================" >> README.TXT
	@echo "" >> README.TXT
	@echo "COMPILATION:" >> README.TXT
	@echo "make single          - Build single computer version" >> README.TXT
	@echo "make                 - Same as 'make single'" >> README.TXT
	@echo "" >> README.TXT
	@echo "RUNNING:" >> README.TXT
	@echo "./matrix_solver      - Run the program" >> README.TXT
	@echo "                      Follow prompts to enter N, M, K values" >> README.TXT
	@echo "" >> README.TXT
	@echo "TESTING:" >> README.TXT
	@echo "make test           - Quick test with 100x100 matrix" >> README.TXT
	@echo "make test-small     - Test with 50x50 matrix" >> README.TXT
	@echo "make test-medium    - Test with 200x200 matrix" >> README.TXT
	@echo "make test-large     - Test with 500x500 matrix" >> README.TXT
	@echo "" >> README.TXT
	@echo "PERFORMANCE MEASUREMENT:" >> README.TXT
	@echo "make perf-measurements  - Run all required tests for results.doc" >> README.TXT
	@echo "make perf-average      - Run multiple times for average calculation" >> README.TXT
	@echo "" >> README.TXT
	@echo "SYSTEM INFO:" >> README.TXT
	@echo "make system-info    - Show system specifications" >> README.TXT
	@echo "make check-deps     - Verify all dependencies are installed" >> README.TXT
	@echo "" >> README.TXT
	@echo "CLEANING:" >> README.TXT
	@echo "make clean          - Remove compiled files" >> README.TXT
	@echo "" >> README.TXT
	@echo "REQUIREMENTS:" >> README.TXT
	@echo "- macOS with Homebrew" >> README.TXT
	@echo "- GCC 14 with OpenMP support" >> README.TXT
	@echo "- Libraries: libomp" >> README.TXT
	@echo "" >> README.TXT
	@echo "AUTHOR: [Your Name and Student ID]" >> README.TXT
	@echo "✅ README.TXT created successfully!"

# Show help
help:
	@echo "Available targets for your assignment:"
	@echo "  all/single       - Build single computer version (default)"
	@echo "  test             - Quick test"
	@echo "  test-small       - Test with small matrix"
	@echo "  test-medium      - Test with medium matrix" 
	@echo "  test-large       - Test with large matrix"
	@echo "  perf-measurements - Run performance tests for results.doc"
	@echo "  perf-average     - Run multiple times for average"
	@echo "  clean            - Remove build files"
	@echo "  debug            - Build debug version"
	@echo "  check-deps       - Check if dependencies are installed"
	@echo "  system-info      - Show system information"
	@echo "  create-readme    - Create README.TXT file"
	@echo "  help             - Show this help message"

.PHONY: all single mpi clean test test-small test-medium test-large perf-measurements perf-average debug check-deps system-info create-readme help