// -*- c++ -*-
// Copyright (c) 2014, Morgan McGuire and Michael Mara
// All rights reserved.
//
// From McGuire and Mara, Efficient GPU Screen-Space Ray Tracing, 
// Journal of Computer Graphics Techniques, 2014
//
// This software is open source under the "BSD 2-clause license":
//
//    Redistribution and use in source and binary forms, with or
//    without modification, are permitted provided that the following
//    conditions are met:
//
//    1. Redistributions of source code must retain the above
//    copyright notice, this list of conditions and the following
//    disclaimer.
//
//    2. Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
//    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
//    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
//    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
//    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
//    USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
//    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
//    THE POSSIBILITY OF SUCH DAMAGE.

#ifndef RAYTRACE_HLSLI
#define RAYTRACE_HLSLI

#include "utility.hlsli"

#define Point2 float2
#define Point3 float3
#define Vector2 float2
#define Vector3 float3
#define Vector4 float4
#define vec2 float2
#define mat4x4 float4x4


float distanceSquared(Point2 A, Point2 B) {
  A -= B;
  return dot(A, A);
}


void swap(in out float a, in out float b) {
  float temp = a;
  a = b;
  b = temp;
}


bool DepthIntersects(float const depth, float const depth_min, float const depth_max, float const thickness) {
  return (depth_max >= depth) && (depth_min - thickness <= depth);
}


/**
    \param vsOrigin Camera-space ray origin, which must be 
    within the view volume and must have z > 0.01 and project within the valid screen rectangle

    \param vsDirection Unit length camera-space ray direction

    \param projectToPixelMatrix A projection matrix that maps to pixel coordinates (not [-1, +1] normalized device coordinates)

    \param vsZBuffer The depth Z buffer

    \param vsZThickness Camera space thickness to ascribe to each pixel in the depth buffer

    \param nearPlaneZ

    \param farPlaneZ

    \param stride Step in horizontal or vertical pixels between samples. This is a float
     because integer math is slow on GPUs, but should be set to an integer >= 1

    \param jitterFraction  Number between 0 and 1 for how far to bump the ray in stride units
      to conceal banding artifacts

    \param maxSteps Maximum number of iterations. Higher gives better images but may be slow

    \param maxRayTraceDistance Maximum camera-space distance to trace before returning a miss

    \param hitPixel Pixel coordinates of the first intersection with the scene

    \param vsHitPoint Camera space location of the ray hit

    Single-layer

 */
