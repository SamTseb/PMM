// Arch_Lab_1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include<mpi.h>
#include<iostream>
#include <ctime>
#include <random>
#include "Matrix.h"

#define DEBUG 0
#define OUT_IN_EXEL 1
using namespace std;

int ProcNum; // Number of available processes
int ProcRank; // Rank of current process

std::ostream& operator<<(std::ostream& out, const Matrix& m) {

	for (size_t i = 0; i < m.rows; ++i) {
		for (size_t j = 0; j < m.columns; ++j)
			out << m.arr[i * m.columns + j] << " ";

		out << endl;
	}

	out << endl;
	return out;
};

// Function for simple definition of matrix and vector elements
void DummyDataInitialization(Matrix* pMatrix, Matrix* pVector, size_t size) {
	srand(time(0));

	for (size_t i = 0; i < size; i++) {
		pVector->arr[i] = rand() % 9 + 1;
		for (size_t j = 0; j < size; j++)
			pMatrix->arr[i * size + j] = rand() % 9 + 1;
	}

#if DEBUG
	cout << '\n';
	cout << *pMatrix;
	cout << *pVector;
#endif // 0
}

// Function for memory allocation and initialization of objects’ elements
void ProcessInitialization( Matrix*& pMatrix, Matrix*& pVector, Matrix*& pResult,
							int*& pProcRows, int*& pProcResult,  size_t& size, size_t& RowNum, char* argv[]) {
	size_t RestRows; // Number of rows, that haven’t been distributed yet

	if (ProcRank == 0) {
		/*do {
			cout << "\nEnter size of the initial objects: ";
			cin >> size;
			if (size < ProcNum) {
				cout << "Size of the objects must be greater than number of processes!\n";
			}
		} while (size < ProcNum);*/

		size = *argv[1] -'0';
	}
	MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Determine the number of matrix rows stored on each process
	RestRows = size;
	for (size_t i = 0; i < ProcRank; i++)
		RestRows = RestRows - RestRows / (ProcNum - i);
	RowNum = RestRows / (ProcNum - ProcRank);

	// Memory allocation
	pVector->arr = new int[size];
	pVector->columns = 1;
	pVector->rows = size;

	pResult->arr = new int[size];
	pResult->columns = 1;
	pResult->rows = size;

	pProcRows = new int[RowNum * size];
	pProcResult = new int[RowNum];
	// Obtain the values of initial objects’ elements
	if (ProcRank == 0) {
		// Initial matrix exists only on the pivot process
		pMatrix->arr = new int[size * size];
		pMatrix->columns = pMatrix->rows = size;
		// Values of elements are defined only on the pivot process
		DummyDataInitialization(pMatrix, pVector, size);
	}
}

// Function for computational process termination
void ProcessTermination(int* pProcRows, int* pProcResult) {
	/*if (ProcRank == 0)
		delete[] pMatrix;
	delete[] pVector;
	delete[] pResult;*/
	delete[] pProcRows;
	delete[] pProcResult;
}

// Function for distribution of the initial objects between the processes
void DataDistribution(int* pMatrix, int* pProcRows, int* pVector, size_t size, size_t RowNum) {
	int* pSendNum; // the number of elements sent to the process
	int* pSendInd; // the index of the first data element sent to the process
	size_t RestRows = size; // Number of rows, that haven’t been distributed yet

	MPI_Bcast(pVector, size, MPI_INT, 0, MPI_COMM_WORLD);

	// Alloc memory for temporary objects
	pSendInd = new int[ProcNum];
	pSendNum = new int[ProcNum];

	// Determine the disposition of the matrix rows for current process
	RowNum = (size / ProcNum);
	pSendNum[0] = RowNum * size;
	pSendInd[0] = 0;
	for (size_t i = 1; i < ProcNum; i++) {
		RestRows -= RowNum;
		RowNum = RestRows / (ProcNum - i);
		pSendNum[i] = RowNum * size;
		pSendInd[i] = pSendInd[i - 1] + pSendNum[i - 1];
	}
	// Scatter the rows
	MPI_Scatterv(pMatrix, pSendNum, pSendInd, MPI_INT, pProcRows, pSendNum[ProcRank], MPI_INT, 0, MPI_COMM_WORLD);
	// Free the memory
	delete[] pSendNum;
	delete[] pSendInd;
}

