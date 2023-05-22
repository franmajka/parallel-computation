#include <iostream>
#include <vector>
#include <random>
#include <thread>
#include <future>
#include <iterator>
#include <chrono>
#include <shared_mutex>
#include <atomic>
#include <fstream>

#include "Timer.hpp"

using namespace std::chrono_literals;

std::vector<int> createRandomArray(size_t size) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(-30, 30);

  std::vector<int> arr(size);

  for (size_t i = 0; i < size; i++) {
    arr[i] = distr(gen);
  }

  return arr;
}

void printArray(const std::vector<int>& arr) {
  for (const auto& el : arr) {
    std::cout << el << " ";
  }

  std::cout << std::endl;
}

std::pair<size_t, int> countGreaterThanAndMaxAmongThem(
  std::vector<int>::const_iterator begin,
  std::vector<int>::const_iterator end,
  int greaterThan
) {
  size_t count = 0;
  int maxEl = INT32_MIN;

  for (auto it = begin; it != end; ++it) {
    if (*it > greaterThan) {
      count++;
      if (*it > maxEl) {
        maxEl = *it;
      }
    }
  }

  return { count, maxEl };
}

std::pair<size_t, int> countGreaterThanAndMaxAmongThemInParallel(
  const std::vector<int>& arr,
  int greaterThan
) {
  size_t nThreads = std::thread::hardware_concurrency();
  size_t nElementsPerThread = arr.size() / nThreads;
  size_t nRestElements = arr.size() % nThreads;

  std::vector<std::future<std::pair<size_t, int>>> futures;

  auto begin = arr.begin();
  while (begin != arr.end()) {
    auto end = begin + nElementsPerThread;
    if (nRestElements) {
      nRestElements--;
      ++end;
    }

    futures.emplace_back(std::async(&countGreaterThanAndMaxAmongThem, begin, end, greaterThan));

    begin = end;
  }

  size_t count = 0;
  int maxEl = INT32_MIN;

  for (auto& res : futures) {
    auto [currCount, currMaxEl] = res.get();

    count += currCount;
    if (currMaxEl > maxEl) {
      maxEl = currMaxEl;
    }
  }

  return { count, maxEl };
}

size_t count = 0;
std::mutex countMtx;

int maxEl = INT32_MIN;
std::mutex maxElMtx;

void countGreaterThanAndMaxAmongThemBlocking(
  std::vector<int>::const_iterator begin,
  std::vector<int>::const_iterator end,
  int greaterThan
) {
  for (auto it = begin; it != end; ++it) {
    if (*it > greaterThan) {
      {
        std::unique_lock lock(countMtx);
        count++;
      }

      std::unique_lock lock(maxElMtx);
      if (*it > maxEl) {
        maxEl = *it;
      }
    }
  }
}

std::pair<size_t, int> countGreaterThanAndMaxAmongThemInParallelBlocking(
  const std::vector<int>& arr,
  int greaterThan
) {
  count = 0;
  maxEl = INT32_MIN;

  size_t nThreads = std::thread::hardware_concurrency();
  size_t nElementsPerThread = arr.size() / nThreads;
  size_t nRestElements = arr.size() % nThreads;

  std::vector<std::thread> threads;

  auto begin = arr.begin();
  while (begin != arr.end()) {
    auto end = begin + nElementsPerThread;
    if (nRestElements) {
      nRestElements--;
      ++end;
    }

    threads.emplace_back(std::thread(&countGreaterThanAndMaxAmongThemBlocking, begin, end, greaterThan));

    begin = end;
  }

  for (auto& thread : threads) {
    thread.join();
  }

  return { count, maxEl };
}

std::atomic<size_t> countAtomic = 0;
std::atomic<int> maxElAtomic = INT32_MIN;

void countGreaterThanAndMaxAmongThemLockFree(
  std::vector<int>::const_iterator begin,
  std::vector<int>::const_iterator end,
  int greaterThan
) {
  for (auto it = begin; it != end; ++it) {
    if (*it > greaterThan) {
      countAtomic++;

      int maxEl = maxElAtomic.load();
      while (true) {
        if (*it > maxEl) {
          if (maxElAtomic.compare_exchange_strong(maxEl, *it)) {
            break;
          }
        } else {
          break;
        }
      }
    }
  }
}

std::pair<size_t, int> countGreaterThanAndMaxAmongThemInParallelLockFree(
  const std::vector<int>& arr,
  int greaterThan
) {
  countAtomic.store(0);
  maxElAtomic.store(INT32_MIN);

  size_t nThreads = std::thread::hardware_concurrency();
  size_t nElementsPerThread = arr.size() / nThreads;
  size_t nRestElements = arr.size() % nThreads;

  std::vector<std::thread> threads;

  auto begin = arr.begin();
  while (begin != arr.end()) {
    auto end = begin + nElementsPerThread;
    if (nRestElements) {
      nRestElements--;
      ++end;
    }

    threads.emplace_back(std::thread(&countGreaterThanAndMaxAmongThemLockFree, begin, end, greaterThan));

    begin = end;
  }

  for (auto& thread : threads) {
    thread.join();
  }

  return { countAtomic.load(), maxElAtomic.load() };
}


int main() {
  int kGreaterThan = 20;
  std::vector<size_t> arraySizes = { 100, 1'000, 5'000, 10'000, 100'000, 1'000'000, 100'000'000, 200'000'000, 300'000'000 };
  std::ofstream file("stats.csv");

  if (!file) {
    std::cout << "Error opening stats!" << std::endl;
    return 1;
  }

  file << "name,arraySize,duration\n";

  for (auto arraySize : arraySizes) {
    std::cout << "New iteration on array with size " << arraySize << std::endl;
    auto arr = createRandomArray(arraySize);

    Timer t;
    countGreaterThanAndMaxAmongThem(arr.begin(), arr.end(), kGreaterThan);
    file << "Single thread," << arraySize << ',' << t.elapsed<Timer::millisecond_t>() << '\n';

    t.reset();
    countGreaterThanAndMaxAmongThemInParallel(arr, kGreaterThan);
    file << "Multithread non blocking," << arraySize << ',' << t.elapsed<Timer::millisecond_t>() << '\n';

    t.reset();
    countGreaterThanAndMaxAmongThemInParallelBlocking(arr, kGreaterThan);
    file << "Multithread blocking," << arraySize << ',' << t.elapsed<Timer::millisecond_t>() << '\n';

    t.reset();
    countGreaterThanAndMaxAmongThemInParallelLockFree(arr, kGreaterThan);
    file << "Multithread atomic," << arraySize << ',' << t.elapsed<Timer::millisecond_t>() << '\n';
  }

  file.close();

  return 0;
}
