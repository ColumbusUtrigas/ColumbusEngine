#pragma once

#include <cmath>
#include <System/Random.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/MathUtil.h>

namespace Columbus
{

	template <typename Type>
	struct Vector4_t;

	typedef Vector4_t<float> Vector4;
	typedef Vector4_t<double> dVector4;
	typedef Vector4_t<int> iVector4;
	typedef Vector4_t<bool> bVector4;

	template <typename Type>
	struct Vector4_t
	{
		Type X = 0;
		Type Y = 0;
		Type Z = 0;
		Type W = 0;

		Vector4_t() {}
		Vector4_t(const Vector4_t& Base) : X(Base.X), Y(Base.Y), Z(Base.Z), W(Base.W) {}
		Vector4_t(Vector4_t&& Base) :
			X(std::move(Base.X)),
			Y(std::move(Base.Y)),
			Z(std::move(Base.Z)),
			W(std::move(Base.W)) {}

		Vector4_t(const Type& Scalar) : X(Scalar), Y(Scalar), Z(Scalar), W(Scalar) {}
		Vector4_t(const Type& InX, const Type& InY, const Type& InZ, const Type& InW) : X(InX), Y(InY), Z(InZ), W(InW) {}

		Vector2_t<Type> XX() const { return Vector2_t<Type>(X, X); }
		Vector2_t<Type> XY() const { return Vector2_t<Type>(X, Y); }
		Vector2_t<Type> XZ() const { return Vector2_t<Type>(X, Z); }
		Vector2_t<Type> XW() const { return Vector2_t<Type>(X, W); }
		Vector2_t<Type> YX() const { return Vector2_t<Type>(Y, X); }
		Vector2_t<Type> YY() const { return Vector2_t<Type>(Y, Y); }
		Vector2_t<Type> YZ() const { return Vector2_t<Type>(Y, Z); }
		Vector2_t<Type> YW() const { return Vector2_t<Type>(Y, W); }
		Vector2_t<Type> ZX() const { return Vector2_t<Type>(Z, X); }
		Vector2_t<Type> ZY() const { return Vector2_t<Type>(Z, Y); }
		Vector2_t<Type> ZZ() const { return Vector2_t<Type>(Z, Z); }
		Vector2_t<Type> ZW() const { return Vector2_t<Type>(Z, W); }
		Vector2_t<Type> WX() const { return Vector2_t<Type>(W, X); }
		Vector2_t<Type> WY() const { return Vector2_t<Type>(W, Y); }
		Vector2_t<Type> WZ() const { return Vector2_t<Type>(W, Z); }
		Vector2_t<Type> WW() const { return Vector2_t<Type>(W, W); }

