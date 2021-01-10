#pragma once

#include <functional>

template <typename...  Args>
using Fun = std::function<Args...>;
