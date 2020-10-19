#pragma once

#include <vector>
#include "scene.h"
#include <chrono>

enum class DisplayType {DEFAULT, GBUFFER_DEFAULT, GBUFFER_NORMAL, GBUFFER_POSITION, GBUFFER_COLOR, DENOISE};

static Scene* hst_scene = NULL;
static glm::vec3* dev_image = NULL;
static Geom* dev_geoms = NULL;
static Material* dev_materials = NULL;
static PathSegment* dev_paths = NULL;
static ShadeableIntersection* dev_intersections = NULL;
static GBufferPixel* dev_gBuffer = NULL;
static GBufferPixelVec3* dev_gBufferNor = NULL;
static GBufferPixelVec3* dev_gBufferPos = NULL;
static GBufferPixelVec3* dev_gBufferCol = NULL;
static GBufferPixelVec3* dev_gBufferCol1 = NULL;
static float* dev_gaussianFilter = NULL;
static int gaussianFilterSize = 5;

static ShadeableIntersection* dev_firstIntersections = NULL; // Cache first bounce of first iter to be re-use in other iters
static Triangle* dev_tris = NULL; // Store triangle information for meshes
static glm::vec2* dev_samples = NULL;

static std::chrono::steady_clock::time_point timePathTrace; // Measure performance

// Depth of field
static float lensRadius = 0.5f;
static float focalDist = 10.f;

void pathtraceInit(Scene *scene);
void pathtraceFree();
void pathtrace(int frame, int iteration, DisplayType displayType, int filterSize, float cPhi, float nPhi, float pPhi);
void showGBuffer(uchar4* pbo, DisplayType displayType);
void showImage(uchar4* pbo, int iter);

void aTrousWaveletFilter(int filterSize, int camResX, int camResY, float cPhi, float nPhi, float pPhi); 
void generateGaussianFilter();