		Vector3_t<Type> XXX() const { return Vector3_t<Type>(X, X, X); }
		Vector3_t<Type> XXY() const { return Vector3_t<Type>(X, X, Y); }
		Vector3_t<Type> XXZ() const { return Vector3_t<Type>(X, X, Z); }
		Vector3_t<Type> XXW() const { return Vector3_t<Type>(X, X, W); }
		Vector3_t<Type> XYX() const { return Vector3_t<Type>(X, Y, X); }
		Vector3_t<Type> XYY() const { return Vector3_t<Type>(X, Y, Y); }
		Vector3_t<Type> XYZ() const { return Vector3_t<Type>(X, Y, Z); }
		Vector3_t<Type> XYW() const { return Vector3_t<Type>(X, Y, W); }
		Vector3_t<Type> XZX() const { return Vector3_t<Type>(X, Z, X); }
		Vector3_t<Type> XZY() const { return Vector3_t<Type>(X, Z, Y); }
		Vector3_t<Type> XZZ() const { return Vector3_t<Type>(X, Z, Z); }
		Vector3_t<Type> XZW() const { return Vector3_t<Type>(X, Z, W); }
		Vector3_t<Type> XWX() const { return Vector3_t<Type>(X, W, X); }
		Vector3_t<Type> XWY() const { return Vector3_t<Type>(X, W, Y); }
		Vector3_t<Type> XWZ() const { return Vector3_t<Type>(X, W, Z); }
		Vector3_t<Type> XWW() const { return Vector3_t<Type>(X, W, W); }
		Vector3_t<Type> YXX() const { return Vector3_t<Type>(Y, X, X); }
		Vector3_t<Type> YXY() const { return Vector3_t<Type>(Y, X, Y); }
		Vector3_t<Type> YXZ() const { return Vector3_t<Type>(Y, X, Z); }
		Vector3_t<Type> YXW() const { return Vector3_t<Type>(Y, X, W); }
		Vector3_t<Type> YYX() const { return Vector3_t<Type>(Y, Y, X); }
		Vector3_t<Type> YYY() const { return Vector3_t<Type>(Y, Y, Y); }
		Vector3_t<Type> YYZ() const { return Vector3_t<Type>(Y, Y, Z); }
		Vector3_t<Type> YYW() const { return Vector3_t<Type>(Y, Y, W); }
		Vector3_t<Type> YZX() const { return Vector3_t<Type>(Y, Z, X); }
		Vector3_t<Type> YZY() const { return Vector3_t<Type>(Y, Z, Y); }
		Vector3_t<Type> YZZ() const { return Vector3_t<Type>(Y, Z, Z); }
		Vector3_t<Type> YZW() const { return Vector3_t<Type>(Y, Z, W); }
		Vector3_t<Type> YWX() const { return Vector3_t<Type>(Y, W, X); }
		Vector3_t<Type> YWY() const { return Vector3_t<Type>(Y, W, Y); }
		Vector3_t<Type> YWZ() const { return Vector3_t<Type>(Y, W, Z); }
		Vector3_t<Type> YWW() const { return Vector3_t<Type>(Y, W, W); }
		Vector3_t<Type> ZXX() const { return Vector3_t<Type>(Z, X, X); }
		Vector3_t<Type> ZXY() const { return Vector3_t<Type>(Z, X, Y); }
		Vector3_t<Type> ZXZ() const { return Vector3_t<Type>(Z, X, Z); }
		Vector3_t<Type> ZXW() const { return Vector3_t<Type>(Z, X, W); }
		Vector3_t<Type> ZYX() const { return Vector3_t<Type>(Z, Y, X); }
		Vector3_t<Type> ZYY() const { return Vector3_t<Type>(Z, Y, Y); }
		Vector3_t<Type> ZYZ() const { return Vector3_t<Type>(Z, Y, Z); }
		Vector3_t<Type> ZYW() const { return Vector3_t<Type>(Z, Y, W); }
		Vector3_t<Type> ZZX() const { return Vector3_t<Type>(Z, Z, X); }
		Vector3_t<Type> ZZY() const { return Vector3_t<Type>(Z, Z, Y); }
		Vector3_t<Type> ZZZ() const { return Vector3_t<Type>(Z, Z, Z); }
		Vector3_t<Type> ZZW() const { return Vector3_t<Type>(Z, Z, W); }
		Vector3_t<Type> ZWX() const { return Vector3_t<Type>(Z, W, X); }
		Vector3_t<Type> ZWY() const { return Vector3_t<Type>(Z, W, Y); }
		Vector3_t<Type> ZWZ() const { return Vector3_t<Type>(Z, W, Z); }
		Vector3_t<Type> ZWW() const { return Vector3_t<Type>(Z, W, W); }
		Vector3_t<Type> WXX() const { return Vector3_t<Type>(W, X, X); }
		Vector3_t<Type> WXY() const { return Vector3_t<Type>(W, X, Y); }
		Vector3_t<Type> WXZ() const { return Vector3_t<Type>(W, X, Z); }
		Vector3_t<Type> WXW() const { return Vector3_t<Type>(W, X, W); }
		Vector3_t<Type> WYX() const { return Vector3_t<Type>(W, Y, X); }
		Vector3_t<Type> WYY() const { return Vector3_t<Type>(W, Y, Y); }
		Vector3_t<Type> WYZ() const { return Vector3_t<Type>(W, Y, Z); }
		Vector3_t<Type> WYW() const { return Vector3_t<Type>(W, Y, W); }
		Vector3_t<Type> WZX() const { return Vector3_t<Type>(W, Z, X); }
		Vector3_t<Type> WZY() const { return Vector3_t<Type>(W, Z, Y); }
		Vector3_t<Type> WZZ() const { return Vector3_t<Type>(W, Z, Z); }
		Vector3_t<Type> WZW() const { return Vector3_t<Type>(W, Z, W); }
		Vector3_t<Type> WWX() const { return Vector3_t<Type>(W, W, X); }
		Vector3_t<Type> WWY() const { return Vector3_t<Type>(W, W, Y); }
		Vector3_t<Type> WWZ() const { return Vector3_t<Type>(W, W, Z); }
		Vector3_t<Type> WWW() const { return Vector3_t<Type>(W, W, W); }

