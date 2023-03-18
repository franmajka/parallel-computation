#include <iostream>
#include <random>
#include <chrono>

#include "TaskStatus.h"
#include "ThreadPool.h"

using namespace std::chrono_literals;

unsigned randomInRange(unsigned min, unsigned max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(min, max);

  return distr(gen);
}

int task() {
  auto sleepSeconds = randomInRange(1, 10);

  std::printf(
    "Task will be completed in %d seconds\n",
    sleepSeconds
  );
  std::this_thread::sleep_for(std::chrono::seconds(sleepSeconds));

  return std::hash<std::thread::id>{}(std::this_thread::get_id()) * sleepSeconds % 100;
}

void callback(TaskStatus<int>&& data) {
  switch (data.status) {
    case Status::Pending: {
      std::printf("Task got into thread pool\n");
      break;
    }
    case Status::Processing: {
      std::printf("Task is processed by a thread[%lu]\n", std::hash<std::thread::id>{}(std::this_thread::get_id()));
      break;
    }
    case Status::Fulfilled: {
      std::printf("Task is completed. Return: %d\n", *data.res);
      break;
    }
    case Status::Rejected: {
      std::printf("Task encountered an error. Message: %s\n", data.errorMsg->c_str());
      break;
    }
  }
}

void taskCreator(ThreadPool<int>& threadPool, uint8_t nTasks) {
  while (nTasks--) {
    threadPool.addTask(&task, &callback);
  }
}

int main() {
  ThreadPool<int> threadPool(4);

  std::vector<std::thread> threads;
  uint8_t nTaskCreators = randomInRange(3, 6);

  while(nTaskCreators--) {
    threads.push_back(std::thread(&taskCreator, std::ref(threadPool), randomInRange(3, 6)));
  }

  for (auto& thread : threads) {
    thread.join();
  }

  return 0;
}
