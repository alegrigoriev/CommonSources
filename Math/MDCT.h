// Copyright Alexander Grigoriev, 2025, All Rights Reserved
#pragma once
#include <complex>

// Windowed modified discrete cosine transfer
// converts [count] real source samples to [count / 2] real terms.
// IMPORTANT: dst array should be of size [count + 2];
// src and dst array can refer to the same address (be aliased onto each other)
template<typename T>
extern void WindowedMDCT(const T* src, T* dst, unsigned count, unsigned options=0);