		Vector4_t<Type> XXXX() const { return Vector4_t<Type>(X, X, X, X); }
		Vector4_t<Type> XXXY() const { return Vector4_t<Type>(X, X, X, Y); }
		Vector4_t<Type> XXXZ() const { return Vector4_t<Type>(X, X, X, Z); }
		Vector4_t<Type> XXXW() const { return Vector4_t<Type>(X, X, X, W); }
		Vector4_t<Type> XXYX() const { return Vector4_t<Type>(X, X, Y, X); }
		Vector4_t<Type> XXYY() const { return Vector4_t<Type>(X, X, Y, Y); }
		Vector4_t<Type> XXYZ() const { return Vector4_t<Type>(X, X, Y, Z); }
		Vector4_t<Type> XXYW() const { return Vector4_t<Type>(X, X, Y, W); }
		Vector4_t<Type> XXZX() const { return Vector4_t<Type>(X, X, Z, X); }
		Vector4_t<Type> XXZY() const { return Vector4_t<Type>(X, X, Z, Y); }
		Vector4_t<Type> XXZZ() const { return Vector4_t<Type>(X, X, Z, Z); }
		Vector4_t<Type> XXZW() const { return Vector4_t<Type>(X, X, Z, W); }
		Vector4_t<Type> XXWX() const { return Vector4_t<Type>(X, X, W, X); }
		Vector4_t<Type> XXWY() const { return Vector4_t<Type>(X, X, W, Y); }
		Vector4_t<Type> XXWZ() const { return Vector4_t<Type>(X, X, W, Z); }
		Vector4_t<Type> XXWW() const { return Vector4_t<Type>(X, X, W, W); }
		Vector4_t<Type> XYXX() const { return Vector4_t<Type>(X, Y, X, X); }
		Vector4_t<Type> XYXY() const { return Vector4_t<Type>(X, Y, X, Y); }
		Vector4_t<Type> XYXZ() const { return Vector4_t<Type>(X, Y, X, Z); }
		Vector4_t<Type> XYXW() const { return Vector4_t<Type>(X, Y, X, W); }
		Vector4_t<Type> XYYX() const { return Vector4_t<Type>(X, Y, Y, X); }
		Vector4_t<Type> XYYY() const { return Vector4_t<Type>(X, Y, Y, Y); }
		Vector4_t<Type> XYYZ() const { return Vector4_t<Type>(X, Y, Y, Z); }
		Vector4_t<Type> XYYW() const { return Vector4_t<Type>(X, Y, Y, W); }
		Vector4_t<Type> XYZX() const { return Vector4_t<Type>(X, Y, Z, X); }
		Vector4_t<Type> XYZY() const { return Vector4_t<Type>(X, Y, Z, Y); }
		Vector4_t<Type> XYZZ() const { return Vector4_t<Type>(X, Y, Z, Z); }
		Vector4_t<Type> XYZW() const { return Vector4_t<Type>(X, Y, Z, W); }
		Vector4_t<Type> XYWX() const { return Vector4_t<Type>(X, Y, W, X); }
		Vector4_t<Type> XYWY() const { return Vector4_t<Type>(X, Y, W, Y); }
		Vector4_t<Type> XYWZ() const { return Vector4_t<Type>(X, Y, W, Z); }
		Vector4_t<Type> XYWW() const { return Vector4_t<Type>(X, Y, W, W); }
		Vector4_t<Type> XZXX() const { return Vector4_t<Type>(X, Z, X, X); }
		Vector4_t<Type> XZXY() const { return Vector4_t<Type>(X, Z, X, Y); }
		Vector4_t<Type> XZXZ() const { return Vector4_t<Type>(X, Z, X, Z); }
		Vector4_t<Type> XZXW() const { return Vector4_t<Type>(X, Z, X, W); }
		Vector4_t<Type> XZYX() const { return Vector4_t<Type>(X, Z, Y, X); }
		Vector4_t<Type> XZYY() const { return Vector4_t<Type>(X, Z, Y, Y); }
		Vector4_t<Type> XZYZ() const { return Vector4_t<Type>(X, Z, Y, Z); }
		Vector4_t<Type> XZYW() const { return Vector4_t<Type>(X, Z, Y, W); }
		Vector4_t<Type> XZZX() const { return Vector4_t<Type>(X, Z, Z, X); }
		Vector4_t<Type> XZZY() const { return Vector4_t<Type>(X, Z, Z, Y); }
		Vector4_t<Type> XZZZ() const { return Vector4_t<Type>(X, Z, Z, Z); }
		Vector4_t<Type> XZZW() const { return Vector4_t<Type>(X, Z, Z, W); }
		Vector4_t<Type> XZWX() const { return Vector4_t<Type>(X, Z, W, X); }
		Vector4_t<Type> XZWY() const { return Vector4_t<Type>(X, Z, W, Y); }
		Vector4_t<Type> XZWZ() const { return Vector4_t<Type>(X, Z, W, Z); }
		Vector4_t<Type> XZWW() const { return Vector4_t<Type>(X, Z, W, W); }
		Vector4_t<Type> XWXX() const { return Vector4_t<Type>(X, W, X, X); }
		Vector4_t<Type> XWXY() const { return Vector4_t<Type>(X, W, X, Y); }
		Vector4_t<Type> XWXZ() const { return Vector4_t<Type>(X, W, X, Z); }
		Vector4_t<Type> XWXW() const { return Vector4_t<Type>(X, W, X, W); }
		Vector4_t<Type> XWYX() const { return Vector4_t<Type>(X, W, Y, X); }
		Vector4_t<Type> XWYY() const { return Vector4_t<Type>(X, W, Y, Y); }
		Vector4_t<Type> XWYZ() const { return Vector4_t<Type>(X, W, Y, Z); }
		Vector4_t<Type> XWYW() const { return Vector4_t<Type>(X, W, Y, W); }
		Vector4_t<Type> XWZX() const { return Vector4_t<Type>(X, W, Z, X); }
		Vector4_t<Type> XWZY() const { return Vector4_t<Type>(X, W, Z, Y); }
		Vector4_t<Type> XWZZ() const { return Vector4_t<Type>(X, W, Z, Z); }
		Vector4_t<Type> XWZW() const { return Vector4_t<Type>(X, W, Z, W); }
		Vector4_t<Type> XWWX() const { return Vector4_t<Type>(X, W, W, X); }
		Vector4_t<Type> XWWY() const { return Vector4_t<Type>(X, W, W, Y); }
		Vector4_t<Type> XWWZ() const { return Vector4_t<Type>(X, W, W, Z); }
		Vector4_t<Type> XWWW() const { return Vector4_t<Type>(X, W, W, W); }
		Vector4_t<Type> YXXX() const { return Vector4_t<Type>(Y, X, X, X); }
		Vector4_t<Type> YXXY() const { return Vector4_t<Type>(Y, X, X, Y); }
		Vector4_t<Type> YXXZ() const { return Vector4_t<Type>(Y, X, X, Z); }
		Vector4_t<Type> YXXW() const { return Vector4_t<Type>(Y, X, X, W); }
		Vector4_t<Type> YXYX() const { return Vector4_t<Type>(Y, X, Y, X); }
		Vector4_t<Type> YXYY() const { return Vector4_t<Type>(Y, X, Y, Y); }
		Vector4_t<Type> YXYZ() const { return Vector4_t<Type>(Y, X, Y, Z); }
		Vector4_t<Type> YXYW() const { return Vector4_t<Type>(Y, X, Y, W); }
		Vector4_t<Type> YXZX() const { return Vector4_t<Type>(Y, X, Z, X); }
		Vector4_t<Type> YXZY() const { return Vector4_t<Type>(Y, X, Z, Y); }
		Vector4_t<Type> YXZZ() const { return Vector4_t<Type>(Y, X, Z, Z); }
		Vector4_t<Type> YXZW() const { return Vector4_t<Type>(Y, X, Z, W); }
		Vector4_t<Type> YXWX() const { return Vector4_t<Type>(Y, X, W, X); }
		Vector4_t<Type> YXWY() const { return Vector4_t<Type>(Y, X, W, Y); }
		Vector4_t<Type> YXWZ() const { return Vector4_t<Type>(Y, X, W, Z); }
		Vector4_t<Type> YXWW() const { return Vector4_t<Type>(Y, X, W, W); }
		Vector4_t<Type> YYXX() const { return Vector4_t<Type>(Y, Y, X, X); }
		Vector4_t<Type> YYXY() const { return Vector4_t<Type>(Y, Y, X, Y); }
		Vector4_t<Type> YYXZ() const { return Vector4_t<Type>(Y, Y, X, Z); }
		Vector4_t<Type> YYXW() const { return Vector4_t<Type>(Y, Y, X, W); }
		Vector4_t<Type> YYYX() const { return Vector4_t<Type>(Y, Y, Y, X); }
		Vector4_t<Type> YYYY() const { return Vector4_t<Type>(Y, Y, Y, Y); }
		Vector4_t<Type> YYYZ() const { return Vector4_t<Type>(Y, Y, Y, Z); }
		Vector4_t<Type> YYYW() const { return Vector4_t<Type>(Y, Y, Y, W); }
		Vector4_t<Type> YYZX() const { return Vector4_t<Type>(Y, Y, Z, X); }
		Vector4_t<Type> YYZY() const { return Vector4_t<Type>(Y, Y, Z, Y); }
		Vector4_t<Type> YYZZ() const { return Vector4_t<Type>(Y, Y, Z, Z); }
		Vector4_t<Type> YYZW() const { return Vector4_t<Type>(Y, Y, Z, W); }
		Vector4_t<Type> YYWX() const { return Vector4_t<Type>(Y, Y, W, X); }
		Vector4_t<Type> YYWY() const { return Vector4_t<Type>(Y, Y, W, Y); }
		Vector4_t<Type> YYWZ() const { return Vector4_t<Type>(Y, Y, W, Z); }
		Vector4_t<Type> YYWW() const { return Vector4_t<Type>(Y, Y, W, W); }
		Vector4_t<Type> YZXX() const { return Vector4_t<Type>(Y, Z, X, X); }
		Vector4_t<Type> YZXY() const { return Vector4_t<Type>(Y, Z, X, Y); }
		Vector4_t<Type> YZXZ() const { return Vector4_t<Type>(Y, Z, X, Z); }
		Vector4_t<Type> YZXW() const { return Vector4_t<Type>(Y, Z, X, W); }
		Vector4_t<Type> YZYX() const { return Vector4_t<Type>(Y, Z, Y, X); }
		Vector4_t<Type> YZYY() const { return Vector4_t<Type>(Y, Z, Y, Y); }
		Vector4_t<Type> YZYZ() const { return Vector4_t<Type>(Y, Z, Y, Z); }
		Vector4_t<Type> YZYW() const { return Vector4_t<Type>(Y, Z, Y, W); }
		Vector4_t<Type> YZZX() const { return Vector4_t<Type>(Y, Z, Z, X); }
		Vector4_t<Type> YZZY() const { return Vector4_t<Type>(Y, Z, Z, Y); }
		Vector4_t<Type> YZZZ() const { return Vector4_t<Type>(Y, Z, Z, Z); }
		Vector4_t<Type> YZZW() const { return Vector4_t<Type>(Y, Z, Z, W); }
		Vector4_t<Type> YZWX() const { return Vector4_t<Type>(Y, Z, W, X); }
		Vector4_t<Type> YZWY() const { return Vector4_t<Type>(Y, Z, W, Y); }
		Vector4_t<Type> YZWZ() const { return Vector4_t<Type>(Y, Z, W, Z); }
		Vector4_t<Type> YZWW() const { return Vector4_t<Type>(Y, Z, W, W); }
		Vector4_t<Type> YWXX() const { return Vector4_t<Type>(Y, W, X, X); }
		Vector4_t<Type> YWXY() const { return Vector4_t<Type>(Y, W, X, Y); }
		Vector4_t<Type> YWXZ() const { return Vector4_t<Type>(Y, W, X, Z); }
		Vector4_t<Type> YWXW() const { return Vector4_t<Type>(Y, W, X, W); }
		Vector4_t<Type> YWYX() const { return Vector4_t<Type>(Y, W, Y, X); }
		Vector4_t<Type> YWYY() const { return Vector4_t<Type>(Y, W, Y, Y); }
		Vector4_t<Type> YWYZ() const { return Vector4_t<Type>(Y, W, Y, Z); }
		Vector4_t<Type> YWYW() const { return Vector4_t<Type>(Y, W, Y, W); }
		Vector4_t<Type> YWZX() const { return Vector4_t<Type>(Y, W, Z, X); }
		Vector4_t<Type> YWZY() const { return Vector4_t<Type>(Y, W, Z, Y); }
		Vector4_t<Type> YWZZ() const { return Vector4_t<Type>(Y, W, Z, Z); }
		Vector4_t<Type> YWZW() const { return Vector4_t<Type>(Y, W, Z, W); }
		Vector4_t<Type> YWWX() const { return Vector4_t<Type>(Y, W, W, X); }
		Vector4_t<Type> YWWY() const { return Vector4_t<Type>(Y, W, W, Y); }
		Vector4_t<Type> YWWZ() const { return Vector4_t<Type>(Y, W, W, Z); }
		Vector4_t<Type> YWWW() const { return Vector4_t<Type>(Y, W, W, W); }
		Vector4_t<Type> ZXXX() const { return Vector4_t<Type>(Z, X, X, X); }
		Vector4_t<Type> ZXXY() const { return Vector4_t<Type>(Z, X, X, Y); }
		Vector4_t<Type> ZXXZ() const { return Vector4_t<Type>(Z, X, X, Z); }
		Vector4_t<Type> ZXXW() const { return Vector4_t<Type>(Z, X, X, W); }
		Vector4_t<Type> ZXYX() const { return Vector4_t<Type>(Z, X, Y, X); }
		Vector4_t<Type> ZXYY() const { return Vector4_t<Type>(Z, X, Y, Y); }
		Vector4_t<Type> ZXYZ() const { return Vector4_t<Type>(Z, X, Y, Z); }
		Vector4_t<Type> ZXYW() const { return Vector4_t<Type>(Z, X, Y, W); }
		Vector4_t<Type> ZXZX() const { return Vector4_t<Type>(Z, X, Z, X); }
		Vector4_t<Type> ZXZY() const { return Vector4_t<Type>(Z, X, Z, Y); }
		Vector4_t<Type> ZXZZ() const { return Vector4_t<Type>(Z, X, Z, Z); }
		Vector4_t<Type> ZXZW() const { return Vector4_t<Type>(Z, X, Z, W); }
		Vector4_t<Type> ZXWX() const { return Vector4_t<Type>(Z, X, W, X); }
		Vector4_t<Type> ZXWY() const { return Vector4_t<Type>(Z, X, W, Y); }
		Vector4_t<Type> ZXWZ() const { return Vector4_t<Type>(Z, X, W, Z); }
		Vector4_t<Type> ZXWW() const { return Vector4_t<Type>(Z, X, W, W); }
		Vector4_t<Type> ZYXX() const { return Vector4_t<Type>(Z, Y, X, X); }
		Vector4_t<Type> ZYXY() const { return Vector4_t<Type>(Z, Y, X, Y); }
		Vector4_t<Type> ZYXZ() const { return Vector4_t<Type>(Z, Y, X, Z); }
		Vector4_t<Type> ZYXW() const { return Vector4_t<Type>(Z, Y, X, W); }
		Vector4_t<Type> ZYYX() const { return Vector4_t<Type>(Z, Y, Y, X); }
		Vector4_t<Type> ZYYY() const { return Vector4_t<Type>(Z, Y, Y, Y); }
		Vector4_t<Type> ZYYZ() const { return Vector4_t<Type>(Z, Y, Y, Z); }
		Vector4_t<Type> ZYYW() const { return Vector4_t<Type>(Z, Y, Y, W); }
		Vector4_t<Type> ZYZX() const { return Vector4_t<Type>(Z, Y, Z, X); }
		Vector4_t<Type> ZYZY() const { return Vector4_t<Type>(Z, Y, Z, Y); }
		Vector4_t<Type> ZYZZ() const { return Vector4_t<Type>(Z, Y, Z, Z); }
		Vector4_t<Type> ZYZW() const { return Vector4_t<Type>(Z, Y, Z, W); }
		Vector4_t<Type> ZYWX() const { return Vector4_t<Type>(Z, Y, W, X); }
		Vector4_t<Type> ZYWY() const { return Vector4_t<Type>(Z, Y, W, Y); }
		Vector4_t<Type> ZYWZ() const { return Vector4_t<Type>(Z, Y, W, Z); }
		Vector4_t<Type> ZYWW() const { return Vector4_t<Type>(Z, Y, W, W); }
		Vector4_t<Type> ZZXX() const { return Vector4_t<Type>(Z, Z, X, X); }
		Vector4_t<Type> ZZXY() const { return Vector4_t<Type>(Z, Z, X, Y); }
		Vector4_t<Type> ZZXZ() const { return Vector4_t<Type>(Z, Z, X, Z); }
		Vector4_t<Type> ZZXW() const { return Vector4_t<Type>(Z, Z, X, W); }
		Vector4_t<Type> ZZYX() const { return Vector4_t<Type>(Z, Z, Y, X); }
		Vector4_t<Type> ZZYY() const { return Vector4_t<Type>(Z, Z, Y, Y); }
		Vector4_t<Type> ZZYZ() const { return Vector4_t<Type>(Z, Z, Y, Z); }
		Vector4_t<Type> ZZYW() const { return Vector4_t<Type>(Z, Z, Y, W); }
		Vector4_t<Type> ZZZX() const { return Vector4_t<Type>(Z, Z, Z, X); }
		Vector4_t<Type> ZZZY() const { return Vector4_t<Type>(Z, Z, Z, Y); }
		Vector4_t<Type> ZZZZ() const { return Vector4_t<Type>(Z, Z, Z, Z); }
		Vector4_t<Type> ZZZW() const { return Vector4_t<Type>(Z, Z, Z, W); }
		Vector4_t<Type> ZZWX() const { return Vector4_t<Type>(Z, Z, W, X); }
		Vector4_t<Type> ZZWY() const { return Vector4_t<Type>(Z, Z, W, Y); }
		Vector4_t<Type> ZZWZ() const { return Vector4_t<Type>(Z, Z, W, Z); }
		Vector4_t<Type> ZZWW() const { return Vector4_t<Type>(Z, Z, W, W); }
		Vector4_t<Type> ZWXX() const { return Vector4_t<Type>(Z, W, X, X); }
		Vector4_t<Type> ZWXY() const { return Vector4_t<Type>(Z, W, X, Y); }
		Vector4_t<Type> ZWXZ() const { return Vector4_t<Type>(Z, W, X, Z); }
		Vector4_t<Type> ZWXW() const { return Vector4_t<Type>(Z, W, X, W); }
		Vector4_t<Type> ZWYX() const { return Vector4_t<Type>(Z, W, Y, X); }
		Vector4_t<Type> ZWYY() const { return Vector4_t<Type>(Z, W, Y, Y); }
		Vector4_t<Type> ZWYZ() const { return Vector4_t<Type>(Z, W, Y, Z); }
		Vector4_t<Type> ZWYW() const { return Vector4_t<Type>(Z, W, Y, W); }
		Vector4_t<Type> ZWZX() const { return Vector4_t<Type>(Z, W, Z, X); }
		Vector4_t<Type> ZWZY() const { return Vector4_t<Type>(Z, W, Z, Y); }
		Vector4_t<Type> ZWZZ() const { return Vector4_t<Type>(Z, W, Z, Z); }
		Vector4_t<Type> ZWZW() const { return Vector4_t<Type>(Z, W, Z, W); }
		Vector4_t<Type> ZWWX() const { return Vector4_t<Type>(Z, W, W, X); }
		Vector4_t<Type> ZWWY() const { return Vector4_t<Type>(Z, W, W, Y); }
		Vector4_t<Type> ZWWZ() const { return Vector4_t<Type>(Z, W, W, Z); }
		Vector4_t<Type> ZWWW() const { return Vector4_t<Type>(Z, W, W, W); }
		Vector4_t<Type> WXXX() const { return Vector4_t<Type>(W, X, X, X); }
		Vector4_t<Type> WXXY() const { return Vector4_t<Type>(W, X, X, Y); }
		Vector4_t<Type> WXXZ() const { return Vector4_t<Type>(W, X, X, Z); }
		Vector4_t<Type> WXXW() const { return Vector4_t<Type>(W, X, X, W); }
		Vector4_t<Type> WXYX() const { return Vector4_t<Type>(W, X, Y, X); }
		Vector4_t<Type> WXYY() const { return Vector4_t<Type>(W, X, Y, Y); }
		Vector4_t<Type> WXYZ() const { return Vector4_t<Type>(W, X, Y, Z); }
		Vector4_t<Type> WXYW() const { return Vector4_t<Type>(W, X, Y, W); }
		Vector4_t<Type> WXZX() const { return Vector4_t<Type>(W, X, Z, X); }
		Vector4_t<Type> WXZY() const { return Vector4_t<Type>(W, X, Z, Y); }
		Vector4_t<Type> WXZZ() const { return Vector4_t<Type>(W, X, Z, Z); }
		Vector4_t<Type> WXZW() const { return Vector4_t<Type>(W, X, Z, W); }
		Vector4_t<Type> WXWX() const { return Vector4_t<Type>(W, X, W, X); }
		Vector4_t<Type> WXWY() const { return Vector4_t<Type>(W, X, W, Y); }
		Vector4_t<Type> WXWZ() const { return Vector4_t<Type>(W, X, W, Z); }
		Vector4_t<Type> WXWW() const { return Vector4_t<Type>(W, X, W, W); }
		Vector4_t<Type> WYXX() const { return Vector4_t<Type>(W, Y, X, X); }
		Vector4_t<Type> WYXY() const { return Vector4_t<Type>(W, Y, X, Y); }
		Vector4_t<Type> WYXZ() const { return Vector4_t<Type>(W, Y, X, Z); }
		Vector4_t<Type> WYXW() const { return Vector4_t<Type>(W, Y, X, W); }
		Vector4_t<Type> WYYX() const { return Vector4_t<Type>(W, Y, Y, X); }
		Vector4_t<Type> WYYY() const { return Vector4_t<Type>(W, Y, Y, Y); }
		Vector4_t<Type> WYYZ() const { return Vector4_t<Type>(W, Y, Y, Z); }
		Vector4_t<Type> WYYW() const { return Vector4_t<Type>(W, Y, Y, W); }
		Vector4_t<Type> WYZX() const { return Vector4_t<Type>(W, Y, Z, X); }
		Vector4_t<Type> WYZY() const { return Vector4_t<Type>(W, Y, Z, Y); }
		Vector4_t<Type> WYZZ() const { return Vector4_t<Type>(W, Y, Z, Z); }
		Vector4_t<Type> WYZW() const { return Vector4_t<Type>(W, Y, Z, W); }
		Vector4_t<Type> WYWX() const { return Vector4_t<Type>(W, Y, W, X); }
		Vector4_t<Type> WYWY() const { return Vector4_t<Type>(W, Y, W, Y); }
		Vector4_t<Type> WYWZ() const { return Vector4_t<Type>(W, Y, W, Z); }
		Vector4_t<Type> WYWW() const { return Vector4_t<Type>(W, Y, W, W); }
		Vector4_t<Type> WZXX() const { return Vector4_t<Type>(W, Z, X, X); }
		Vector4_t<Type> WZXY() const { return Vector4_t<Type>(W, Z, X, Y); }
		Vector4_t<Type> WZXZ() const { return Vector4_t<Type>(W, Z, X, Z); }
		Vector4_t<Type> WZXW() const { return Vector4_t<Type>(W, Z, X, W); }
		Vector4_t<Type> WZYX() const { return Vector4_t<Type>(W, Z, Y, X); }
		Vector4_t<Type> WZYY() const { return Vector4_t<Type>(W, Z, Y, Y); }
		Vector4_t<Type> WZYZ() const { return Vector4_t<Type>(W, Z, Y, Z); }
		Vector4_t<Type> WZYW() const { return Vector4_t<Type>(W, Z, Y, W); }
		Vector4_t<Type> WZZX() const { return Vector4_t<Type>(W, Z, Z, X); }
		Vector4_t<Type> WZZY() const { return Vector4_t<Type>(W, Z, Z, Y); }
		Vector4_t<Type> WZZZ() const { return Vector4_t<Type>(W, Z, Z, Z); }
		Vector4_t<Type> WZZW() const { return Vector4_t<Type>(W, Z, Z, W); }
		Vector4_t<Type> WZWX() const { return Vector4_t<Type>(W, Z, W, X); }
		Vector4_t<Type> WZWY() const { return Vector4_t<Type>(W, Z, W, Y); }
		Vector4_t<Type> WZWZ() const { return Vector4_t<Type>(W, Z, W, Z); }
		Vector4_t<Type> WZWW() const { return Vector4_t<Type>(W, Z, W, W); }
		Vector4_t<Type> WWXX() const { return Vector4_t<Type>(W, W, X, X); }
		Vector4_t<Type> WWXY() const { return Vector4_t<Type>(W, W, X, Y); }
		Vector4_t<Type> WWXZ() const { return Vector4_t<Type>(W, W, X, Z); }
		Vector4_t<Type> WWXW() const { return Vector4_t<Type>(W, W, X, W); }
		Vector4_t<Type> WWYX() const { return Vector4_t<Type>(W, W, Y, X); }
		Vector4_t<Type> WWYY() const { return Vector4_t<Type>(W, W, Y, Y); }
		Vector4_t<Type> WWYZ() const { return Vector4_t<Type>(W, W, Y, Z); }
		Vector4_t<Type> WWYW() const { return Vector4_t<Type>(W, W, Y, W); }
		Vector4_t<Type> WWZX() const { return Vector4_t<Type>(W, W, Z, X); }
		Vector4_t<Type> WWZY() const { return Vector4_t<Type>(W, W, Z, Y); }
		Vector4_t<Type> WWZZ() const { return Vector4_t<Type>(W, W, Z, Z); }
		Vector4_t<Type> WWZW() const { return Vector4_t<Type>(W, W, Z, W); }
		Vector4_t<Type> WWWX() const { return Vector4_t<Type>(W, W, W, X); }
		Vector4_t<Type> WWWY() const { return Vector4_t<Type>(W, W, W, Y); }
		Vector4_t<Type> WWWZ() const { return Vector4_t<Type>(W, W, W, Z); }
		Vector4_t<Type> WWWW() const { return Vector4_t<Type>(W, W, W, W); }

