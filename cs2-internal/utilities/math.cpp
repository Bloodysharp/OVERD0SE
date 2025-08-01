#include "math.h"
#include <DirectXMath.h>
#include "memory.h"

#include "../core/sdk.h"
#include "../imgui/imgui_internal.h"

bool MATH::Setup()
{
	bool bSuccess = true;

	const void* hTier0Lib = MEM::GetModuleBaseHandle(TIER0_DLL);
	if (hTier0Lib == nullptr)
		return false;

	fnRandomSeed = reinterpret_cast<decltype(fnRandomSeed)>(MEM::GetExportAddress(hTier0Lib, CXOR("RandomSeed")));
	bSuccess &= (fnRandomSeed != nullptr);

	fnRandomFloat = reinterpret_cast<decltype(fnRandomFloat)>(MEM::GetExportAddress(hTier0Lib, CXOR("RandomFloat")));
	bSuccess &= (fnRandomFloat != nullptr);

	fnRandomFloatExp = reinterpret_cast<decltype(fnRandomFloatExp)>(MEM::GetExportAddress(hTier0Lib, CXOR("RandomFloatExp")));
	bSuccess &= (fnRandomFloatExp != nullptr);

	fnRandomInt = reinterpret_cast<decltype(fnRandomInt)>(MEM::GetExportAddress(hTier0Lib, CXOR("RandomInt")));
	bSuccess &= (fnRandomInt != nullptr);

	fnRandomGaussianFloat = reinterpret_cast<decltype(fnRandomGaussianFloat)>(MEM::GetExportAddress(hTier0Lib, CXOR("RandomGaussianFloat")));
	bSuccess &= (fnRandomGaussianFloat != nullptr);

	return bSuccess;
}

