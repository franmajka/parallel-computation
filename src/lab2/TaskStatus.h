#pragma once

#include <optional>

enum Status {
  Pending,
  Processing,
  Fulfilled,
  Rejected,
};

template<typename Ret>
struct TaskStatus {
  Status status;
  std::optional<Ret> res = std::nullopt;
  std::optional<std::string> errorMsg = std::nullopt;
};
