#pragma once
#include <ctime>
#include <random>

class Matrix
{
public:

	int* arr;
	int rows;
	int columns;

	Matrix() {
		arr = NULL;
		rows = 0;
		columns = 0;
	}

	Matrix(int rows, int columns)
	{
		srand(time(0));

		this->rows = rows;
		this->columns = columns;
		arr = new int[rows * columns];

		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < columns; ++j)
			{
				arr[i * columns + j] = rand() % 9 + 1;
			}
		}
	}

	void transpose()
	{
		int* result = new int[rows * columns];

		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < columns; ++j)
			{
				result[i * columns + j] = 0;
			}
		}

		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < columns; ++j)
			{
				result[j * rows + i] = arr[i * columns + j];
			}
		}

		delete[] arr;

		arr = result;

		int tempRows = rows;
		rows = columns;
		columns = tempRows;
	}

	Matrix operator*(const Matrix& matrix) {

		if (columns == matrix.rows) {

			Matrix result = Matrix(rows, matrix.columns);

			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < matrix.columns; ++j) {
					result.arr[i * matrix.columns + j] = 0;
					for (int k = 0; k < columns; ++k)
						result.arr[i * matrix.columns + j] += arr[i * columns + k] * matrix.arr[j * matrix.columns + k];
				}

			return result;
		}

		return Matrix(0, 0);
	}

};