#pragma once

#include "Types.h"
#include <vector>
#include <utility>

namespace Columbus
{

	template <typename T>
	class TStableSparseArray
	{
	private:
		struct SparseEntry
		{
			u32 generation;  // Handle generation
			i32 dense_index; // Index into the dense array (-1 if invalid)
		};
	public:
		struct Handle
		{
			u32 index = (u32)-1;     // Sparse array index
			u32 generation = (u32)-1;
		};

	public:
		Handle Add(const T& value)
		{
			u32 sparse_idx;
			if (!free_indices.empty())
			{
				sparse_idx = free_indices.back();
				free_indices.pop_back();
			}
			else
			{
				sparse_idx = (u32)sparse.size();
				sparse.push_back({ 0, -1 });
			}

			// Update the sparse entry
			sparse[sparse_idx].generation++;
			sparse[sparse_idx].dense_index = (u32)dense.size();

			// Add to the dense array and map
			dense.push_back(std::move(value));
			dense_to_sparse.push_back(sparse_idx);

			return { sparse_idx, sparse[sparse_idx].generation };
		}

		void Remove(Handle handle)
		{
			if (!IsValid(handle))
				return;

			const uint32_t dense_idx = sparse[handle.index].dense_index;

			// Swap with the last element if not already the last
			if (dense_idx != dense.size() - 1)
			{
				std::swap(dense[dense_idx], dense.back());
				std::swap(dense_to_sparse[dense_idx], dense_to_sparse.back());

				// Update the swapped element's sparse entry
				const uint32_t swapped_sparse_idx = dense_to_sparse[dense_idx];
				sparse[swapped_sparse_idx].dense_index = dense_idx;
			}

			// Remove the last element
			dense.pop_back();
			dense_to_sparse.pop_back();

			// Invalidate the handle and recycle its sparse index
			sparse[handle.index].dense_index = -1;
			free_indices.push_back(handle.index);
		}

		T* Get(Handle handle)
		{
			if (!IsValid(handle))
				return nullptr;

			return &dense[sparse[handle.index].dense_index];
		}

		// return the first occurence of a value and return its handle
		Handle Find(const T& value)
		{
			// O(n) because can't make a cache on struct values
			for (size_t i = 0; i < dense.size(); ++i)
			{
				if (dense[i] == value)
				{
					u32 sparse_idx = dense_to_sparse[i];
					return { sparse_idx, sparse[sparse_idx].generation };
				}
			}

			return {}; // invalid
		}

		// remove the first occurance of a value, return true if found and removed
		bool Remove(const T& value)
		{
			Handle handle = Find(value);
			if (IsValid(handle))
			{
				Remove(handle);
				return true;
			}

			return false;
		}

		// Direct access to the contiguous data (e.g., for GPU upload)
		const T* Data() const { return dense.data(); }
		size_t   Size() const { return dense.size(); }

		T& operator[](size_t id) { return dense[id]; }

		std::vector<T>::iterator begin() { return dense.begin(); }
		std::vector<T>::iterator end()   { return dense.end(); }

		bool IsValid(Handle handle) const
		{
			return handle.index < sparse.size() &&
				sparse[handle.index].generation == handle.generation &&
				sparse[handle.index].dense_index != -1;
		}

	private:
		std::vector<SparseEntry> sparse;
		std::vector<T> dense;
		std::vector<u32> dense_to_sparse; // Maps dense indices -> sparse indices
		std::vector<u32> free_indices;
	};

}
