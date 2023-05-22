#include "helpers.h"

void printMatrix(const IntMatrix &matrix)  {
  for (const auto& row : matrix) {
    for (const auto& el : row) {
      std::cout << std::setw(8) << el << " ";
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
}

size_t getMaxElementInColumnRowIdx(const IntMatrix& matrix, size_t colIdx) {
  size_t maxElementRowIdx = 0;

  for (size_t i = 1; i < matrix.size(); i++) {
    if (matrix[i][colIdx] > matrix[maxElementRowIdx][colIdx]) {
      maxElementRowIdx = i;
    }
  }

  return maxElementRowIdx;
}

void swapMatrixMaxElementsWithDiagonal(IntMatrix& matrix, size_t colBeginIdx, size_t colEndIdx) {
  for (size_t colIdx = colBeginIdx; colIdx < colEndIdx; colIdx++) {
    size_t maxElementRowIdx = getMaxElementInColumnRowIdx(matrix, colIdx);

    auto tmp = matrix[colIdx][colIdx];
    matrix[colIdx][colIdx] = matrix[maxElementRowIdx][colIdx];
    matrix[maxElementRowIdx][colIdx] = tmp;
  }
}

uint32_t parallelSwapMatrixMaxElementsWithDiagonal(IntMatrix &matrix, size_t nThreads) {
  Timer t;
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

  auto duration = t.elapsed<Timer::millisecond_t>();
  std::cout << "Duration = " << duration << std::endl;
  return duration;
}
