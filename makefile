CC=gcc
CFLAGS=-fopenmp
LDFLAGS=-lmpi -lm
TARGET=hw2
SRC=matrix_solver_1.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

run:
	mpiexec -np 2 ./$(TARGET)

test: $(TARGET)
	echo "1000 1000\n10" | mpiexec -np 2 ./$(TARGET)

clean:
	rm -f $(TARGET)