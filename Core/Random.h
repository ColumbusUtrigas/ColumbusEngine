#pragma once

#include <random>

namespace Columbus
{
	/// Helper templates to decide on the <random>-provided distribution to use
	template<class T, bool IsIntegral, bool IsFloat>
	struct RandomDistribution;

	template<class T>
	struct RandomDistribution<T, true, false>
	{
		using DistrType = std::uniform_int_distribution<T>;
	};

	template<class T>
	struct RandomDistribution<T, false, true>
	{
		using DistrType = std::uniform_real_distribution<T>;
	};

	template<class T>
	using RandomDistributionType = typename RandomDistribution<T, std::is_integral_v<T>, std::is_floating_point_v<T>>::DistrType;

	class Random
	{
	public:

		/// Generates a random value in range Min...Max
		template <typename Type>
		inline static Type Range(const Type Min, const Type Max)
		{
			typename RandomDistributionType<Type> dist{ Min, Max };
			return dist(engine);
		}

	private:
		inline static std::random_device hwRandom_;
		inline static std::default_random_engine engine{ hwRandom_() };
	};
}