		Vector4_t& operator=(const Vector4_t& Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;
			W = Other.W;
			return *this;
		}

		Vector4_t& operator=(Vector4_t&& Other)
		{
			X = std::move(Other.X);
			Y = std::move(Other.Y);
			Z = std::move(Other.Z);
			W = std::move(Other.W);
			return *this;
		}

		Vector4_t operator+() const
		{
			return *this;
		}

		Vector4_t operator+(const Type& Scalar) const
		{
			return Vector4_t(X + Scalar, Y + Scalar, Z + Scalar, W + Scalar);
		}

		friend Vector4_t operator+(const Type& Scalar, const Vector4_t& Other)
		{
			return Other + Scalar;
		}

		Vector4_t operator+(const Vector4_t& Other) const
		{
			return Vector4_t(X + Other.X, Y + Other.Y, Z + Other.Z, W + Other.W);
		}

		Vector4_t operator-() const
		{
			return Vector4_t(-X, -Y, -Z, -W);
		}

		Vector4_t operator-(const Type& Scalar) const
		{
			return Vector4_t(X - Scalar, Y - Scalar, Z - Scalar, W - Scalar);
		}

		friend Vector4_t operator-(const Type& Scalar, const Vector4_t& Other)
		{
			return Vector4_t(Scalar - Other.X, Scalar - Other.Y, Scalar - Other.Z, Scalar - Other.W);
		}

