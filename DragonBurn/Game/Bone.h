#pragma once
#include <vector>
#include <optional>
#include <list>
#include "Game.h"
#include <cmath>
#include <unordered_map>

#undef M_PI
#define M_PI 3.14159265358979323846

// Original bone indices - keeping for compatibility
enum BONEINDEX : DWORD
{
	head = 6,
	neck_0 = 5,
	spine_0 = 1,
	spine_1 = 2,
	spine_2 = 3,
	spine_3 = 4,
	pelvis = 0,
	arm_upper_L = 8,
	arm_lower_L = 9,
	hand_L = 10,
	arm_upper_R = 13,
	arm_lower_R = 14,
	hand_R = 15,
	leg_upper_L = 22,
	leg_lower_L = 23,
	ankle_L = 24,
	leg_upper_R = 25,
	leg_lower_R = 26,
	ankle_R = 27,
};



class Quaternion_t
{
public:
	float x, y, z, w;

	// Constructors
	Quaternion_t() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	Quaternion_t(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	// Copy constructor
	Quaternion_t(const Quaternion_t& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

	// Assignment operator
	Quaternion_t& operator=(const Quaternion_t& other) {
		if (this != &other) {
			x = other.x;
			y = other.y;
			z = other.z;
			w = other.w;
		}
		return *this;
	}

	// Identity quaternion (no rotation)
	static Quaternion_t Identity() {
		return Quaternion_t(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// Create quaternion from Euler angles (in radians)
	static Quaternion_t FromEuler(float pitch, float yaw, float roll) {
		float cy = cosf(yaw * 0.5f);
		float sy = sinf(yaw * 0.5f);
		float cp = cosf(pitch * 0.5f);
		float sp = sinf(pitch * 0.5f);
		float cr = cosf(roll * 0.5f);
		float sr = sinf(roll * 0.5f);

		return Quaternion_t(
			sr * cp * cy - cr * sp * sy,  // x
			cr * sp * cy + sr * cp * sy,  // y
			cr * cp * sy - sr * sp * cy,  // z
			cr * cp * cy + sr * sp * sy   // w
		);
	}

	// Convert to Euler angles (in radians)
	void ToEuler(float& pitch, float& yaw, float& roll) const {
		// Roll (x-axis rotation)
		float sinr_cosp = 2 * (w * x + y * z);
		float cosr_cosp = 1 - 2 * (x * x + y * y);
		roll = atan2f(sinr_cosp, cosr_cosp);

		// Pitch (y-axis rotation)
		float sinp = 2 * (w * y - z * x);
		if (fabsf(sinp) >= 1)
			pitch = copysignf(M_PI / 2, sinp); // Use 90 degrees if out of range
		else
			pitch = asinf(sinp);

		// Yaw (z-axis rotation)
		float siny_cosp = 2 * (w * z + x * y);
		float cosy_cosp = 1 - 2 * (y * y + z * z);
		yaw = atan2f(siny_cosp, cosy_cosp);
	}

	// Normalize the quaternion
	void Normalize() {
		float length = sqrtf(x * x + y * y + z * z + w * w);
		if (length > 0.0f) {
			x /= length;
			y /= length;
			z /= length;
			w /= length;
		}
	}

	// Get normalized quaternion
	Quaternion_t Normalized() const {
		Quaternion_t result(*this);
		result.Normalize();
		return result;
	}

	// Quaternion multiplication
	Quaternion_t operator*(const Quaternion_t& other) const {
		return Quaternion_t(
			w * other.x + x * other.w + y * other.z - z * other.y,
			w * other.y - x * other.z + y * other.w + z * other.x,
			w * other.z + x * other.y - y * other.x + z * other.w,
			w * other.w - x * other.x - y * other.y - z * other.z
		);
	}

	// Conjugate (inverse for unit quaternions)
	Quaternion_t Conjugate() const {
		return Quaternion_t(-x, -y, -z, w);
	}

	// Dot product
	float Dot(const Quaternion_t& other) const {
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}

	// Linear interpolation
	static Quaternion_t Lerp(const Quaternion_t& a, const Quaternion_t& b, float t) {
		float dot = a.Dot(b);

		// If dot product is negative, slerp would take the long route.
		// To fix this, we can flip one of the quaternions.
		Quaternion_t b_corrected = (dot < 0.0f) ? Quaternion_t(-b.x, -b.y, -b.z, -b.w) : b;

		return Quaternion_t(
			a.x + t * (b_corrected.x - a.x),
			a.y + t * (b_corrected.y - a.y),
			a.z + t * (b_corrected.z - a.z),
			a.w + t * (b_corrected.w - a.w)
		).Normalized();
	}

	// Check if quaternion is valid (not NaN or infinite)
	bool IsValid() const {
		return !isnan(x) && !isnan(y) && !isnan(z) && !isnan(w) &&
			!isinf(x) && !isinf(y) && !isinf(z) && !isinf(w);
	}

	// Convert to string for debugging
	std::string ToString() const {
		return "Quaternion(" + std::to_string(x) + ", " + std::to_string(y) +
			", " + std::to_string(z) + ", " + std::to_string(w) + ")";
	}
};

struct CBoneData {
	Vec3 Location;
	float Scale;
	Quaternion_t Rotation;
};

struct BoneJointData {
	Vec3 Pos;
	float Scale;
	Quaternion_t Rotation;
};

struct BoneJointPos {
	Vec3 Pos;
	Vec2 ScreenPos;
	bool IsVisible = false;
};

// Vector transformation functions
namespace VectorTransform {
	// Cross product helper
	inline Vec3 Cross(const Vec3& a, const Vec3& b) {
		return Vec3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}

	// Vector addition helper
	inline Vec3 Add(const Vec3& a, const Vec3& b) {
		return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
	}

	// Vector subtraction helper
	inline Vec3 Subtract(const Vec3& a, const Vec3& b) {
		return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
	}

	// Scalar multiplication helper
	inline Vec3 Multiply(const Vec3& v, float scalar) {
		return Vec3(v.x * scalar, v.y * scalar, v.z * scalar);
	}

	// Distance helper
	inline float Distance(const Vec3& a, const Vec3& b) {
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		float dz = a.z - b.z;
		return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	// Rotation helper
	inline Vec3 Rotate(const Vec3& v, const Quaternion_t& q) {
		Vec3 qvec(q.x, q.y, q.z);
		Vec3 uv = Cross(qvec, v);
		Vec3 uuv = Cross(qvec, uv);
		uv = Multiply(uv, (2.0f * q.w));
		uuv = Multiply(uuv, 2.0f);
		return Add(Add(v, uv), uuv);
	}

	// Extension helper
	inline Vec3 Extend(const Vec3& start, const Vec3& end, float distance) {
		Vec3 dir = Subtract(end, start);
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
		if (len > 0.0f) {
			dir.x /= len;
			dir.y /= len;
			dir.z /= len;
			return Vec3(
				start.x + dir.x * distance,
				start.y + dir.y * distance,
				start.z + dir.z * distance
			);
		}
		return start;
	}
};

class c_model_state {
public:
	std::optional<CBoneData> GetBoneLocation(int index);
	std::optional<Vec3> GetBonePosition(int index);
};

class CHitBox {
public:
	CHitBox();
	CHitBox(CHitBox* hitbox);
	std::string Name();
	Vec3 GetStart();
	Vec3 GetEnd();
	float GetRadius();

private:
	char* name;
	char* unk1;
	char* unk2;
	Vec3 Start;
	Vec3 End;
	float Radius;
};

class CHitBoxes {
public:
	uint32_t GetHashName();
	uint32_t GetHitBoxCount();
	CHitBox* GetpHitBox(int index);
	CHitBox GetHitBox(int index);
};

class CRenderMesh {
public:
	CHitBoxes* GetHitboxes(int n);
	int GetHitBoxCount();
};

class CRenderMeshes {
public:
	CRenderMesh* GetRenderMesh(int index);
};

class CModel {
public:
	CRenderMeshes* GetMeshes();
	uint32_t GetMeshesCount();
	CHitBox* GetpHitBox(int index);
};

class c_game_scene_node {
public:
	c_model_state* GetBoneArray();
	CModel* GetModel();
};

class CBone {
private:


public:
	std::vector<BoneJointPos> BonePosList;
	std::vector<CBoneData> EnhancedBoneData;

	DWORD64 EntityPawnAddress = 0;
	c_game_scene_node* GameSceneNode = nullptr;

	bool UpdateAllBoneData(const DWORD64& EntityPawnAddress);
	std::optional<CBoneData> GetEnhancedBoneData(int index) const;
	bool UpdateAllBoneDataBatch(const DWORD64& EntityPawnAddress);
	CHitBox* GetHitBox(int index) const;
	std::optional<Vec3> GetBonePosition(int boneIndex);
	std::optional<Vec2> GetBoneScreenPosition(int boneIndex);
	bool IsBoneVisible(int boneIndex);
};

namespace BoneJointList {
	inline std::list<DWORD> Trunk = { neck_0, spine_3, spine_2, spine_1, spine_0, pelvis };
	inline std::list<DWORD> LeftArm = { neck_0, arm_upper_L, arm_lower_L, hand_L };
	inline std::list<DWORD> RightArm = { neck_0, arm_upper_R, arm_lower_R, hand_R };
	inline std::list<DWORD> LeftLeg = { pelvis, leg_upper_L, leg_lower_L, ankle_L };
	inline std::list<DWORD> RightLeg = { pelvis, leg_upper_R, leg_lower_R, ankle_R };
	inline std::vector<std::list<DWORD>> List = { Trunk, LeftArm, RightArm, LeftLeg, RightLeg };
}

// Hitbox drawing functions
int HitboxToBone(const std::string& box);
void CreateCircle(Vec3 point, Vec3 center, float radius, std::vector<Vec3>& vec, int segments = 12);

void DrawCapsule(Vec3 vMin, Vec3 vMax, float radius, Quaternion_t rotation, Vec3 origPos, ImColor color, int segments = 12, float thickness = 1.0f);
