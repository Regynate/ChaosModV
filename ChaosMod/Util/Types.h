#pragma once

#include "Natives.h"

struct ChaosVector3
{
	float x, y, z;
	explicit ChaosVector3(const Vector3 &vec) : x(vec.x), y(vec.y), z(vec.z)
	{
	}

	ChaosVector3(float x, float y, float z) : x(x), y(y), z(z)
	{
	}

	ChaosVector3 operator+(const ChaosVector3& other)
	{
		return ChaosVector3(x + other.x, y + other.y, z + other.z);
	}
};

struct ChaosVector2
{
	float x, y;
};

struct ChaosVector4
{
	float x, y, z, w;

	bool operator==(ChaosVector4 other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	void operator=(ChaosVector4 other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
	}

	ChaosVector4 operator*(const float mul) const
	{
		return { x * mul, y * mul, z * mul, w * mul };
	}

	ChaosVector4 operator+(const ChaosVector4 other) const
	{
		return { x + other.x, y + other.y, z + other.z, w + other.w };
	}

	ChaosVector4(const ChaosVector3 vec) : x(vec.x), y(vec.y), z(vec.z), w(0)
	{
	}

	ChaosVector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
	{
	}

	ChaosVector4() : x(0), y(0), z(0), w(0)
	{
	}

	bool IsDefault()
	{
		return x == 0 && y == 0 && z == 0 && w == 0;
	}

	float scalar(const ChaosVector4 other) const
	{
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}
};

struct ChaosMatrix4x4
{
	ChaosVector4 right;
	ChaosVector4 forward;
	ChaosVector4 up;
	ChaosVector4 translation;

	void operator=(ChaosMatrix4x4 other)
	{
		right       = other.right;
		forward     = other.forward;
		up          = other.up;
		translation = other.translation;
	}

	ChaosMatrix4x4 operator*(ChaosVector4 vec) const
	{
		return {
			right * vec.x,
			forward * vec.y,
			up * vec.z,
			translation * vec.w,
		};
	}

	ChaosMatrix4x4(ChaosVector4 right, ChaosVector4 forward, ChaosVector4 up, ChaosVector4 translation)
	    : right(right), forward(forward), up(up), translation(translation)
	{
	}

	ChaosMatrix4x4() : right(), forward(), up(), translation()
	{
	}

	bool operator==(const ChaosMatrix4x4 other) const
	{
		LOG((right == other.right) << " " << (forward == other.forward) << " " << (up == other.up) << " " << (translation
		    == other.translation));

		return right == other.right && forward == other.forward && up == other.up && translation == other.translation;
	}

	bool IsDefault()
	{
		return right.IsDefault() && forward.IsDefault() && up.IsDefault() && translation.IsDefault();
	}

	ChaosVector4 col1()
	{
		return { right.x, forward.x, up.x, translation.x };
	}

	ChaosVector4 col2()
	{
		return { right.y, forward.y, up.y, translation.y };
	}

	ChaosVector4 col3()
	{
		return { right.z, forward.z, up.z, translation.z };
	}

	ChaosVector4 col4()
	{
		return { right.w, forward.w, up.w, translation.w };
	}

	// clang-format off
	ChaosMatrix4x4 operator*(ChaosMatrix4x4 other) const
	{
		return {
			right.scalar(other.col1()), right.scalar(other.col2()), right.scalar(other.col3()), other.right.w,
			forward.scalar(other.col1()), forward.scalar(other.col2()), forward.scalar(other.col3()), other.forward.w,
			up.scalar(other.col1()), up.scalar(other.col2()), up.scalar(other.col3()), other.up.w,
			translation.scalar(other.col1()), translation.scalar(other.col2()), translation.scalar(other.col3()), other.translation.w
		};
	}

	ChaosMatrix4x4(float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float x3, float y3,
		float z3, float w3, float x4, float y4, float z4, float w4)
	    : right(x1, y1, z1, w1), forward(x2, y2, z2, w2), up(x3, y3, z3, w3), translation(x4, y4, z4, w4)
		{
		}
		
	static ChaosMatrix4x4 TranslationMatrix(ChaosVector3 translation)
	{
		return
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			translation.x, translation.y, translation.z, 1
		};
	}
	
	static ChaosMatrix4x4 ScaleMatrix(ChaosVector3 scale)
	{
		return
		{
			scale.x, 0, 0, 0,
			0, scale.y, 0, 0,
			0, 0, scale.z, 0,
			0, 0, 0, 1
		};
	}

	static ChaosMatrix4x4 RotationMatrix(ChaosVector3 rotation)
	{
		return
		ChaosMatrix4x4 {
			1, 0, 0, 0,
			0, cos(rotation.x), -sin(rotation.x), 0,
			0, sin(rotation.x), cos(rotation.x), 0,
			0, 0, 0, 1
		} *
		ChaosMatrix4x4 {
			cos(rotation.y), 0, sin(rotation.y), 0,
			0, 1, 0, 0,
			-sin(rotation.y), 0, cos(rotation.y), 0,
			0, 0, 0, 1
		} *
		ChaosMatrix4x4 {
			cos(rotation.z), sin(rotation.z), 0, 0,
			-sin(rotation.z), cos(rotation.z), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
	}

	void Log() const {
		LOG(std::fixed << std::setprecision(3) << right.x << " " << right.y << " " << right.z << " " << right.w << " | " <<
			forward.x << " " << forward.y << " " << forward.z << " " << forward.w << " | " <<
			up.x << " " << up.y << " " << up.z << " " << up.w << " | " <<
			translation.x << " " << translation.y << " " << translation.z << " " << translation.w);
	}
	// clang-format on
};

struct ChaosMatrix4x3
{
	ChaosVector4 right;
	ChaosVector4 forward;
	ChaosVector4 up;

	void operator=(ChaosMatrix4x3 other)
	{
		forward = other.forward;
		up      = other.up;
		right   = other.right;
	}

	ChaosMatrix4x3 operator*(ChaosVector4 vec) const
	{
		return { right * vec.x, forward * vec.y, up * vec.z };
	}
};

inline Vector3 GetCoordAround(Entity entity, float angle, float radius, float zOffset, bool relative)
{
	if (relative)
	{
		Vector3 offset = Vector3::Init(-radius * SIN(angle + 90), radius * SIN(angle), zOffset);

		return GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(entity, offset.x, offset.y, offset.z);
	}
	else
	{
		Vector3 entityPosition = GET_ENTITY_COORDS(entity, false);
		return Vector3::Init(entityPosition.x - radius * SIN(angle + 90), entityPosition.y + radius * SIN(angle),
		                     entityPosition.z + zOffset);
	}
}