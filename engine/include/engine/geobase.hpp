#pragma once

#include <glm/glm.hpp>

#define RAY_CAST_EPS 0.00001f

namespace Morpheus {
	struct Ray;
	struct Frustum;
	struct Plane;
	struct BoundingBox;
	struct Triangle;

	struct RayIntersection {
		float mDistance;
		glm::vec3 mLocation;
	};

	struct Ray {
		glm::vec3 mStart;
		glm::vec3 mDirection;
		float mtMax;

		template <typename T>
		inline bool intersect(const T& obj, RayIntersection* intersection) const {
			intersection->mDistance = 0.0f;
			bool bResult = obj.intersect(*this, &intersection->mDistance);
			intersection->mLocation = mStart + dist * mDirection;
			return bResult;
		}

		template <typename T>
		inline bool intersect(const T& obj) const {
			return obj.intersect(*this);
		}
	};

	struct Triangle {
		glm::vec3 mV1;
		glm::vec3 mV2;
		glm::vec3 mV3;

		inline BoundingBox computeBoundingBox() const {
			BoundingBox bb(mV1, mV1);
			bb.mergeInPlace(mV2);
			bb.mergeInPlace(mV3);
			return bb;
		}
	};

	struct Frustum {
		Plane mPlanes[6];
		glm::vec3 mPoints[8];

		bool intersect(const BoundingBox& bb) const;
	};

	struct Plane {
		glm::vec4 mSeparator;

		inline Plane() {}
		inline Plane(const glm::vec3& normal, float distance) {
			mSeparator.x = normal.x;
			mSeparator.y = normal.y;
			mSeparator.z = normal.z;
			mSeparator.w = -distance;
		}

		inline Plane(const glm::vec4& separator) : mSeparator(separator) {
		}

		inline float eval(const glm::vec3& v) {
			return glm::dot(mSeparator, glm::vec4(v, 1.0f));
		}

		inline float eval(const glm::vec4& v) {
			return glm::dot(mSeparator, v);
		}

		inline float eval(const float x, const float y, const float z) {
			return mSeparator.x * x + mSeparator.y * y + mSeparator.z * z + mSeparator.w;
		}

		inline float eval(const float x, const float y, const float z, const float w) {
			return mSeparator.x * x + mSeparator.y * y + mSeparator.z * z + mSeparator.w * w;
		}

		// Make a plane from three points
		inline Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
			glm::vec3 e12 = p2 - p1;
			glm::vec3 e13 = p3 - p1;
			glm::vec3 normal = glm::normalize(glm::cross(e12, e13));
			float distance = glm::dot(p1, normal);
			mSeparator = glm::vec4(normal, -distance);
		}

		inline Plane(const Triangle& tri) : Plane(tri.mV1, tri.mV2, tri.mV3) {
		}

		bool intersect(const Ray& ray, float* hitt = nullptr) const;
	};

	struct BoundingBox {
		glm::vec3 mLower;
		glm::vec3 mUpper;

		inline BoundingBox() {}
		inline BoundingBox(const glm::vec3& lower, const glm::vec3& upper) : mLower(lower),
			mUpper(upper) {	
		}

		inline BoundingBox merge(const BoundingBox& bb) {
			BoundingBox result;
			result.mLower = glm::min(mLower, bb.mLower);
			result.mUpper = glm::max(mUpper, bb.mUpper);
			return result;
		}

		inline BoundingBox merge(const glm::vec3& v) {
			BoundingBox result;
			result.mLower = glm::min(mLower, v);
			result.mUpper = glm::max(mUpper, v);
			return result;
		}

		inline void mergeInPlace(const BoundingBox&& bb) {
			mLower = glm::min(mLower, bb.mLower);
			mUpper = glm::max(mUpper, bb.mUpper);
		}

		inline void mergeInPlace(const glm::vec3& v) {
			mLower = glm::min(mLower, v);
			mUpper = glm::max(mUpper, v);
		}

		inline static BoundingBox empty() {
			auto lower = std::numeric_limits<float>::infinity();
			auto upper = -std::numeric_limits<float>::infinity();

			return BoundingBox(glm::vec3(lower, lower, lower), glm::vec3(upper, upper, upper));
		}

		inline bool isEmpty() const {
			return mLower.x > mUpper.x ||
				mLower.y > mUpper.y ||
				mLower.z > mUpper.z;
		}

		bool intersect(const Ray& ray, float* hitt0 = nullptr, float* hitt1 = nullptr) const;
	};
}