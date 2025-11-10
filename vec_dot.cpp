#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <stddef.h>

// ======================================================
// x86/x64 AVX 实现
// ======================================================
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>

float vec_dot(const float *a, const float *b, size_t n)
{
    size_t i = 0;
    const size_t step = 8;
    __m256 vsum = _mm256_setzero_ps();

    for (; i + step <= n; i += step)
    {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        vsum = _mm256_add_ps(vsum, _mm256_mul_ps(va, vb));
    }

    __m128 low = _mm256_castps256_ps128(vsum);
    __m128 high = _mm256_extractf128_ps(vsum, 1);
    __m128 sum128 = _mm_add_ps(low, high);
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    float result = _mm_cvtss_f32(sum128);

    for (; i < n; ++i)
        result += a[i] * b[i];
    return result;
}

const char *arch_name() { return "AVX (x86/x64)"; }

// ======================================================
// ARM/ARM64 NEON 实现
// ======================================================
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>

float vec_dot(const float *a, const float *b, size_t n)
{
    size_t i = 0;
    const size_t step = 4;
    float32x4_t vsum = vdupq_n_f32(0.0f);

    for (; i + step <= n; i += step)
    {
        float32x4_t va = vld1q_f32(a + i);
        float32x4_t vb = vld1q_f32(b + i);
        vsum = vmlaq_f32(vsum, va, vb);
    }

    float32x2_t sum2 = vadd_f32(vget_low_f32(vsum), vget_high_f32(vsum));
    float result = vget_lane_f32(sum2, 0) + vget_lane_f32(sum2, 1);

    for (; i < n; ++i)
        result += a[i] * b[i];
    return result;
}

const char *arch_name() { return "NEON (ARM/ARM64)"; }

// ======================================================
// 默认标量版本（无 SIMD）
// ======================================================
#else
float vec_dot(const float *a, const float *b, size_t n)
{
    float s = 0;
    for (size_t i = 0; i < n; ++i)
        s += a[i] * b[i];
    return s;
}
const char *arch_name() { return "Scalar (No SIMD)"; }
#endif

// ======================================================
// 主函数（平台无关）
// ======================================================
int main()
{
    const size_t N = 1000003;
    std::vector<float> A(N), B(N);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (size_t i = 0; i < N; ++i)
    {
        A[i] = dist(rng);
        B[i] = dist(rng);
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    float r = vec_dot(A.data(), B.data(), N);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "Arch: " << arch_name() << "\n";
    std::cout << "Result = " << r << "  time: " << ms << " ms\n";
    return 0;
}
