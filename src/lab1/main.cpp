#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <thread>
#include <fstream>

#include "Timer.hpp"

template<typename T = int>
using Matrix = std::vector<std::vector<T>>;
using IntMatrix = Matrix<int>;

IntMatrix createRandomSquareMatrix(size_t size) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(-size * size, size * size);

  IntMatrix matrix;

  for (size_t i = 0; i < size; i++) {
    decltype(matrix)::value_type row;

    for (size_t j = 0; j < size; j++) {
      row.push_back(distr(gen));
    }

    matrix.push_back(row);
  }

  return matrix;
}

void printMatrix(const IntMatrix& matrix) {
  for (const auto& row : matrix) {
    for (const auto& el : row) {
      std::cout << std::setw(8) << el << " ";
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
}

template<typename T = int>
size_t getMaxElementInColumnRowIdx(const Matrix<T>& matrix, size_t colIdx) {
  size_t maxElementRowIdx = 0;

  for (size_t i = 1; i < matrix.size(); i++) {
    if (matrix[i][colIdx] > matrix[maxElementRowIdx][colIdx]) {
      maxElementRowIdx = i;
    }
  }

  return maxElementRowIdx;
}

template<typename T = int>
void swapMatrixMaxElementsWithDiagonal(Matrix<T>& matrix, size_t colBeginIdx, size_t colEndIdx) {
  for (size_t colIdx = colBeginIdx; colIdx < colEndIdx; colIdx++) {
    size_t maxElementRowIdx = getMaxElementInColumnRowIdx(matrix, colIdx);

    auto tmp = matrix[colIdx][colIdx];
    matrix[colIdx][colIdx] = matrix[maxElementRowIdx][colIdx];
    matrix[maxElementRowIdx][colIdx] = tmp;
  }
}

template<typename T = int>
void swapMatrixMaxElementsWithDiagonal(Matrix<T>& matrix) {
  return swapMatrixMaxElementsWithDiagonal(matrix, 0, matrix.size());
}

template<typename T = int>
void parallelSwapMatrixMaxElementsWithDiagonal(Matrix<T>& matrix, size_t nThreads) {
  std::vector<std::thread> threads;

  size_t nColsPerThread = matrix.size() / nThreads;
  size_t nRestCols = matrix.size() % nThreads;
  size_t nRestColsLeft = nRestCols;

  for (size_t i = 0; i < nThreads; i++) {
    size_t colBeginIdx = i * nColsPerThread;
    if (i && nRestCols) {
      colBeginIdx += std::min(i, nRestCols);
    }

    size_t colEndIdx = colBeginIdx + nColsPerThread;
    if (nRestColsLeft) {
      colEndIdx++;
      nRestColsLeft--;
    }

    threads.push_back(std::thread(
      [&matrix, colBeginIdx, colEndIdx]() {
        swapMatrixMaxElementsWithDiagonal(matrix, colBeginIdx, colEndIdx);
      }
    ));
  }

  for (auto& thread : threads) {
    thread.join();
  }
}

int main() {
  std::vector<size_t> matrixSizes = { 10, 100, 1'000, 5'000, 10'000, 15'000, 20'000, 25'000, /* 100'000, 1'000'000 */ };

  size_t nPhysicalCores = 4;
  size_t nLogicalCores = std::thread::hardware_concurrency();
  std::vector<size_t> threadNumbers = {
    nPhysicalCores / 2,
    nPhysicalCores,
    nLogicalCores,
    2 * nLogicalCores,
    4 * nLogicalCores,
    8 * nLogicalCores,
    16 * nLogicalCores
  };

  std::ofstream file("stats.csv");

  if (!file) {
    std::cout << "Error opening stats!" << std::endl;
    return 1;
  }

  file << "matrixSize,nThreads,duration\n";

  for (const auto& matrixSize : matrixSizes) {
    auto matrix = createRandomSquareMatrix(matrixSize);
    auto matrixCopy = matrix;

    Timer t;
    swapMatrixMaxElementsWithDiagonal(matrixCopy);
    file << matrixSize << ',' << 1 << ',' << t.elapsed<Timer::millisecond_t>() << '\n';

    for (const auto& nThreads : threadNumbers) {
      matrixCopy = matrix;

      t.reset();
      parallelSwapMatrixMaxElementsWithDiagonal(matrixCopy, nThreads);
      file << matrixSize << ',' << nThreads << ',' << t.elapsed<Timer::millisecond_t>() << '\n';
    }
  }

  file.close();

  return 0;
}
