﻿#pragma once
// used: [stl] numeric_limits
#include <limits>
// used: [crt] isfinite, fmodf, sqrtf
#include <cmath>
#include <algorithm>

// forward declarations
struct QAngle_t;
struct Matrix3x4_t;

// @source: master/public/mathlib/vector.h

struct Vector2D_t
{
	constexpr Vector2D_t(const float x = 0.0f, const float y = 0.0f) :
		x(x), y(y) { }


#pragma region vector_assignment_operators

	constexpr Vector2D_t& operator=(const Vector2D_t& vecBase2D)
	{
		this->x = vecBase2D.x;
		this->y = vecBase2D.y;
		return *this;
	}

#pragma endregion

#pragma region vector_arithmetic_assignment_operators

	constexpr Vector2D_t& operator+=(const Vector2D_t& vecBase)
	{
		this->x += vecBase.x;
		this->y += vecBase.y;
		return *this;
	}

	constexpr Vector2D_t& operator-=(const Vector2D_t& vecBase)
	{
		this->x -= vecBase.x;
		this->y -= vecBase.y;
		return *this;
	}

	constexpr Vector2D_t& operator*=(const Vector2D_t& vecBase)
	{
		this->x *= vecBase.x;
		this->y *= vecBase.y;
		return *this;
	}

	constexpr Vector2D_t& operator/=(const Vector2D_t& vecBase)
	{
		this->x /= vecBase.x;
		this->y /= vecBase.y;
		return *this;
	}

	constexpr Vector2D_t& operator+=(const float flAdd)
	{
		this->x += flAdd;
		this->y += flAdd;
		return *this;
	}

	constexpr Vector2D_t& operator-=(const float flSubtract)
	{
		this->x -= flSubtract;
		this->y -= flSubtract;
		return *this;
	}

	constexpr Vector2D_t& operator*=(const float flMultiply)
	{
		this->x *= flMultiply;
		this->y *= flMultiply;
		return *this;
	}

	constexpr Vector2D_t& operator/=(const float flDivide)
	{
		this->x /= flDivide;
		this->y /= flDivide;
		return *this;
	}

#pragma endregion

#pragma region vector_arithmetic_unary_operators

	constexpr Vector2D_t& operator-()
	{
		this->x = -this->x;
		this->y = -this->y;
		return *this;
	}

	constexpr Vector2D_t operator-() const
	{
		return { -this->x, -this->y };
	}

#pragma endregion

#pragma region vector_arithmetic_ternary_operators

	Vector2D_t operator+(const Vector2D_t& vecAdd) const
	{
		return { this->x + vecAdd.x, this->y + vecAdd.y };
	}

	Vector2D_t operator-(const Vector2D_t& vecSubtract) const
	{
		return { this->x - vecSubtract.x, this->y - vecSubtract.y };
	}

	Vector2D_t operator*(const Vector2D_t& vecMultiply) const
	{
		return { this->x * vecMultiply.x, this->y * vecMultiply.y };
	}

	Vector2D_t operator/(const Vector2D_t& vecDivide) const
	{
		return { this->x / vecDivide.x, this->y / vecDivide.y };
	}

	Vector2D_t operator+(const float flAdd) const
	{
		return { this->x + flAdd, this->y + flAdd };
	}

	Vector2D_t operator-(const float flSubtract) const
	{
		return { this->x - flSubtract, this->y - flSubtract };
	}

	Vector2D_t operator*(const float flMultiply) const
	{
		return { this->x * flMultiply, this->y * flMultiply };
	}

	Vector2D_t operator/(const float flDivide) const
	{
		return { this->x / flDivide, this->y / flDivide };
	}

#pragma endregion

	[[nodiscard]] bool IsZero() const
	{
		// @note: to make this implementation right, we should use fpclassify here, but game aren't doing same, probably it's better to keep this same, just ensure that it will be compiled same
		return (this->x == 0.0f && this->y == 0.0f);
	}

	[[nodiscard]] float Dot(const Vector2D_t& v) const {
		return (this->x * v.x +
			this->y * v.y);
	}

	float LengthSquared() const {
		return (this->Dot(*this));
	}

	float Length() const {
		return (std::sqrt(this->LengthSquared()));
	}

	float Length2DSquared() const {
		return (x * x + y * y);
	}


	float x = 0.0f, y = 0.0f;
};

struct Vector_t
{
	constexpr Vector_t(const float x = 0.0f, const float y = 0.0f, const float z = 0.0f) :
		x(x), y(y), z(z) { }

