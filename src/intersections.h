#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>

#include "sceneStructs.h"
#include "utilities.h"

/**
 * Handy-dandy hash function that provides seeds for random number generation.
 */
__host__ __device__ inline unsigned int utilhash(unsigned int a) {
	a = (a + 0x7ed55d16) + (a << 12);
	a = (a ^ 0xc761c23c) ^ (a >> 19);
	a = (a + 0x165667b1) + (a << 5);
	a = (a + 0xd3a2646c) ^ (a << 9);
	a = (a + 0xfd7046c5) + (a << 3);
	a = (a ^ 0xb55a4f09) ^ (a >> 16);
	return a;
}

// CHECKITOUT
/**
 * Compute a point at parameter value `t` on ray `r`.
 * Falls slightly short so that it doesn't intersect the object it's hitting.
 */
__host__ __device__ glm::vec3 getPointOnRay(Ray r, float t) {
	return r.origin + (t - .0001f) * glm::normalize(r.direction);
}

/**
 * Multiplies a mat4 and a vec4 and returns a vec3 clipped from the vec4.
 */
__host__ __device__ glm::vec3 multiplyMV(glm::mat4 m, glm::vec4 v) {
	return glm::vec3(m * v);
}

// CHECKITOUT
/**
 * Test intersection between a ray and a transformed cube. Untransformed,
 * the cube ranges from -0.5 to 0.5 in each axis and is centered at the origin.
 *
 * @param intersectionPoint  Output parameter for point of intersection.
 * @param normal             Output parameter for surface normal.
 * @param outside            Output param for whether the ray came from outside.
 * @return                   Ray parameter `t` value. -1 if no intersection.
 */
__host__ __device__ float boxIntersectionTest(Geom box, Ray r,
	glm::vec3& intersectionPoint, glm::vec3& normal, bool& outside) {
	Ray q;
	q.origin = multiplyMV(box.inverseTransform, glm::vec4(r.origin, 1.0f));
	q.direction = glm::normalize(multiplyMV(box.inverseTransform, glm::vec4(r.direction, 0.0f)));

	float tmin = -1e38f;
	float tmax = 1e38f;
	glm::vec3 tmin_n;
	glm::vec3 tmax_n;
	for (int xyz = 0; xyz < 3; ++xyz) {
		float qdxyz = q.direction[xyz];
		/*if (glm::abs(qdxyz) > 0.00001f)*/ {
			float t1 = (-0.5f - q.origin[xyz]) / qdxyz;
			float t2 = (+0.5f - q.origin[xyz]) / qdxyz;
			float ta = glm::min(t1, t2);
			float tb = glm::max(t1, t2);
			glm::vec3 n;
			n[xyz] = t2 < t1 ? +1 : -1;
			if (ta > 0 && ta > tmin) {
				tmin = ta;
				tmin_n = n;
			}
			if (tb < tmax) {
				tmax = tb;
				tmax_n = n;
			}
		}
	}

	if (tmax >= tmin && tmax > 0) {
		outside = true;
		if (tmin <= 0) {
			tmin = tmax;
			tmin_n = tmax_n;
			outside = false;
		}
		intersectionPoint = multiplyMV(box.transform, glm::vec4(getPointOnRay(q, tmin), 1.0f));
		normal = glm::normalize(multiplyMV(box.invTranspose, glm::vec4(tmin_n, 0.0f)));
		return glm::length(r.origin - intersectionPoint);
	}
	return -1;
}


// CHECKITOUT
/**
 * Test intersection between a ray and a transformed sphere. Untransformed,
 * the sphere always has radius 0.5 and is centered at the origin.
 *
 * @param intersectionPoint  Output parameter for point of intersection.
 * @param normal             Output parameter for surface normal.
 * @param outside            Output param for whether the ray came from outside.
 * @return                   Ray parameter `t` value. -1 if no intersection.
 */
__host__ __device__ float sphereIntersectionTest(Geom sphere, Ray r,
	glm::vec3& intersectionPoint, glm::vec3& normal, bool& outside) {
	float radius = .5;

	glm::vec3 ro = multiplyMV(sphere.inverseTransform, glm::vec4(r.origin, 1.0f));
	glm::vec3 rd = glm::normalize(multiplyMV(sphere.inverseTransform, glm::vec4(r.direction, 0.0f)));

	Ray rt;
	rt.origin = ro;
	rt.direction = rd;

	float vDotDirection = glm::dot(rt.origin, rt.direction);
	float radicand = vDotDirection * vDotDirection - (glm::dot(rt.origin, rt.origin) - powf(radius, 2));
	if (radicand < 0) {
		return -1;
	}

	float squareRoot = sqrt(radicand);
	float firstTerm = -vDotDirection;
	float t1 = firstTerm + squareRoot;
	float t2 = firstTerm - squareRoot;

	float t = 0;
	if (t1 < 0 && t2 < 0) {
		return -1;
	}
	else if (t1 > 0 && t2 > 0) {
		t = min(t1, t2);
		outside = true;
	}
	else {
		t = max(t1, t2);
		outside = false;
	}

	glm::vec3 objspaceIntersection = getPointOnRay(rt, t);

	intersectionPoint = multiplyMV(sphere.transform, glm::vec4(objspaceIntersection, 1.f));
	normal = glm::normalize(multiplyMV(sphere.invTranspose, glm::vec4(objspaceIntersection, 0.f)));
	if (!outside) {
		normal = -normal;
	}

	return glm::length(r.origin - intersectionPoint);
}