		Vector4_t operator-(const Vector4_t& Other) const
		{
			return Vector4_t(X - Other.X, Y - Other.Y, Z - Other.Z, W - Other.W);
		}

		Vector4_t operator*(const Type& Scalar) const
		{
			return Vector4_t(X * Scalar, Y * Scalar, Z * Scalar, W * Scalar);
		}

		friend Vector4_t operator*(const Type& Scalar, const Vector4_t& Other)
		{
			return Other * Scalar;
		}

		Vector4_t operator*(const Vector4_t& Other) const
		{
			return Vector4_t(X * Other.X, Y * Other.Y, Z * Other.Z, W * Other.W);
		}

		Vector4_t operator/(const Type& Scalar) const
		{
			const Type Factor = 1.0 / Scalar;
			return Vector4_t(X * Factor, Y * Factor, Z * Factor, W * Factor);
		}

		friend Vector4_t operator/(const Type& Scalar, const Vector4_t& Other)
		{
			return Vector4_t(Scalar / Other.X, Scalar / Other.Y, Scalar / Other.Z, Scalar / Other.W);
		}

		Vector4_t operator/(const Vector4_t& Other) const
		{
			return Vector4_t(X / Other.X, Y / Other.Y, Z / Other.Z, W / Other.W);
		}

		Vector4_t& operator+=(const Type& Scalar)
		{
			return *this = *this + Scalar;
		}

