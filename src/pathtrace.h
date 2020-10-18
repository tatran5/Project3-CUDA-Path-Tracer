#pragma once

#include <vector>
#include "scene.h"

enum class DisplayType {DEFAULT, GBUFFER_DEFAULT, GBUFFER_NORMAL, GBUFFER_POSITION};

void pathtraceInit(Scene *scene);
void pathtraceFree();
void pathtrace(int frame, int iteration);
void showGBuffer(uchar4* pbo);
void showImage(uchar4* pbo, int iter);