// Process rows and vector multiplication
void ParallelResultCalculation(int* pProcRows, int* pVector, int* pProcResult, size_t size, size_t RowNum) {
	for (size_t i = 0; i < RowNum; i++) {
		pProcResult[i] = 0;
		for (size_t j = 0; j < size; j++) {
			pProcResult[i] += pProcRows[i * size + j] * pVector[j];
		}
	}
}

// Result vector replication
void ResultReplication(int* pProcResult, int* pResult, size_t size, size_t RowNum) {
	int* pReceiveNum; // Number of elements, that current process sends
	int* pReceiveInd; // Index of the first element from current processin result vector
	size_t RestRows = size; // Number of rows, that haven’t been distributed yet

	// Alloc memory for temporary objects
	pReceiveNum = new int[ProcNum];
	pReceiveInd = new int[ProcNum];

	// Detrmine the disposition of the result vector block of current
	// processor
	pReceiveInd[0] = 0;
	pReceiveNum[0] = size / ProcNum;

	for (size_t i = 1; i < ProcNum; i++) {
		RestRows -= pReceiveNum[i - 1];
		pReceiveNum[i] = RestRows / (ProcNum - i);
		pReceiveInd[i] = pReceiveInd[i - 1] + pReceiveNum[i - 1];
	}
	// Gather the whole result vector on every processor
	MPI_Allgatherv(pProcResult, pReceiveNum[ProcRank], MPI_INT, pResult, pReceiveNum, pReceiveInd, MPI_INT, MPI_COMM_WORLD);
	// Free the memory
	delete[] pReceiveNum;
	delete[] pReceiveInd;
}

void MatrixMultiplicationMPI(Matrix* matrix, Matrix* vector, Matrix* result, int argc, char** argv) {
	size_t size = 0; // Sizes of initial matrix and vector
	int* pProcRows = NULL; // Stripe of the matrix on current process
	int* pProcResult = NULL; // Block of result vector on current process
	size_t RowNum = 0; // Number of rows in matrix stripe 
	double start, time; // For get the time of the computation

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	// Memory allocation and data initialization
	ProcessInitialization(matrix, vector, result, pProcRows, pProcResult, size, RowNum, argv);

	int* pMatrix = matrix->arr; // The first argument - initial matrix
	int* pVector = vector->arr; // The second argument - initial vector
	//int* pResult = result->arr; // Result vector for matrix-vector multiplication

	// Start the time of the computation
	start = MPI_Wtime();

	// Distributing the initial objects between the processes
	DataDistribution(pMatrix, pProcRows, pVector, size, RowNum);

	// Process rows and vector multiplication
	ParallelResultCalculation(pProcRows, pVector, pProcResult, size, RowNum);

	// Result replication
	ResultReplication(pProcResult, result->arr, size, RowNum);

	// Stop, getting time of the computation
	time = MPI_Wtime() - start;

	ProcessTermination(pProcRows, pProcResult);
	MPI_Finalize();

	if (ProcRank == 0) {
#if DEBUG
		cout << *result;
#endif // 0

#if OUT_IN_EXEL
		cout << time;
#else
		cout << "\nTime of computation: " << time;
#endif // 0

	}
}
 

int main(int argc, char* argv[])
{
	Matrix* A = new Matrix();
	Matrix* B = new Matrix();
	Matrix* C = new Matrix();

	MatrixMultiplicationMPI(A,B,C, argc, argv);
	return 0;
}