	constexpr Vector_t(const float* arrVector) :
		x(arrVector[0]), y(arrVector[1]), z(arrVector[2]) { }

	constexpr Vector_t(const Vector2D_t& vecBase2D) :
		x(vecBase2D.x), y(vecBase2D.y) { }

#pragma region vector_array_operators

	[[nodiscard]] float& operator[](const int nIndex)
	{
		return reinterpret_cast<float*>(this)[nIndex];
	}

	[[nodiscard]] const float& operator[](const int nIndex) const
	{
		return reinterpret_cast<const float*>(this)[nIndex];
	}

#pragma endregion

#pragma region vector_relational_operators

	bool operator==(const Vector_t& vecBase) const
	{
		return this->IsEqual(vecBase);
	}

	bool operator!=(const Vector_t& vecBase) const
	{
		return !this->IsEqual(vecBase);
	}

#pragma endregion

#pragma region vector_assignment_operators

	constexpr Vector_t& operator=(const Vector_t& vecBase)
	{
		this->x = vecBase.x;
		this->y = vecBase.y;
		this->z = vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator=(const Vector2D_t& vecBase2D)
	{
		this->x = vecBase2D.x;
		this->y = vecBase2D.y;
		this->z = 0.0f;
		return *this;
	}

#pragma endregion

#pragma region vector_arithmetic_assignment_operators

	constexpr Vector_t& operator+=(const Vector_t& vecBase)
	{
		this->x += vecBase.x;
		this->y += vecBase.y;
		this->z += vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator-=(const Vector_t& vecBase)
	{
		this->x -= vecBase.x;
		this->y -= vecBase.y;
		this->z -= vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator*=(const Vector_t& vecBase)
	{
		this->x *= vecBase.x;
		this->y *= vecBase.y;
		this->z *= vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator/=(const Vector_t& vecBase)
	{
		this->x /= vecBase.x;
		this->y /= vecBase.y;
		this->z /= vecBase.z;
		return *this;
	}

	constexpr Vector_t& operator+=(const float flAdd)
	{
		this->x += flAdd;
		this->y += flAdd;
		this->z += flAdd;
		return *this;
	}

	constexpr Vector_t& operator-=(const float flSubtract)
	{
		this->x -= flSubtract;
		this->y -= flSubtract;
		this->z -= flSubtract;
		return *this;
	}

	constexpr Vector_t& operator*=(const float flMultiply)
	{
		this->x *= flMultiply;
		this->y *= flMultiply;
		this->z *= flMultiply;
		return *this;
	}

	constexpr Vector_t& operator/=(const float flDivide)
	{
		this->x /= flDivide;
		this->y /= flDivide;
		this->z /= flDivide;
		return *this;
	}

#pragma endregion

#pragma region vector_arithmetic_unary_operators

	constexpr Vector_t& operator-()
	{
		this->x = -this->x;
		this->y = -this->y;
		this->z = -this->z;
		return *this;
	}

	constexpr Vector_t operator-() const
	{
		return { -this->x, -this->y, -this->z };
	}

#pragma endregion

#pragma region vector_arithmetic_ternary_operators

	Vector_t operator+(const Vector_t& vecAdd) const
	{
		return { this->x + vecAdd.x, this->y + vecAdd.y, this->z + vecAdd.z };
	}

	Vector_t operator-(const Vector_t& vecSubtract) const
	{
		return { this->x - vecSubtract.x, this->y - vecSubtract.y, this->z - vecSubtract.z };
	}

	Vector_t operator*(const Vector_t& vecMultiply) const
	{
		return { this->x * vecMultiply.x, this->y * vecMultiply.y, this->z * vecMultiply.z };
	}

	Vector_t operator/(const Vector_t& vecDivide) const
	{
		return { this->x / vecDivide.x, this->y / vecDivide.y, this->z / vecDivide.z };
	}

	Vector_t operator+(const float flAdd) const
	{
		return { this->x + flAdd, this->y + flAdd, this->z + flAdd };
	}

	Vector_t operator-(const float flSubtract) const
	{
		return { this->x - flSubtract, this->y - flSubtract, this->z - flSubtract };
	}

	Vector_t operator*(const float flMultiply) const
	{
		return { this->x * flMultiply, this->y * flMultiply, this->z * flMultiply };
	}

	Vector_t operator/(const float flDivide) const
	{
		return { this->x / flDivide, this->y / flDivide, this->z / flDivide };
	}

#pragma endregion

	/// @returns: true if each component of the vector is finite, false otherwise
	[[nodiscard]] bool IsValid() const
	{
		return std::isfinite(this->x) && std::isfinite(this->y) && std::isfinite(this->z);
	}

	constexpr void Invalidate()
	{
		this->x = this->y = this->z = std::numeric_limits<float>::infinity();
	}

	/// @returns: true if each component of the vector equals to another, false otherwise
	[[nodiscard]] bool IsEqual(const Vector_t& vecEqual, const float flErrorMargin = std::numeric_limits<float>::epsilon()) const
	{
		return (std::fabsf(this->x - vecEqual.x) < flErrorMargin && std::fabsf(this->y - vecEqual.y) < flErrorMargin && std::fabsf(this->z - vecEqual.z) < flErrorMargin);
	}

	/// @returns: true if each component of the vector equals to zero, false otherwise
	bool IsZero() const
	{
		// @note: to make this implementation right, we should use fpclassify here, but game aren't doing same, probably it's better to keep this same, just ensure that it will be compiled same
		return (this->x == 0.0f && this->y == 0.0f && this->z == 0.0f);
	}

	[[nodiscard]] float Length() const
	{
		return std::sqrtf(this->LengthSqr());
	}

	[[nodiscard]] constexpr float LengthSqr() const
	{
		return DotProduct(*this);
	}

	[[nodiscard]] float Length2D() const
	{
		return std::sqrtf(this->Length2DSqr());
	}

	[[nodiscard]] constexpr float Length2DSqr() const
	{
		return (this->x * this->x + this->y * this->y);
	}

	[[nodiscard]] float DistTo(const Vector_t& vecEnd) const
	{
		return (*this - vecEnd).Length();
	}

	[[nodiscard]] float Dist(const Vector_t& vecEnd)
	{
		return (*this - vecEnd).Length();
	}
	// New methods to add
	[[nodiscard]] constexpr Vector_t Min(const Vector_t& other) const
	{
		return { std::min(this->x, other.x), std::min(this->y, other.y), std::min(this->z, other.z) };
	}

	[[nodiscard]] constexpr Vector_t Max(const Vector_t& other) const
	{
		return { std::max(this->x, other.x), std::max(this->y, other.y), std::max(this->z, other.z) };
	}

	[[nodiscard]] constexpr float DistToSqr(const Vector_t& vecEnd) const
	{
		return (*this - vecEnd).LengthSqr();
	}

	/// normalize magnitude of each component of the vector
	/// @returns: length of the vector
	float NormalizeInPlace()
	{
		const float flLength = this->Length();
		const float flRadius = 1.0f / (flLength + std::numeric_limits<float>::epsilon());

		this->x *= flRadius;
		this->y *= flRadius;
		this->z *= flRadius;

		return flLength;
	}

	/// normalize magnitude of each component of the vector
	/// @returns: copy of the vector with normalized components
	[[nodiscard]] Vector_t Normalized() const
	{
		Vector_t vecOut = *this;
		vecOut.NormalizeInPlace();
		return vecOut;
	}
	void clamp() {
		this->x = std::clamp(std::remainder(this->x, 180.0f), -89.0f, 89.0f);
		this->y = std::clamp(std::remainder(this->y, 360.0f), -180.0f, 180.0f);
		this->z = 0.f;
	}

	Vector_t clampVec()
	{
		if (x < -89.0f)
			x = -89.0f;

		if (x > 89.0f)
			x = 89.0f;

		while (y < -180.0f)
			y += 360.0f;

		while (y > 180.0f)
			y -= 360.0f;

		z = 0.0f;
		return *this;
	}

	Vector_t clampVec3(Vector_t value, Vector_t min, Vector_t max) {
		return Vector_t(
			std::clamp(value.x, min.x, max.x),
			std::clamp(value.y, min.y, max.y),
			std::clamp(value.z, min.z, max.z)
		);
	}

	void Normalize() {
			this->x = std::remainderf(this->x, 180.f);
			this->y = std::remainderf(this->y, 360.f);
	}

	__forceinline float length_sqr() const {
		return ((x * x) + (y * y) + (z * z));
	}

	__forceinline float length_2d_sqr() const {
		return ((x * x) + (y * y));
	}

	__forceinline float length() const {
		return std::sqrt(length_sqr());
	}


	__forceinline float normalize() {
		float len = this->length();

		(*this) /= (length() + std::numeric_limits< float >::epsilon());

		return len;
	}

	Vector_t normalizess() {
		float length = this->length();

		return *this / length;
	}

	__forceinline Vector_t normalized2() const
	{
		Vector_t res = *this;
		float l = res.length();

		if (l)  //-V550
			res /= l;
		else
			res.x = res.y = res.z = 0.0f;

		return res;
	}

	__forceinline Vector_t normalized() const {
		auto vec = *this;

		vec.normalize();

		return vec;
	}
	Vector_t Normalizes() const
	{
		Vector_t vecOut = *this;
		vecOut.NormalizeInPlace();
		return vecOut;
	}
	[[nodiscarrd]] inline float Dot(const Vector_t& vOther) const
	{
		const Vector_t& a = *this;

		return (a.x * vOther.x + a.y * vOther.y + a.z * vOther.z);
	}

	[[nodiscarrd]] inline float Dot(const float* fOther) const
	{
		const Vector_t& a = *this;

		return (a.x * fOther[0] + a.y * fOther[1] + a.z * fOther[2]);
	}
	[[nodiscard]] constexpr float DotProduct(const Vector_t& vecDot) const
	{
		return (this->x * vecDot.x + this->y * vecDot.y + this->z * vecDot.z);
	}

	[[nodiscard]] constexpr Vector_t CrossProduct(const Vector_t& vecCross) const
	{
		return { this->y * vecCross.z - this->z * vecCross.y, this->z * vecCross.x - this->x * vecCross.z, this->x * vecCross.y - this->y * vecCross.x };
	}

	/// @returns: transformed vector by given transformation matrix
	[[nodiscard]] Vector_t Transform(const Matrix3x4_t& matTransform) const;

	[[nodiscard]] Vector2D_t ToVector2D() const
	{
		return { this->x, this->y };
	}

	/// convert forward direction vector to other direction vectors
	/// @param[out] pvecRight [optional] output for converted right vector
	/// @param[out] pvecUp [optional] output for converted up vector
	void ToDirections(Vector_t* pvecRight, Vector_t* pvecUp) const
	{
		if (std::fabsf(this->x) < 1e-6f && std::fabsf(this->y) < 1e-6f)
		{
			// pitch 90 degrees up/down from identity
			if (pvecRight != nullptr)
			{
				pvecRight->x = 0.0f;
				pvecRight->y = -1.0f;
				pvecRight->z = 0.0f;
			}

			if (pvecUp != nullptr)
			{
				pvecUp->x = -this->z;
				pvecUp->y = 0.0f;
				pvecUp->z = 0.0f;
			}
		}
		else
		{
			if (pvecRight != nullptr)
			{
				pvecRight->x = this->y;
				pvecRight->y = -this->x;
				pvecRight->z = 0.0f;
				pvecRight->NormalizeInPlace();
			}

			if (pvecUp != nullptr)
			{
				pvecUp->x = (-this->x) * this->z;
				pvecUp->y = -(this->y * this->z);
				pvecUp->z = this->y * this->y - (-this->x) * this->x;
				pvecUp->NormalizeInPlace();
			}
		}
	}

	/// @returns: 2D angles converted from direction vector
	[[nodiscard]] QAngle_t ToAngles() const;
	[[nodiscard]] QAngle_t ToEulerAngles(Vector_t* pseudoup /*= nullptr*/);

	/// @returns: matrix converted from forward direction vector
	[[nodiscard]] Matrix3x4_t ToMatrix() const;

	float x = 0.0f, y = 0.0f, z = 0.0f;
};

struct Vector4D_t
{
	constexpr Vector4D_t(const float x = 0.0f, const float y = 0.0f, const float z = 0.0f, const float w = 0.0f) :
		x(x), y(y), z(z), w(w) { }

	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
};

struct alignas(16) VectorAligned_t : Vector_t
{
	VectorAligned_t() = default;

	explicit VectorAligned_t(const Vector_t& vecBase)
	{
		this->x = vecBase.x;
		this->y = vecBase.y;
		this->z = vecBase.z;
		this->w = 0.0f;
	}

	constexpr VectorAligned_t& operator=(const Vector_t& vecBase)
	{
		this->x = vecBase.x;
		this->y = vecBase.y;
		this->z = vecBase.z;
		this->w = 0.0f;
		return *this;
	}

	float w = 0.0f;
};

static_assert(alignof(VectorAligned_t) == 16);

struct alignas(16) bone_data2 {
	Vector_t pos;
	float scale;
	Vector4D_t rot;
};