// ----------------------------------------------------------------------------------------------
// ---------------------------- MESH INTERSECTION -----------------------------------------------
// ----------------------------------------------------------------------------------------------

// Modified from https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
__host__ __device__ bool boundingBoxIntersectionTestLocal(
	glm::vec3 minBoxPos, glm::vec3 maxBoxPos,
	glm::vec3 rayOrigin, glm::vec3 rayDirection) {

	glm::vec3 tMin = glm::vec3(
		(minBoxPos.x - rayOrigin.x) / rayDirection.x,
		(minBoxPos.y - rayOrigin.y) / rayDirection.y, 
		(minBoxPos.z - rayOrigin.z) / rayDirection.z);
	glm::vec3 tMax = glm::vec3(
		(maxBoxPos.x - rayOrigin.x) / rayDirection.x,
		(maxBoxPos.y - rayOrigin.y) / rayDirection.y,
		(maxBoxPos.z - rayOrigin.z) / rayDirection.z);

	if (tMin.x > tMax.x) {
		float temp = tMin.x;
		tMin.x = tMax.x;
		tMax.x = temp;
	}

	if (tMin.y > tMax.y) {
		float temp = tMin.y;
		tMin.y = tMax.y;
		tMax.y = temp;
	}
	
	if (tMin.x > tMax.y || tMin.y > tMax.x) {
		return false;
	}

	tMin.x = tMin.y > tMin.x ? tMin.y : tMin.x;
	tMax.x = tMax.y < tMax.x ? tMax.y : tMax.x;

	if (tMin.z > tMax.z) {
		float temp = tMin.z;
		tMin.z = tMax.z;
		tMax.z = temp;
	}

	if (tMin.z > tMax.z) {
		float temp = tMin.z;
		tMin.z = tMax.z;
		tMax.z = temp;
	}

	return !(tMin.x > tMax.z || tMin.z > tMax.x);
}

__host__ __device__ float meshIntersectionTest(const Triangle* tris, Geom mesh, Ray r,
	glm::vec3& intersectionPoint, glm::vec3& normal, bool& outside) {

	// Ray parameters when transformed to local space of the object
	glm::vec3 ro = multiplyMV(mesh.inverseTransform, glm::vec4(r.origin, 1.0f));
	glm::vec3 rd = glm::normalize(multiplyMV(mesh.inverseTransform, glm::vec4(r.direction, 0.0f)));

	bool hitBoundingBox = boundingBoxIntersectionTestLocal(mesh.minPos, mesh.maxPos, ro, rd);
	if (!hitBoundingBox) return -1;

	float t_min = INFINITY;
	Triangle tri;

	glm::vec3 bary;

	for (int i = mesh.triangleIdxStart; i <= mesh.triangleIdxEnd; i++) {
		Triangle curTri = tris[i];
		glm::vec3 curBary;
		if (glm::intersectRayTriangle(ro, rd, curTri.v[0], curTri.v[1], curTri.v[2], curBary)) {
			float t = curBary.z;
			if (t < t_min) {
				t_min = t;
				tri = curTri;
				bary = curBary;
			}
		}
	}
	if (t_min == INFINITY) {
		return -1;
	}

	// Calculate normal
	glm::vec3 n0 = tri.n[0];
	glm::vec3 n1 = tri.n[1];
	glm::vec3 n2 = tri.n[2];
	if (glm::length(n0) == 0) n0 = glm::normalize(glm::cross(tri.v[1] - tri.v[0], tri.v[2] - tri.v[0]));
	if (glm::length(n1) == 1) n1 = glm::normalize(glm::cross(tri.v[2] - tri.v[1], tri.v[0] - tri.v[1]));
	if (glm::length(n2) == 2) n2 = glm::normalize(glm::cross(tri.v[0] - tri.v[2], tri.v[1] - tri.v[2]));

	glm::vec3 baryPos = (1.f - bary.x - bary.y) * tri.v[0] + bary.x * tri.v[1] + bary.y * tri.v[2];
	float S = 0.5f * glm::length(glm::cross(tri.v[0] - tri.v[1], tri.v[2] - tri.v[1]));
	float S0 = 0.5f * glm::length(glm::cross(tri.v[1] - baryPos, tri.v[2] - baryPos));
	float S1 = 0.5f * glm::length(glm::cross(tri.v[0] - baryPos, tri.v[2] - baryPos));
	float S2 = 0.5f * glm::length(glm::cross(tri.v[0] - baryPos, tri.v[1] - baryPos));

	// normal = glm::normalize(glm::cross(tri.v[1] - tri.v[0], tri.v[2] - tri.v[0]));
	normal = glm::normalize(n0 * S0 / S + n1 * S1 / S + n2 * S2 / S);

	outside = glm::dot(normal, rd) <= 0;
	intersectionPoint = ro + rd * t_min;

	intersectionPoint = multiplyMV(mesh.transform, glm::vec4(intersectionPoint, 1.f));
	normal = glm::normalize(multiplyMV(mesh.invTranspose, glm::vec4(normal, 0.0f)));
	return glm::length(r.origin - intersectionPoint);
}