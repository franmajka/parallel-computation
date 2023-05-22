#pragma once

#include <iostream>
#include <vector>
#include <iomanip>
#include <thread>

#include "Timer.hpp"

template<typename T = int>
using Matrix = std::vector<std::vector<T>>;
using IntMatrix = Matrix<int>;

void printMatrix(const IntMatrix& matrix);

size_t getMaxElementInColumnRowIdx(const IntMatrix& matrix, size_t colIdx);
void swapMatrixMaxElementsWithDiagonal(IntMatrix& matrix, size_t colBeginIdx, size_t colEndIdx);

uint32_t parallelSwapMatrixMaxElementsWithDiagonal(IntMatrix& matrix, size_t nThreads);
