#pragma once

#include <Math/Box.h>
#include <Math/InterpolationCurve.h>
#include <Math/Quaternion.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <json.hpp>

namespace nlohmann
{

	// curve
	template <typename T>
	struct adl_serializer<Columbus::InterpolationCurve<T>>
	{
		static void to_json(json& j, const Columbus::InterpolationCurve<T>& c)
		{
			for (size_t i = 0; i < c.Points.size(); i++)
			{
				j[i]["key"] = c.Points[i].Key;
				j[i]["value"] = c.Points[i].Value;
			}
		}

		static void from_json(const json& j, Columbus::InterpolationCurve<T>& c)
		{
			for (const auto& p : j)
			{
				c.AddPoint(p["value"].get<T>(), p["key"].get<float>())
			}
		}
	};

	// vector2
	template <typename T>
	struct adl_serializer<Columbus::Vector2_t<T>>
	{
		static void to_json(json& j, const Columbus::Vector2_t<T>& v)
		{
			j[0] = v.X;
			j[1] = v.Y;
		}

		static void from_json(const json& j, Columbus::Vector2_t<T>& v)
		{
			v.X = j[0];
			v.Y = j[1];
		}
	};

	// vector3
	template <typename T>
	struct adl_serializer<Columbus::Vector3_t<T>>
	{
		static void to_json(json& j, const Columbus::Vector3_t<T>& v)
		{
			j[0] = v.X;
			j[1] = v.Y;
			j[2] = v.Z;
		}

		static void from_json(const json& j, Columbus::Vector3_t<T>& v)
		{
			v.X = j[0];
			v.Y = j[1];
			v.Z = j[2];
		}
	};

	// vector4
	template <typename T>
	struct adl_serializer<Columbus::Vector4_t<T>>
	{
		static void to_json(json& j, const Columbus::Vector4_t<T>& v)
		{
			j[0] = v.X;
			j[1] = v.Y;
			j[2] = v.Z;
			j[3] = v.W;
		}

		static void from_json(const json& j, Columbus::Vector4_t<T>& v)
		{
			v.X = j[0];
			v.Y = j[1];
			v.Z = j[2];
			v.W = j[3];
		}
	};

	// box
	template <>
	struct adl_serializer<Columbus::Box>
	{
		static void to_json(json& j, const Columbus::Box& b)
		{
			j["min"] = b.Min;
			j["max"] = b.Max;
		}

		static void from_json(const json& j, Columbus::Box& b)
		{
			b.Min = j["min"].get<Columbus::Vector3>();
			b.Max = j["max"].get<Columbus::Vector3>();
		}
	};

}