bool traceScreenSpaceRay
(Point3 vsOrigin,
 Vector3 vsDirection,
 mat4x4 projectToPixelMatrix,
 Texture2D<float> vsZBuffer,
 float vsZThickness,
 float nearPlaneZ,
 float farPlaneZ,
 float stride,
 float jitterFraction,
 float maxSteps,
 in float maxRayTraceDistance,
 out Point2 hitPixel,
 out Point3 vsHitPoint) {
  // Clip ray to a near plane in 3D (doesn't have to be *the* near plane, although that would be a good idea)
  float rayLength = ((vsOrigin.z + vsDirection.z * maxRayTraceDistance) < nearPlaneZ)
                      ? (nearPlaneZ - vsOrigin.z) / vsDirection.z
                      : maxRayTraceDistance;
  Point3 vsEndPoint = vsDirection * rayLength + vsOrigin;

  // Project into screen space
  Vector4 H0 = mul(Vector4(vsOrigin, 1.0), projectToPixelMatrix);
  Vector4 H1 = mul(Vector4(vsEndPoint, 1.0), projectToPixelMatrix);

  // There are a lot of divisions by w that can be turned into multiplications
  // at some minor precision loss...and we need to interpolate these 1/w values
  // anyway.
  //
  // Because the caller was required to clip to the near plane,
  // this homogeneous division (projecting from 4D to 2D) is guaranteed 
  // to succeed. 
  float k0 = 1.0 / H0.w;
  float k1 = 1.0 / H1.w;

  // Switch the original points to values that interpolate linearly in 2D
  Point3 Q0 = vsOrigin * k0;
  Point3 Q1 = vsEndPoint * k1;

  // Screen-space endpoints
  Point2 P0 = H0.xy * k0;
  Point2 P1 = H1.xy * k1;

  // [Optional clipping to frustum sides here]

  // Initialize to off screen
  hitPixel = Point2(-1.0, -1.0);

  // If the line is degenerate, make it cover at least one pixel
  // to avoid handling zero-pixel extent as a special case later
  P1 += (vec2)(distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0;

  Vector2 delta = P1 - P0;

  // Permute so that the primary iteration is in x to reduce
  // large branches later
  bool permute = false;
  if (abs(delta.x) < abs(delta.y)) {
    // More-vertical line. Create a permutation that swaps x and y in the output
    permute = true;

    // Directly swizzle the inputs
    delta = delta.yx;
    P1 = P1.yx;
    P0 = P0.yx;
  }

  // From now on, "x" is the primary iteration direction and "y" is the secondary one

  float stepDirection = sign(delta.x);
  float invdx = stepDirection / delta.x;
  Vector2 dP = Vector2(stepDirection, invdx * delta.y);

  // Track the derivatives of Q and k
  Vector3 dQ = (Q1 - Q0) * invdx;
  float dk = (k1 - k0) * invdx;

  // Scale derivatives by the desired pixel stride
  dP *= stride;
  dQ *= stride;
  dk *= stride;

  // Offset the starting values by the jitter fraction
  P0 += dP * jitterFraction;
  Q0 += dQ * jitterFraction;
  k0 += dk * jitterFraction;

  // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, and k from k0 to k1
  Point3 Q = Q0;
  float k = k0;

  // We track the ray depth at +/- 1/2 pixel to treat pixels as clip-space solid 
  // voxels. Because the depth at -1/2 for a given pixel will be the same as at 
  // +1/2 for the previous iteration, we actually only have to compute one value 
  // per iteration.
  float prevZMaxEstimate = vsOrigin.z;
  float stepCount = 0.0;
  float rayZMax = prevZMaxEstimate, rayZMin = prevZMaxEstimate;
  float sceneZMax = rayZMax + 1e4;

  // P1.x is never modified after this point, so pre-scale it by 
  // the step direction for a signed comparison
  float end = P1.x * stepDirection;

  // We only advance the z field of Q in the inner loop, since
  // Q.xy is never used until after the loop terminates.

  for (Point2 P = P0;
       ((P.x * stepDirection) <= end) &&
       (stepCount < maxSteps) &&
       !DepthIntersects(sceneZMax, rayZMin, rayZMax, vsZThickness) &&
       (sceneZMax != 0.0);
       P += dP, Q.z += dQ.z, k += dk, stepCount += 1.0) {
    hitPixel = permute ? P.yx : P;

    // The depth range that the ray covers within this loop
    // iteration.  Assume that the ray is moving in increasing z
    // and swap if backwards.  Because one end of the interval is
    // shared between adjacent iterations, we track the previous
    // value and then swap as needed to ensure correct ordering
    rayZMin = prevZMaxEstimate;

    // Compute the value at 1/2 pixel into the future
    rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
    prevZMaxEstimate = rayZMax;
    if (rayZMin > rayZMax) { swap(rayZMin, rayZMax); }

    // Camera-space z of the background
    sceneZMax = LinearizeDepth(vsZBuffer.Load(int3(hitPixel, 0)).r, nearPlaneZ, farPlaneZ);
  } // pixel on ray

  Q.xy += dQ.xy * stepCount;
  vsHitPoint = Q * (1.0 / k);

  // Matches the new loop condition:
  return DepthIntersects(sceneZMax, rayZMin, rayZMax, vsZThickness);
}

#endif
