#pragma once

#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	struct ParticleContainer
	{
		template <typename T>
		struct DataWrapper
		{
		private:
			T* Allocation = nullptr;
			T* Data = nullptr;

			DataWrapper& SwapData(T*& New)
			{
				T* Tmp = Data;
				Data = New;
				New = Tmp;
				return *this;
			}
		public:
			DataWrapper() {}
			DataWrapper(const DataWrapper&) = delete;
			DataWrapper(DataWrapper&& Base) noexcept { SwapData(Base.Data); }
			DataWrapper& operator=(const DataWrapper&) = delete;
			DataWrapper& operator=(DataWrapper&& Base) noexcept { return SwapData(Base.Data); }
			T& operator[](size_t Index) const { return Data[Index]; }

			void Allocate(size_t Size)
			{
				Allocation = (T*)realloc((void*)Allocation, Size * sizeof(T) + 16);
				Data = Allocation;
				Data = (T*)(((uintptr_t)Data + 15) & ~((uintptr_t)15));
			}

			void Swap(size_t A, size_t B)
			{
				T Tmp = Data[A];
				Data[A] = Data[B];
				Data[B] = Tmp;
			}

			~DataWrapper() { if (Allocation != nullptr) free(Allocation); }
		};

		size_t Count = 0;

		DataWrapper<float> Ages;
		DataWrapper<float> Lifetimes;
		DataWrapper<float> Percents;
		DataWrapper<float> Rotations;
		DataWrapper<float> RotationVelocities;
		DataWrapper<float> Distances;
		DataWrapper<uint32> Frames;
		DataWrapper<Vector3> Positions;
		DataWrapper<Vector3> Velocities;
		DataWrapper<Vector3> Sizes;
		DataWrapper<Vector4> Colors;

		void Allocate(size_t MaxSize)
		{
			Ages.Allocate(MaxSize);
			Lifetimes.Allocate(MaxSize);
			Percents.Allocate(MaxSize);
			Rotations.Allocate(MaxSize);
			RotationVelocities.Allocate(MaxSize);
			Distances.Allocate(MaxSize);
			Frames.Allocate(MaxSize);
			Positions.Allocate(MaxSize);
			Velocities.Allocate(MaxSize);
			Sizes.Allocate(MaxSize);
			Colors.Allocate(MaxSize);
		}

		void Move(size_t To, size_t From)
		{
			Ages[To] = Ages[From];
			Lifetimes[To] = Lifetimes[From];
			Percents[To] = Percents[From];
			Rotations[To] = Rotations[From];
			RotationVelocities[To] = RotationVelocities[From];
			Distances[To] = Distances[From];
			Frames[To] = Frames[From];
			Positions[To] = Positions[From];
			Velocities[To] = Velocities[From];
			Sizes[To] = Sizes[From];
			Colors[To] = Colors[From];
		}

		void Swap(size_t A, size_t B)
		{
			Ages.Swap(A, B);
			Lifetimes.Swap(A, B);
			Percents.Swap(A, B);
			Rotations.Swap(A, B);
			RotationVelocities.Swap(A, B);
			Distances.Swap(A, B);
			Frames.Swap(A, B);
			Positions.Swap(A, B);
			Velocities.Swap(A, B);
			Sizes.Swap(A, B);
			Colors.Swap(A, B);
		}
	};

}


