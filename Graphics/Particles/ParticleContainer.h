#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <utility>

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
		public:
			DataWrapper() {}
			DataWrapper(const DataWrapper&) = delete;
			DataWrapper(DataWrapper&& Base) noexcept { *this = std::move(Base); }
			DataWrapper& operator=(const DataWrapper&) = delete;
			DataWrapper& operator=(DataWrapper&& Base) noexcept { std::swap(Data, Base.Data); return *this; }
			T& operator[](size_t Index) const { return Data[Index]; }

			void Allocate(size_t Size)
			{
				Allocation = (T*)realloc((void*)Allocation, Size * sizeof(T) + 16);
				Data = Allocation;
				Data = (T*)(((uintptr_t)Data + 15) & ~((uintptr_t)15));
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
			std::swap(Ages[A], Ages[B]);
			std::swap(Lifetimes[A], Lifetimes[B]);
			std::swap(Percents[A], Percents[B]);
			std::swap(Rotations[A], Rotations[B]);
			std::swap(RotationVelocities[A], RotationVelocities[B]);
			std::swap(Distances[A], Distances[B]);
			std::swap(Frames[A], Frames[B]);
			std::swap(Positions[A], Positions[B]);
			std::swap(Velocities[A], Velocities[B]);
			std::swap(Sizes[A], Sizes[B]);
			std::swap(Colors[A], Colors[B]);
		}
	};

}


