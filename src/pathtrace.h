#pragma once

#include <vector>
#include "scene.h"

enum class DisplayType {DEFAULT, GBUFFER_DEFAULT, GBUFFER_NORMAL, GBUFFER_POSITION};

void pathtraceInit(Scene *scene);
void pathtraceFree();
void pathtrace(int frame, int iteration, DisplayType displayType, float cPhi, float nPhi, float pPhi);
void showGBuffer(uchar4* pbo, DisplayType displayType);
void showImage(uchar4* pbo, int iter);
void generateGaussianFilter();
void aTrousWaveletFilter(int blurSize, float cPhi, float nPhi, float pPhi, int camResX, int camResY);