		Vector4_t& operator+=(const Vector4_t& Other)
		{
			return *this = *this + Other;
		}

		Vector4_t& operator-=(const Type& Scalar)
		{
			return *this = *this - Scalar;
		}

		Vector4_t& operator-=(const Vector4_t& Other)
		{
			return *this = *this - Other;
		}

		Vector4_t& operator*=(const Type& Scalar)
		{
			return *this = *this * Scalar;
		}

		Vector4_t& operator*=(const Vector4_t& Other)
		{
			return *this = *this * Other;
		}

		Vector4_t& operator/=(const Type& Scalar)
		{
			return *this = *this / Scalar;
		}

		Vector4_t& operator/=(const Vector4_t& Other)
		{
			return *this = *this / Other;
		}

		bool operator==(const Vector4_t& Other) const
		{
			return X == Other.X && Y == Other.Y && Z == Other.Z && W == Other.W;
		}

		bool operator!=(const Vector4_t& Other) const
		{
			return !(*this == Other);
		}

		static Vector4_t Random(const Vector4_t& Min, const Vector4_t& Max)
		{
			Vector4 Result;

			Result.X = Random::range(Min.X, Max.X);
			Result.Y = Random::range(Min.Y, Max.Y);
			Result.Z = Random::range(Min.Z, Max.Z);
			Result.W = Random::range(Min.W, Max.W);

			return Result;
		}

		Vector4_t Clamped(const Vector4_t& Min, const Vector4_t& Max) const
		{
			return Vector4_t(Clamp(X, Min.X, Max.X), Clamp(Y, Min.Y, Max.Y), Clamp(Z, Min.Z, Max.Z), Clamp(W, Min.W, Max.W));
		}

		Vector4_t& Clamp(const Vector4_t& Min, const Vector4_t& Max)
		{
			return *this = Clamped(Min, Max);
		}

		Vector4_t Normalized() const
		{
			return *this * (1.0 / Math::Sqrt(X * X + Y * Y + Z * Z + W * W));
		}

		Vector4_t& Normalize()
		{
			return *this = Normalized();
		}

		Type Length(const Vector4_t& Other) const
		{
			return Math::Sqrt(Math::Pow(Other.X - X, 2) + Math::Pow(Other.Y - Y, 2) + Math::Pow(Other.Z - Z, 2) + Math::Pow(Other.W - W, 2));
		}

		Type Dot(const Vector4_t& Other) const
		{
			return X * Other.X + Y * Other.Y + Z * Other.Z + W * Other.W;
		}
	};

}