// distance between to line segments
float MATH::segment_dist(Vector_t start1, Vector_t end1, Vector_t start2, Vector_t end2) noexcept {
    Vector_t u = end1 - start1;
    Vector_t v = end2 - start2;
    Vector_t w = start1 - start2;

    float a = u.DotProduct(u);
    float b = u.DotProduct(v);
    float c = v.DotProduct(v);
    float d = u.DotProduct(w);
    float e = v.DotProduct(w);
    float D = a * c - b * b;
    float sc, sN, sD = D;
    float tc, tN, tD = D;

    if (D < 0.001f) {
        sN = 0.0f;
        sD = 1.0f;
        tN = e;
        tD = c;
    }
    else {
        sN = (b * e - c * d);
        tN = (a * e - b * d);
        if (sN < 0.0f) {
            sN = 0.0f;
            tN = e;
            tD = c;
        }
        else if (sN > sD) {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0f) {
        tN = 0.0f;

        if (-d < 0.0f) {
            sN = 0.0f;
        }
        else if (-d > a) {
            sN = sD;
        }
        else {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD) {
        tN = tD;

        if ((-d + b) < 0.0f) {
            sN = 0;
        }
        else if ((-d + b) > a) {
            sN = sD;
        }
        else {
            sN = (-d + b);
            sD = a;
        }
    }

    sc = (std::abs(sN) < 0.001f ? 0.0f : sN / sD);
    tc = (std::abs(tN) < 0.001f ? 0.0f : tN / tD);

    Vector_t dP = w + (u * sc) - (v * tc);
    return dP.Length();
}

void MATH::rotate_triangle(std::array<ImVec2, 3>& points, float rotation)
{
	const auto pointsCenter = (points.at(0) + points.at(1) + points.at(2)) / 3;
	for (auto& point : points)
	{
		point -= pointsCenter;

		const auto tempX = point.x;
		const auto tempY = point.y;

		const auto theta = M_DEG2RAD(rotation);
		const auto c = cos(theta);
		const auto s = sin(theta);

		point.x = tempX * c - tempY * s;
		point.y = tempX * s + tempY * c;

		point += pointsCenter;
	}
}
void MATH::TransformAABB(const Matrix3x4a_t& transform, const Vector_t& minsIn, const Vector_t& maxsIn, Vector_t& minsOut, Vector_t& maxsOut) {
	const Vector_t localCenter = (minsIn + maxsIn) * 0.5f;
	const Vector_t& localExtent = maxsIn - localCenter;

	const auto& mat = transform.arrData;
	const Vector_t worldAxisX{ mat[0][0], mat[0][1], mat[0][2] };
	const Vector_t worldAxisY{ mat[1][0], mat[1][1], mat[1][2] };
	const Vector_t worldAxisZ{ mat[2][0], mat[2][1], mat[2][2] };

	const Vector_t worldCenter = localCenter.Transform(transform);
	const Vector_t worldExtent{
		localExtent.DotProduct(worldAxisX),
		localExtent.DotProduct(worldAxisY),
		localExtent.DotProduct(worldAxisZ),
	};

	minsOut = worldCenter - worldExtent;
	maxsOut = worldCenter + worldExtent;
}
#include <numbers>

void MATH::angle_vector(const QAngle_t& angles, Vector_t& forward) noexcept {
	const float x = angles.x * std::numbers::pi_v<float> / 180.f;
	const float y = angles.y * std::numbers::pi_v<float> / 180.f;
	const float sp = std::sin(x);
	const float cp = std::cos(x);
	const float sy = std::sin(y);
	const float cy = std::cos(y);
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

Vector_t MATH::angle_vector(const QAngle_t& angles) noexcept {
	Vector_t forward;
	angle_vector(angles, forward);
	return forward;
}

void  MATH::new_vector_angles(const Vector_t& forward, Vector_t& angles)
{
	Vector_t view;

	if (!forward[0] && !forward[1])
	{
		view[0] = 0.0f;
		view[1] = 0.0f;
	}
	else
	{
		view[1] = atan2(forward[1], forward[0]) * 180.0f / k_pi;

		if (view[1] < 0.0f)
			view[1] += 360.0f;

		view[2] = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		view[0] = atan2(forward[2], view[2]) * 180.0f / k_pi;
	}

	angles[0] = -view[0];
	angles[1] = view[1];
	angles[2] = 0.f;
}

void MATH::normalize_angles(Vector_t& angles)
{
	while (angles.x > 89.0f)
		angles.x -= 180.0f;

	while (angles.x < -89.0f)
		angles.x += 180.0f;

	while (angles.y < -180.0f)
		angles.y += 360.0f;

	while (angles.y > 180.0f)
		angles.y -= 360.0f;
}

QAngle_t MATH::angle_normalize(QAngle_t& angles)
{
	while (angles.x > 89.0f)
		angles.x -= 180.0f;
	while (angles.x < -89.0f)
		angles.x += 180.0f;
	while (angles.y > 180.0f)
		angles.y -= 360.0f;
	while (angles.y < -180.0f)
		angles.y += 360.0f;
	angles.z = 0.0f;

	return angles;
}

float MATH::AngleNormalize(float angle) {
	angle = fmodf(angle, 360.0f);
	if (angle > 180)
	{
		angle -= 360;
	}
	if (angle < -180)
	{
		angle += 360;
	}
	return angle;
}

float MATH::AngleDiff(float next, float cur) {
	float delta = next - cur;

	if (delta < -180)
		delta += 360;
	else if (delta > 180)
		delta -= 360;

	return delta;
}

void MATH::angle_vectores(const QAngle_t& angles, QAngle_t& forward, QAngle_t& right, QAngle_t& up)
{
	float sr, sp, sy, cr, cp, cy;

	DirectX::XMScalarSinCos(&sp, &cp, deg2rad(angles[0]));
	DirectX::XMScalarSinCos(&sy, &cy, deg2rad(angles[1]));
	DirectX::XMScalarSinCos(&sr, &cr, deg2rad(angles[2]));

	forward.x = (cp * cy);
	forward.y = (cp * sy);
	forward.z = (-sp);
	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr * cy);
	right.z = (-1 * sr * cp);
	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = (cr * cp);
}

void MATH::angle_vectors(const Vector_t& angles, Vector_t& forward)
{
	float sp, sy, cp, cy;

	DirectX::XMScalarSinCos(&sp, &cp, deg2rad(angles[0]));
	DirectX::XMScalarSinCos(&sy, &cy, deg2rad(angles[1]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void MATH::super_fast_rsqrt(float a, float* out)
{
	const auto xx = _mm_load_ss(&a);
	auto xr = _mm_rsqrt_ss(xx);
	auto xt = _mm_mul_ss(xr, xr);
	xt = _mm_mul_ss(xt, xx);
	xt = _mm_sub_ss(_mm_set_ss(3.f), xt);
	xt = _mm_mul_ss(xt, _mm_set_ss(0.5f));
	xr = _mm_mul_ss(xr, xt);
	_mm_store_ss(out, xr);
}

float MATH::super_fast_vec_normalize(Vector_t& vec)
{
	const auto sqrlen = vec.LengthSqr() + 1.0e-10f;
	float invlen;
	super_fast_rsqrt(sqrlen, &invlen);
	vec.x *= invlen;
	vec.y *= invlen;
	vec.z *= invlen;
	return sqrlen * invlen;
}

void MATH::vector_angles_gomo(const Vector_t& forward, Vector_t& angles)
{
	Vector_t view;

	if (!forward[0] && !forward[1])
	{
		view[0] = 0.0f;
		view[1] = 0.0f;
	}
	else
	{
		view[1] = atan2(forward[1], forward[0]) * 180.0f / m_rad_pi;

		if (view[1] < 0.0f)
			view[1] += 360.0f;

		view[2] = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		view[0] = atan2(forward[2], view[2]) * 180.0f / m_rad_pi;
	}

	angles[0] = -view[0];
	angles[1] = view[1];
	angles[2] = 0.f;
}

void MATH::angle_vectors_second(const Vector_t& angles, Vector_t& forward)
{
	float sp, sy, cp, cy;

	sy = sin(M_DEG2RAD(angles[1]));
	cy = cos(M_DEG2RAD(angles[1]));

	sp = sin(M_DEG2RAD(angles[0]));
	cp = cos(M_DEG2RAD(angles[0]));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void MATH::new_angle_vectors(const Vector_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up)
{
	auto sin_cos = [](float radian, float* sin, float* cos)
		{
			*sin = std::sin(radian);
			*cos = std::cos(radian);
		};

	float sp, sy, sr, cp, cy, cr;

	sin_cos(angles.x * 0.017453292f, &sp, &cp);
	sin_cos(angles.y * 0.017453292f, &sy, &cy);
	sin_cos(angles.z * 0.017453292f, &sr, &cr);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = sr * sp * cy - cr * sy;
		right->y = cr * cy + sr * sp * sy;
		right->z = sr * cp;
	}

	if (up)
	{
		up->x = sr * sy + cr * sp * cy;
		up->y = cr * sp * sy - sr * cy;
		up->z = cr * cp;
	}
}

Vector_t MATH::new_calculate_angle(const Vector_t& src, const Vector_t& dst) {
	Vector_t angles;

	Vector_t delta = src - dst;
	float hyp = delta.Length2D();

	angles.y = std::atanf(delta.y / delta.x) * m_rad_pi;
	angles.x = std::atanf(-delta.z / hyp) * -m_rad_pi;
	angles.z = 0.0f;

	if (delta.x >= 0.0f)
		angles.y += 180.0f;

	return angles;
}

QAngle_t MATH::CalcAngles(Vector_t viewPos, Vector_t aimPos)
{
	QAngle_t angle = { 0, 0, 0 };

	Vector_t delta = aimPos - viewPos;

	angle.x = -asin(delta.z / delta.Length()) * (180.0f / 3.141592654f);
	angle.y = atan2(delta.y, delta.x) * (180.0f / 3.141592654f);

	return angle;
}

void MATH::anglevectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up )
{
	float cp = std::cos(M_DEG2RAD(angles.x)), sp = std::sin(M_DEG2RAD(angles.x));
	float cy = std::cos(M_DEG2RAD(angles.y)), sy = std::sin(M_DEG2RAD(angles.y));
	float cr = std::cos(M_DEG2RAD(angles.z)), sr = std::sin(M_DEG2RAD(angles.z));

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right) {
		right->x = -1.f * sr * sp * cy + -1.f * cr * -sy;
		right->y = -1.f * sr * sp * sy + -1.f * cr * cy;
		right->z = -1.f * sr * cp;
	}

	if (up) {
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}

void MATH::VectorAngless(const Vector_t& forward, QAngle_t& angles, Vector_t* up) {
	Vector_t  left;
	float   len, up_z, pitch, yaw, roll;

	// get 2d length.
	len = forward.Length2D();

	if (up && len > 0.001f) {
		pitch = M_RAD2DEG(std::atan2(-forward.z, len));
		yaw = M_RAD2DEG(std::atan2(forward.y, forward.x));

		// get left direction vector using cross product.
		left = (*up).CrossProduct(forward).Normalized();

		// calculate up_z.
		up_z = (left.y * forward.x) - (left.x * forward.y);

		// calculate roll.
		roll = M_RAD2DEG(std::atan2(left.z, up_z));
	}

	else {
		if (len > 0.f) {
			// calculate pitch and yaw.
			pitch = M_RAD2DEG(std::atan2(-forward.z, len));
			yaw = M_RAD2DEG(std::atan2(forward.y, forward.x));
			roll = 0.f;
		}

		else {
			pitch = (forward.z > 0.f) ? -90.f : 90.f;
			yaw = 0.f;
			roll = 0.f;
		}
	}

	// set out angles.
	angles = { pitch, yaw, roll };
}

Vector_t MATH::angle_vectors_fs(const Vector_t& angles)
{
	float	sp, sy, cp, cy;

	DirectX::XMScalarSinCos(&sp, &cp, deg2rad(angles.x));
	DirectX::XMScalarSinCos(&sy, &cy, deg2rad(angles.y));

	return Vector_t(
		cp * cy,
		cp * sy,
		-sp
	);
}

void MATH::angle_vectors_fs(Vector_t& angles, Vector_t& forward)
{
	float sp, sy, cp, cy;

	DirectX::XMScalarSinCos(&sp, &cp, deg2rad(angles.x));
	DirectX::XMScalarSinCos(&sy, &cy, deg2rad(angles.y));

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

bool MATH::world_to_screen(const Vector_t& in, Vector_t& out, ImVec2 screen) {
	if (!ImGui::GetCurrentContext()) return false;

	const auto width = SDK::ViewMatrix[3][0] * in.x + SDK::ViewMatrix[3][1] * in.y +
		SDK::ViewMatrix[3][2] * in.z + SDK::ViewMatrix[3][3];

	if (width < 0.001f)
		return false;

	const auto inverse = 1.f / width;

	out.x = (SDK::ViewMatrix[0][0] * in.x + SDK::ViewMatrix[0][1] * in.y
		+ SDK::ViewMatrix[0][2] * in.z + SDK::ViewMatrix[0][3]) * inverse;

	out.y = (SDK::ViewMatrix[1][0] * in.x + SDK::ViewMatrix[1][1] * in.y
		+ SDK::ViewMatrix[1][2] * in.z + SDK::ViewMatrix[1][3]) * inverse;

	out.x = (screen.x * 0.5f) + (out.x * screen.x) * 0.5f;
	out.y = (screen.y * 0.5f) - (out.y * screen.y) * 0.5f;

	return true;
}

bool MATH::world_screen(Vector_t& in, Vector_t& out, ImVec2 screen)
{
	static auto screen_transofrm = reinterpret_cast<bool(CS_FASTCALL*)(Vector_t&, Vector_t&)>(MEM::FindPattern(CLIENT_DLL, CXOR("48 89 74 24 ? 57 48 83 EC 20 48 8B 05 ? ? ? ? 48 8B FA")));

	if (!screen_transofrm)
		return false;

	bool on_screen = screen_transofrm(in, out);
	if (on_screen)
		return false;

	const auto& screen_size = ImGui::GetIO().DisplaySize;

	const float screen_size_x = screen.x;
	const float screen_size_y = screen.y;

	out.x = ((out.x + 1.0f) * 0.5f) * screen_size_x;
	out.y = screen_size_y - (((out.y + 1.0f) * 0.5f) * screen_size_y);

	return true;

}

bool MATH::WorldToScreen(const Vector_t& in, ImVec2& out) {
	if (!ImGui::GetCurrentContext()) return false;

	auto z = SDK::ViewMatrix[3][0] * in.x + SDK::ViewMatrix[3][1] * in.y +
		SDK::ViewMatrix[3][2] * in.z + SDK::ViewMatrix[3][3];

	if (z < 0.001f)
		return false;

	out = { (ImGui::GetIO().DisplaySize.x * 0.5f), (ImGui::GetIO().DisplaySize.y * 0.5f) };
	out.x *= 1.0f + (SDK::ViewMatrix[0][0] * in.x + SDK::ViewMatrix[0][1] * in.y +
		SDK::ViewMatrix[0][2] * in.z + SDK::ViewMatrix[0][3]) /
		z;
	out.y *= 1.0f - (SDK::ViewMatrix[1][0] * in.x + SDK::ViewMatrix[1][1] * in.y +
		SDK::ViewMatrix[1][2] * in.z + SDK::ViewMatrix[1][3]) /
		z;

	out = { out.x, out.y };
	return true;
}