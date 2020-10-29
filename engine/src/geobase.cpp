#include <engine/geobase.hpp>

namespace Morpheus {
	bool Frustum::intersect(const BoundingBox& bb) const {

		// check box outside/inside of frustum
		for( int i=0; i<6; i++ )
		{
			int out = 0;
			out += glm::dot(mPlanes[i].mSeparator, glm::vec4(bb.mLower.x, bb.mLower.y, bb.mLower.z, 1.0f)) < 0.0 ? 1:0;
			out += glm::dot(mPlanes[i].mSeparator, glm::vec4(bb.mUpper.x, bb.mLower.y, bb.mLower.z, 1.0f)) < 0.0 ? 1:0;
			out += glm::dot(mPlanes[i].mSeparator, glm::vec4(bb.mLower.x, bb.mUpper.y, bb.mLower.z, 1.0f)) < 0.0 ? 1:0;
			out += glm::dot(mPlanes[i].mSeparator, glm::vec4(bb.mUpper.x, bb.mUpper.y, bb.mLower.z, 1.0f)) < 0.0 ? 1:0;
			out += glm::dot(mPlanes[i].mSeparator, glm::vec4(bb.mLower.x, bb.mLower.y, bb.mUpper.z, 1.0f)) < 0.0 ? 1:0;
			out += glm::dot(mPlanes[i].mSeparator, glm::vec4(bb.mUpper.x, bb.mLower.y, bb.mUpper.z, 1.0f)) < 0.0 ? 1:0;
			out += glm::dot(mPlanes[i].mSeparator, glm::vec4(bb.mLower.x, bb.mUpper.y, bb.mUpper.z, 1.0f)) < 0.0 ? 1:0;
			out += glm::dot(mPlanes[i].mSeparator, glm::vec4(bb.mUpper.x, bb.mUpper.y, bb.mUpper.z, 1.0f)) < 0.0 ? 1:0;
			if( out==8 ) return false;
		}

		// check frustum outside/inside box
		int out;
		out=0; for( int i=0; i<8; i++ ) out += ((mPoints[i].x > bb.mUpper.x)?1:0); if( out==8 ) return false;
		out=0; for( int i=0; i<8; i++ ) out += ((mPoints[i].x < bb.mLower.x)?1:0); if( out==8 ) return false;
		out=0; for( int i=0; i<8; i++ ) out += ((mPoints[i].y > bb.mUpper.y)?1:0); if( out==8 ) return false;
		out=0; for( int i=0; i<8; i++ ) out += ((mPoints[i].y < bb.mLower.y)?1:0); if( out==8 ) return false;
		out=0; for( int i=0; i<8; i++ ) out += ((mPoints[i].z > bb.mUpper.z)?1:0); if( out==8 ) return false;
		out=0; for( int i=0; i<8; i++ ) out += ((mPoints[i].z < bb.mLower.z)?1:0); if( out==8 ) return false;

		return true;
	}

	bool Plane::intersect(const Ray& ray, float* hitt) const {
		glm::vec3 normal(mSeparator);
		float dot_factor = glm::dot(ray.mDirection, normal);
		if (dot_factor < RAY_CAST_EPS && dot_factor > -RAY_CAST_EPS) {
			return false;
		}
		
		float t = (-mSeparator.w - glm::dot(ray.mStart, normal)) / dot_factor;
		bool bIntersect = t > 0.0 && t < ray.mtMax;
		if (hitt) *hitt = t;
		return bIntersect;
	}

	bool BoundingBox::intersect(const Ray& ray, float* hitt0, float* hitt1) const {
		float t0 = 0, t1 = ray.mtMax;
		for (int i = 0; i < 3; ++i) {
			// Update interval for _i_th bounding box slab
			float invRayDir = 1 / ray.mDirection[i];
			float tNear = (mLower[i] - ray.mStart[i]) * invRayDir;
			float tFar = (mUpper[i] - ray.mStart[i]) * invRayDir;

			// Update parametric interval from slab intersection $t$ values
			if (tNear > tFar) std::swap(tNear, tFar);

			// Update _tFar_ to ensure robust ray--bounds intersection
			tFar *= 1 + 2 * gamma(3);
			t0 = tNear > t0 ? tNear : t0;
			t1 = tFar < t1 ? tFar : t1;
			if (t0 > t1) return false;
		}
		if (hitt0) *hitt0 = t0;
		if (hitt1) *hitt1 = t1;
		return true;
	}
}