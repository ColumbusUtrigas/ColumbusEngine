#pragma once

#include <memory>

template <typename T>
using UPtr = std::unique_ptr<T>;

template <typename T>
using SPtr = std::shared_ptr<T>;

template <typename T>
using WPtr = std::weak_ptr<T>;

// TODO: SRef analogous to Unreal's TSharedRef

namespace Columbus
{

}


