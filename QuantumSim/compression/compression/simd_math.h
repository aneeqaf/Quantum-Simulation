//
//  simd_math.h
//  compression
//
//  Created by Aneeqa Fatima on 1/15/19.
//  Copyright © 2019 Aneeqa Fatima. All rights reserved.
//

#ifndef simd_math_h
#define simd_math_h

// Capability definitions for 256-bit AVX2
#define GMX_SIMD                                1
#define GMX_SIMD_HAVE_FLOAT                     1
#define GMX_SIMD_HAVE_DOUBLE                    1
#define GMX_SIMD_HAVE_LOADU                     1
#define GMX_SIMD_HAVE_STOREU                    1
#define GMX_SIMD_HAVE_LOGICAL                   1
#define GMX_SIMD_HAVE_FMA                       1
#define GMX_SIMD_HAVE_FINT32_EXTRACT            1
#define GMX_SIMD_HAVE_FINT32_LOGICAL            1
#define GMX_SIMD_HAVE_FINT32_ARITHMETICS        1
#define GMX_SIMD_HAVE_DINT32_EXTRACT            1
#define GMX_SIMD_HAVE_DINT32_LOGICAL            1
#define GMX_SIMD_HAVE_DINT32_ARITHMETICS        1
#define GMX_SIMD_HAVE_NATIVE_COPYSIGN_FLOAT     0
#define GMX_SIMD_HAVE_NATIVE_RSQRT_ITER_FLOAT   0
#define GMX_SIMD_HAVE_NATIVE_RCP_ITER_FLOAT     0
#define GMX_SIMD_HAVE_NATIVE_LOG_FLOAT          0
#define GMX_SIMD_HAVE_NATIVE_EXP2_FLOAT         0
#define GMX_SIMD_HAVE_NATIVE_EXP_FLOAT          0
#define GMX_SIMD_HAVE_NATIVE_COPYSIGN_DOUBLE    0
#define GMX_SIMD_HAVE_NATIVE_RSQRT_ITER_DOUBLE  0
#define GMX_SIMD_HAVE_NATIVE_RCP_ITER_DOUBLE    0
#define GMX_SIMD_HAVE_NATIVE_LOG_DOUBLE         0
#define GMX_SIMD_HAVE_NATIVE_EXP2_DOUBLE        0
#define GMX_SIMD_HAVE_NATIVE_EXP_DOUBLE         0
#define GMX_SIMD_HAVE_GATHER_LOADU_BYSIMDINT_TRANSPOSE_FLOAT   1
#define GMX_SIMD_HAVE_GATHER_LOADU_BYSIMDINT_TRANSPOSE_DOUBLE  1
#define GMX_SIMD_HAVE_HSIMD_UTIL_FLOAT          1
#define GMX_SIMD_HAVE_HSIMD_UTIL_DOUBLE         0 // Not needed for width 4
#define GMX_SIMD_HAVE_4NSIMD_UTIL_FLOAT         1

#define GMX_SIMD4_HAVE_FLOAT                    0
#define GMX_SIMD4_HAVE_DOUBLE                   0

// Implementation details
#define GMX_SIMD_FLOAT_WIDTH                    8
#define GMX_SIMD_DOUBLE_WIDTH                   4
#define GMX_SIMD_FINT32_WIDTH                   8
#define GMX_SIMD_DINT32_WIDTH                   4
#define GMX_SIMD4_WIDTH                         4
#define GMX_SIMD_ALIGNMENT                     32 // Bytes (8*single or 4*double)
#define GMX_SIMD_RSQRT_BITS                    11
#define GMX_SIMD_RCP_BITS                      11

/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2012,2013,2014,2015,2017,2018, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
#ifndef GMX_SIMD_SIMD_MATH_H
#define GMX_SIMD_SIMD_MATH_H

/*! \libinternal \file
 *
 * \brief Math functions for SIMD datatypes.
 *
 * \attention This file is generic for all SIMD architectures, so you cannot
 * assume that any of the optional SIMD features (as defined in simd.h) are
 * present. In particular, this means you cannot assume support for integers,
 * logical operations (neither on floating-point nor integer values), shifts,
 * and the architecture might only have SIMD for either float or double.
 * Second, to keep this file clean and general, any additions to this file
 * must work for all possible SIMD architectures in both single and double
 * precision (if they support it), and you cannot make any assumptions about
 * SIMD width.
 *
 * \author Erik Lindahl <erik.lindahl@scilifelab.se>
 *
 * \inlibraryapi
 * \ingroup module_simd
 */

#include "config.h"

//#include <cmath>

#include <limits>

#include "utilities.h"
#include "basedefinitions.h"
#include "real.h"
#include "impl_x86_avx_256_simd_double.h"

namespace gmx
{
    
    class SimdSetZeroProxy
    {
    public:
        //!\brief Conversion method that returns 0.0 as float
        operator float() const { return 0.0f; }
        //!\brief Conversion method that returns 0.0 as double
        operator double() const { return 0.0; }
        //!\brief Conversion method that returns 0.0 as int32
        operator std::int32_t() const { return 0; }
#if GMX_SIMD_HAVE_FLOAT
        //!\brief Conversion method that will execute setZero() for SimdFloat
        operator SimdFloat() const { return setZeroF(); }
        //!\brief Conversion method that will execute setZero() for SimdFInt32
        operator SimdFInt32() const { return setZeroFI(); }
#endif
#if GMX_SIMD_HAVE_DOUBLE
        //!\brief Conversion method that will execute setZero() for SimdDouble
        operator SimdDouble() const { return setZeroD(); }
        //!\brief Conversion method that will execute setZero() for SimdDInt32
        operator SimdDInt32() const { return setZeroDI(); }
#endif
    };
    
    static inline const SimdSetZeroProxy gmx_simdcall
    setZero()
    {
        return {};
    }
    
#if GMX_SIMD
    
    /*! \cond libapi */
    /*! \addtogroup module_simd */
    /*! \{ */
    
    /*! \name Implementation accuracy settings
     *  \{
     */
    
    /*! \} */
    
    /*! \brief Helper function to set any SIMD or scalar variable to zero
     *
     * \return Proxy object that will call the actual function to set a SIMD/scalar
     *         variable to zero based on the conversion function called when you
     *         assign the result.
     */
    
#if GMX_SIMD_HAVE_FLOAT
    
    /*! \name Single precision SIMD math functions
     *
     *  \note In most cases you should use the real-precision functions instead.
     *  \{
     */
    
    /****************************************
     * SINGLE PRECISION SIMD MATH FUNCTIONS *
     ****************************************/
    
#if !GMX_SIMD_HAVE_NATIVE_COPYSIGN_FLOAT
    /*! \brief Composes floating point value with the magnitude of x and the sign of y.
     *
     * \param x Values to set sign for
     * \param y Values used to set sign
     * \return  Magnitude of x, sign of y
     */
    static inline SimdFloat gmx_simdcall
    copysign(SimdFloat x, SimdFloat y)
    {
#if GMX_SIMD_HAVE_LOGICAL
        return abs(x) | ( SimdFloat(GMX_FLOAT_NEGZERO) & y );
#else
        return blend(abs(x), -abs(x), y < setZero());
#endif
    }
#endif
    
#if !GMX_SIMD_HAVE_NATIVE_RSQRT_ITER_FLOAT
    /*! \brief Perform one Newton-Raphson iteration to improve 1/sqrt(x) for SIMD float.
     *
     * This is a low-level routine that should only be used by SIMD math routine
     * that evaluates the inverse square root.
     *
     *  \param lu Approximation of 1/sqrt(x), typically obtained from lookup.
     *  \param x  The reference (starting) value x for which we want 1/sqrt(x).
     *  \return   An improved approximation with roughly twice as many bits of accuracy.
     */
    static inline SimdFloat gmx_simdcall
    rsqrtIter(SimdFloat lu, SimdFloat x)
    {
        SimdFloat tmp1 = x*lu;
        SimdFloat tmp2 = SimdFloat(-0.5f)*lu;
        tmp1 = fma(tmp1, lu, SimdFloat(-3.0f));
        return tmp1*tmp2;
    }
#endif
    
    /*! \brief Calculate 1/sqrt(x) for SIMD float.
     *
     *  \param x Argument that must be larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *
     *  \return 1/sqrt(x). Result is undefined if your argument was invalid.
     */
    static inline SimdFloat gmx_simdcall
    invsqrt(SimdFloat x)
    {
        SimdFloat lu = rsqrt(x);
#if (GMX_SIMD_RSQRT_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief Calculate 1/sqrt(x) for two SIMD floats.
     *
     * \param x0  First set of arguments, x0 must be in single range (see below).
     * \param x1  Second set of arguments, x1 must be in single range (see below).
     * \param[out] out0  Result 1/sqrt(x0)
     * \param[out] out1  Result 1/sqrt(x1)
     *
     *  In particular for double precision we can sometimes calculate square root
     *  pairs slightly faster by using single precision until the very last step.
     *
     * \note Both arguments must be larger than GMX_FLOAT_MIN and smaller than
     *       GMX_FLOAT_MAX, i.e. within the range of single precision.
     *       For the single precision implementation this is obviously always
     *       true for positive values, but for double precision it adds an
     *       extra restriction since the first lookup step might have to be
     *       performed in single precision on some architectures. Note that the
     *       responsibility for checking falls on you - this routine does not
     *       check arguments.
     */
    static inline void gmx_simdcall
    invsqrtPair(SimdFloat x0,    SimdFloat x1,
                SimdFloat *out0, SimdFloat *out1)
    {
        *out0 = invsqrt(x0);
        *out1 = invsqrt(x1);
    }
    
#if !GMX_SIMD_HAVE_NATIVE_RCP_ITER_FLOAT
    /*! \brief Perform one Newton-Raphson iteration to improve 1/x for SIMD float.
     *
     * This is a low-level routine that should only be used by SIMD math routine
     * that evaluates the reciprocal.
     *
     *  \param lu Approximation of 1/x, typically obtained from lookup.
     *  \param x  The reference (starting) value x for which we want 1/x.
     *  \return   An improved approximation with roughly twice as many bits of accuracy.
     */
    static inline SimdFloat gmx_simdcall
    rcpIter(SimdFloat lu, SimdFloat x)
    {
        return lu*fnma(lu, x, SimdFloat(2.0f));
    }
#endif
    
    /*! \brief Calculate 1/x for SIMD float.
     *
     *  \param x Argument with magnitude larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *
     *  \return 1/x. Result is undefined if your argument was invalid.
     */
    static inline SimdFloat gmx_simdcall
    inv(SimdFloat x)
    {
        SimdFloat lu = rcp(x);
#if (GMX_SIMD_RCP_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief Division for SIMD floats
     *
     * \param nom    Nominator
     * \param denom  Denominator, with magnitude in range (GMX_FLOAT_MIN,GMX_FLOAT_MAX).
     *               For single precision this is equivalent to a nonzero argument,
     *               but in double precision it adds an extra restriction since
     *               the first lookup step might have to be performed in single
     *               precision on some architectures. Note that the responsibility
     *               for checking falls on you - this routine does not check arguments.
     *
     * \return nom/denom
     *
     * \note This function does not use any masking to avoid problems with
     *       zero values in the denominator.
     */
    static inline SimdFloat gmx_simdcall
    operator/(SimdFloat nom, SimdFloat denom)
    {
        return nom*inv(denom);
    }
    
    /*! \brief Calculate 1/sqrt(x) for masked entries of SIMD float.
     *
     *  This routine only evaluates 1/sqrt(x) for elements for which mask is true.
     *  Illegal values in the masked-out elements will not lead to
     *  floating-point exceptions.
     *
     *  \param x Argument that must be larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX for masked-in entries.
     *           See \ref invsqrt for the discussion about argument restrictions.
     *  \param m Mask
     *  \return 1/sqrt(x). Result is undefined if your argument was invalid or
     *          entry was not masked, and 0.0 for masked-out entries.
     */
    static inline SimdFloat
    maskzInvsqrt(SimdFloat x, SimdFBool m)
    {
        SimdFloat lu = maskzRsqrt(x, m);
#if (GMX_SIMD_RSQRT_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief Calculate 1/x for SIMD float, masked version.
     *
     *  \param x Argument with magnitude larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX for masked-in entries.
     *           See \ref invsqrt for the discussion about argument restrictions.
     *  \param m Mask
     *  \return 1/x for elements where m is true, or 0.0 for masked-out entries.
     */
    static inline SimdFloat gmx_simdcall
    maskzInv(SimdFloat x, SimdFBool m)
    {
        SimdFloat lu = maskzRcp(x, m);
#if (GMX_SIMD_RCP_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief Calculate sqrt(x) for SIMD floats
     *
     *  \tparam opt By default, this function checks if the input value is 0.0
     *              and masks this to return the correct result. If you are certain
     *              your argument will never be zero, and you know you need to save
     *              every single cycle you can, you can alternatively call the
     *              function as sqrt<MathOptimization::Unsafe>(x).
     *
     *  \param  x   Argument that must be in range 0 <=x <= GMX_FLOAT_MAX, since the
     *              lookup step often has to be implemented in single precision.
     *              Arguments smaller than GMX_FLOAT_MIN will always lead to a zero
     *              result, even in double precision. If you are using the unsafe
     *              math optimization parameter, the argument must be in the range
     *              GMX_FLOAT_MIN <= x <= GMX_FLOAT_MAX.
     *
     *  \return sqrt(x). The result is undefined if the input value does not fall
     *          in the allowed range specified for the argument.
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdFloat gmx_simdcall
    sqrt(SimdFloat x)
    {
        if (opt == MathOptimization::Safe)
        {
            SimdFloat  res = maskzInvsqrt(x, setZero() < x);
            return res*x;
        }
        else
        {
            return x * invsqrt(x);
        }
    }
    
#if !GMX_SIMD_HAVE_NATIVE_LOG_FLOAT
    /*! \brief SIMD float log(x). This is the natural logarithm.
     *
     * \param x Argument, should be >0.
     * \result The natural logarithm of x. Undefined if argument is invalid.
     */
    static inline SimdFloat gmx_simdcall
    log(SimdFloat x)
    {
        const SimdFloat  one(1.0f);
        const SimdFloat  two(2.0f);
        const SimdFloat  invsqrt2(1.0f/std::sqrt(2.0f));
        const SimdFloat  corr(0.693147180559945286226764f);
        const SimdFloat  CL9(0.2371599674224853515625f);
        const SimdFloat  CL7(0.285279005765914916992188f);
        const SimdFloat  CL5(0.400005519390106201171875f);
        const SimdFloat  CL3(0.666666567325592041015625f);
        const SimdFloat  CL1(2.0f);
        SimdFloat        fExp, x2, p;
        SimdFBool        m;
        SimdFInt32       iExp;
        
        x     = frexp(x, &iExp);
        fExp  = cvtI2R(iExp);
        
        m     = x < invsqrt2;
        // Adjust to non-IEEE format for x<1/sqrt(2): exponent -= 1, mantissa *= 2.0
        fExp  = fExp - selectByMask(one, m);
        x     = x * blend(one, two, m);
        
        x     = (x-one) * inv( x+one );
        x2    = x * x;
        
        p     = fma(CL9, x2, CL7);
        p     = fma(p, x2, CL5);
        p     = fma(p, x2, CL3);
        p     = fma(p, x2, CL1);
        p     = fma(p, x, corr*fExp);
        
        return p;
    }
#endif
    
#if !GMX_SIMD_HAVE_NATIVE_EXP2_FLOAT
    /*! \brief SIMD float 2^x
     *
     * \tparam opt If this is changed from the default (safe) into the unsafe
     *             option, input values that would otherwise lead to zero-clamped
     *             results are not allowed and will lead to undefined results.
     *
     * \param x Argument. For the default (safe) function version this can be
     *          arbitrarily small value, but the routine might clamp the result to
     *          zero for arguments that would produce subnormal IEEE754-2008 results.
     *          This corresponds to inputs below -126 in single or -1022 in double,
     *          and it might overflow for arguments reaching 127 (single) or
     *          1023 (double). If you enable the unsafe math optimization,
     *          very small arguments will not necessarily be zero-clamped, but
     *          can produce undefined results.
     *
     * \result 2^x. The result is undefined for very large arguments that cause
     *          internal floating-point overflow. If unsafe optimizations are enabled,
     *          this is also true for very small values.
     *
     * \note    The definition range of this function is just-so-slightly smaller
     *          than the allowed IEEE exponents for many architectures. This is due
     *          to the implementation, which will hopefully improve in the future.
     *
     * \warning You cannot rely on this implementation returning inf for arguments
     *          that cause overflow. If you have some very large
     *          values and need to rely on getting a valid numerical output,
     *          take the minimum of your variable and the largest valid argument
     *          before calling this routine.
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdFloat gmx_simdcall
    exp2(SimdFloat x)
    {
        const SimdFloat  CC6(0.0001534581200287996416911311f);
        const SimdFloat  CC5(0.001339993121934088894618990f);
        const SimdFloat  CC4(0.009618488957115180159497841f);
        const SimdFloat  CC3(0.05550328776964726865751735f);
        const SimdFloat  CC2(0.2402264689063408646490722f);
        const SimdFloat  CC1(0.6931472057372680777553816f);
        const SimdFloat  one(1.0f);
        
        SimdFloat        intpart;
        SimdFloat        fexppart;
        SimdFloat        p;
        
        // Large negative values are valid arguments to exp2(), so there are two
        // things we need to account for:
        // 1. When the exponents reaches -127, the (biased) exponent field will be
        //    zero and we can no longer multiply with it. There are special IEEE
        //    formats to handle this range, but for now we have to accept that
        //    we cannot handle those arguments. If input value becomes even more
        //    negative, it will start to loop and we would end up with invalid
        //    exponents. Thus, we need to limit or mask this.
        // 2. For VERY large negative values, we will have problems that the
        //    subtraction to get the fractional part loses accuracy, and then we
        //    can end up with overflows in the polynomial.
        //
        // For now, we handle this by forwarding the math optimization setting to
        // ldexp, where the routine will return zero for very small arguments.
        //
        // However, before doing that we need to make sure we do not call cvtR2I
        // with an argument that is so negative it cannot be converted to an integer.
        if (opt == MathOptimization::Safe)
        {
            x         = max(x, SimdFloat(std::numeric_limits<std::int32_t>::lowest()));
        }
        
        fexppart  = ldexp<opt>(one, cvtR2I(x));
        intpart   = round(x);
        x         = x - intpart;
        
        p         = fma(CC6, x, CC5);
        p         = fma(p, x, CC4);
        p         = fma(p, x, CC3);
        p         = fma(p, x, CC2);
        p         = fma(p, x, CC1);
        p         = fma(p, x, one);
        x         = p * fexppart;
        return x;
    }
#endif
    
#if !GMX_SIMD_HAVE_NATIVE_EXP_FLOAT
    /*! \brief SIMD float exp(x).
     *
     * In addition to scaling the argument for 2^x this routine correctly does
     * extended precision arithmetics to improve accuracy.
     *
     * \tparam opt If this is changed from the default (safe) into the unsafe
     *             option, input values that would otherwise lead to zero-clamped
     *             results are not allowed and will lead to undefined results.
     *
     * \param x Argument. For the default (safe) function version this can be
     *          arbitrarily small value, but the routine might clamp the result to
     *          zero for arguments that would produce subnormal IEEE754-2008 results.
     *          This corresponds to input arguments reaching
     *          -126*ln(2)=-87.3 in single, or -1022*ln(2)=-708.4 (double).
     *          Similarly, it might overflow for arguments reaching
     *          127*ln(2)=88.0 (single) or 1023*ln(2)=709.1 (double). If the
     *          unsafe math optimizations are enabled, small input values that would
     *          result in zero-clamped output are not allowed.
     *
     * \result exp(x). Overflowing arguments are likely to either return 0 or inf,
     *         depending on the underlying implementation. If unsafe optimizations
     *         are enabled, this is also true for very small values.
     *
     * \note    The definition range of this function is just-so-slightly smaller
     *          than the allowed IEEE exponents for many architectures. This is due
     *          to the implementation, which will hopefully improve in the future.
     *
     * \warning You cannot rely on this implementation returning inf for arguments
     *          that cause overflow. If you have some very large
     *          values and need to rely on getting a valid numerical output,
     *          take the minimum of your variable and the largest valid argument
     *          before calling this routine.
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdFloat gmx_simdcall
    exp(SimdFloat x)
    {
        const SimdFloat  argscale(1.44269504088896341f);
        const SimdFloat  invargscale0(-0.693145751953125f);
        const SimdFloat  invargscale1(-1.428606765330187045e-06f);
        const SimdFloat  CC4(0.00136324646882712841033936f);
        const SimdFloat  CC3(0.00836596917361021041870117f);
        const SimdFloat  CC2(0.0416710823774337768554688f);
        const SimdFloat  CC1(0.166665524244308471679688f);
        const SimdFloat  CC0(0.499999850988388061523438f);
        const SimdFloat  one(1.0f);
        SimdFloat        fexppart;
        SimdFloat        intpart;
        SimdFloat        y, p;
        
        // Large negative values are valid arguments to exp2(), so there are two
        // things we need to account for:
        // 1. When the exponents reaches -127, the (biased) exponent field will be
        //    zero and we can no longer multiply with it. There are special IEEE
        //    formats to handle this range, but for now we have to accept that
        //    we cannot handle those arguments. If input value becomes even more
        //    negative, it will start to loop and we would end up with invalid
        //    exponents. Thus, we need to limit or mask this.
        // 2. For VERY large negative values, we will have problems that the
        //    subtraction to get the fractional part loses accuracy, and then we
        //    can end up with overflows in the polynomial.
        //
        // For now, we handle this by forwarding the math optimization setting to
        // ldexp, where the routine will return zero for very small arguments.
        //
        // However, before doing that we need to make sure we do not call cvtR2I
        // with an argument that is so negative it cannot be converted to an integer
        // after being multiplied by argscale.
        
        if (opt == MathOptimization::Safe)
        {
            x         = max(x, SimdFloat(std::numeric_limits<std::int32_t>::lowest())/argscale);
        }
        
        y         = x * argscale;
        
        
        fexppart  = ldexp<opt>(one, cvtR2I(y));
        intpart   = round(y);
        
        // Extended precision arithmetics
        x         = fma(invargscale0, intpart, x);
        x         = fma(invargscale1, intpart, x);
        
        p         = fma(CC4, x, CC3);
        p         = fma(p, x, CC2);
        p         = fma(p, x, CC1);
        p         = fma(p, x, CC0);
        p         = fma(x*x, p, x);
        x         = fma(p, fexppart, fexppart);
        return x;
    }
#endif
    
    /*! \brief SIMD float asin(x).
     *
     * \param x The argument to evaluate atan for
     * \result Atan(x), same argument/value range as standard math library.
     */
    static inline SimdFloat gmx_simdcall
    atan(SimdFloat x)
    {
        const SimdFloat halfpi(static_cast<float>(M_PI/2.0f));
        const SimdFloat CA17(0.002823638962581753730774f);
        const SimdFloat CA15(-0.01595690287649631500244f);
        const SimdFloat CA13(0.04250498861074447631836f);
        const SimdFloat CA11(-0.07489009201526641845703f);
        const SimdFloat CA9 (0.1063479334115982055664f);
        const SimdFloat CA7 (-0.1420273631811141967773f);
        const SimdFloat CA5 (0.1999269574880599975585f);
        const SimdFloat CA3 (-0.3333310186862945556640f);
        const SimdFloat one (1.0f);
        SimdFloat       x2, x3, x4, pA, pB;
        SimdFBool       m, m2;
        
        m     = x < setZero();
        x     = abs(x);
        m2    = one < x;
        x     = blend(x, maskzInv(x, m2), m2);
        
        x2    = x * x;
        x3    = x2 * x;
        x4    = x2 * x2;
        pA    = fma(CA17, x4, CA13);
        pB    = fma(CA15, x4, CA11);
        pA    = fma(pA, x4, CA9);
        pB    = fma(pB, x4, CA7);
        pA    = fma(pA, x4, CA5);
        pB    = fma(pB, x4, CA3);
        pA    = fma(pA, x2, pB);
        pA    = fma(pA, x3, x);
        
        pA    = blend(pA, halfpi-pA, m2);
        pA    = blend(pA, -pA, m);
        
        return pA;
    }
    
    /*! \brief SIMD float atan2(y,x).
     *
     * \param y Y component of vector, any quartile
     * \param x X component of vector, any quartile
     * \result Atan(y,x), same argument/value range as standard math library.
     *
     * \note This routine should provide correct results for all finite
     * non-zero or positive-zero arguments. However, negative zero arguments will
     * be treated as positive zero, which means the return value will deviate from
     * the standard math library atan2(y,x) for those cases. That should not be
     * of any concern in Gromacs, and in particular it will not affect calculations
     * of angles from vectors.
     */
    static inline SimdFloat gmx_simdcall
    atan2(SimdFloat y, SimdFloat x)
    {
        const SimdFloat pi(static_cast<float>(M_PI));
        const SimdFloat halfpi(static_cast<float>(M_PI/2.0));
        SimdFloat       xinv, p, aoffset;
        SimdFBool       mask_xnz, mask_ynz, mask_xlt0, mask_ylt0;
        
        mask_xnz  = x != setZero();
        mask_ynz  = y != setZero();
        mask_xlt0 = x < setZero();
        mask_ylt0 = y < setZero();
        
        aoffset   = selectByNotMask(halfpi, mask_xnz);
        aoffset   = selectByMask(aoffset, mask_ynz);
        
        aoffset   = blend(aoffset, pi, mask_xlt0);
        aoffset   = blend(aoffset, -aoffset, mask_ylt0);
        
        xinv      = maskzInv(x, mask_xnz);
        p         = y * xinv;
        p         = atan(p);
        p         = p + aoffset;
        
        return p;
    }
#endif
    
#if GMX_SIMD_HAVE_DOUBLE
    
    
    /*! \name Double precision SIMD math functions
     *
     *  \note In most cases you should use the real-precision functions instead.
     *  \{
     */
    
    /****************************************
     * DOUBLE PRECISION SIMD MATH FUNCTIONS *
     ****************************************/
    
#if !GMX_SIMD_HAVE_NATIVE_COPYSIGN_DOUBLE
    /*! \brief Composes floating point value with the magnitude of x and the sign of y.
     *
     * \param x Values to set sign for
     * \param y Values used to set sign
     * \return  Magnitude of x, sign of y
     */
    static inline SimdDouble gmx_simdcall
    copysign(SimdDouble x, SimdDouble y)
    {
#if GMX_SIMD_HAVE_LOGICAL
        return abs(x) | (SimdDouble(GMX_DOUBLE_NEGZERO) & y);
#else
        return blend(abs(x), -abs(x), (y < setZero()));
#endif
    }
#endif
    
#if !GMX_SIMD_HAVE_NATIVE_RSQRT_ITER_DOUBLE
    /*! \brief Perform one Newton-Raphson iteration to improve 1/sqrt(x) for SIMD double.
     *
     * This is a low-level routine that should only be used by SIMD math routine
     * that evaluates the inverse square root.
     *
     *  \param lu Approximation of 1/sqrt(x), typically obtained from lookup.
     *  \param x  The reference (starting) value x for which we want 1/sqrt(x).
     *  \return   An improved approximation with roughly twice as many bits of accuracy.
     */
    static inline SimdDouble gmx_simdcall
    rsqrtIter(SimdDouble lu, SimdDouble x)
    {
        SimdDouble tmp1 = x*lu;
        SimdDouble tmp2 = SimdDouble(-0.5)*lu;
        tmp1 = fma(tmp1, lu, SimdDouble(-3.0));
        return tmp1*tmp2;
    }
#endif
    
    /*! \brief Calculate 1/sqrt(x) for SIMD double.
     *
     *  \param x Argument that must be larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *
     *  \return 1/sqrt(x). Result is undefined if your argument was invalid.
     */
    static inline SimdDouble gmx_simdcall
    invsqrt(SimdDouble x)
    {
        SimdDouble lu = rsqrt(x);
#if (GMX_SIMD_RSQRT_BITS < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*2 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*4 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*8 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rsqrtIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief Calculate 1/sqrt(x) for two SIMD doubles.
     *
     * \param x0  First set of arguments, x0 must be in single range (see below).
     * \param x1  Second set of arguments, x1 must be in single range (see below).
     * \param[out] out0  Result 1/sqrt(x0)
     * \param[out] out1  Result 1/sqrt(x1)
     *
     *  In particular for double precision we can sometimes calculate square root
     *  pairs slightly faster by using single precision until the very last step.
     *
     * \note Both arguments must be larger than GMX_FLOAT_MIN and smaller than
     *       GMX_FLOAT_MAX, i.e. within the range of single precision.
     *       For the single precision implementation this is obviously always
     *       true for positive values, but for double precision it adds an
     *       extra restriction since the first lookup step might have to be
     *       performed in single precision on some architectures. Note that the
     *       responsibility for checking falls on you - this routine does not
     *       check arguments.
     */
    static inline void gmx_simdcall
    invsqrtPair(SimdDouble x0,    SimdDouble x1,
                SimdDouble *out0, SimdDouble *out1)
    {
#if GMX_SIMD_HAVE_FLOAT && (GMX_SIMD_FLOAT_WIDTH == 2*GMX_SIMD_DOUBLE_WIDTH) && (GMX_SIMD_RSQRT_BITS < 22)
        SimdFloat  xf  = cvtDD2F(x0, x1);
        SimdFloat  luf = rsqrt(xf);
        SimdDouble lu0, lu1;
        // Intermediate target is single - mantissa+1 bits
#if (GMX_SIMD_RSQRT_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        luf = rsqrtIter(luf, xf);
#endif
#if (GMX_SIMD_RSQRT_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        luf = rsqrtIter(luf, xf);
#endif
#if (GMX_SIMD_RSQRT_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        luf = rsqrtIter(luf, xf);
#endif
        cvtF2DD(luf, &lu0, &lu1);
        // Last iteration(s) performed in double - if we had 22 bits, this gets us to 44 (~1e-15)
#if (GMX_SIMD_ACCURACY_BITS_SINGLE < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu0 = rsqrtIter(lu0, x0);
        lu1 = rsqrtIter(lu1, x1);
#endif
#if (GMX_SIMD_ACCURACY_BITS_SINGLE*2 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu0 = rsqrtIter(lu0, x0);
        lu1 = rsqrtIter(lu1, x1);
#endif
        *out0 = lu0;
        *out1 = lu1;
#else
        *out0 = invsqrt(x0);
        *out1 = invsqrt(x1);
#endif
    }
    
#if !GMX_SIMD_HAVE_NATIVE_RCP_ITER_DOUBLE
    /*! \brief Perform one Newton-Raphson iteration to improve 1/x for SIMD double.
     *
     * This is a low-level routine that should only be used by SIMD math routine
     * that evaluates the reciprocal.
     *
     *  \param lu Approximation of 1/x, typically obtained from lookup.
     *  \param x  The reference (starting) value x for which we want 1/x.
     *  \return   An improved approximation with roughly twice as many bits of accuracy.
     */
    static inline SimdDouble gmx_simdcall
    rcpIter(SimdDouble lu, SimdDouble x)
    {
        return lu*fnma(lu, x, SimdDouble(2.0));
    }
#endif
    
    /*! \brief Calculate 1/x for SIMD double.
     *
     *  \param x Argument with magnitude larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *
     *  \return 1/x. Result is undefined if your argument was invalid.
     */
    static inline SimdDouble gmx_simdcall
    inv(SimdDouble x)
    {
        SimdDouble lu = rcp(x);
#if (GMX_SIMD_RCP_BITS < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*2 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*4 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*8 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rcpIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief Division for SIMD doubles
     *
     * \param nom    Nominator
     * \param denom  Denominator, with magnitude in range (GMX_FLOAT_MIN,GMX_FLOAT_MAX).
     *               For single precision this is equivalent to a nonzero argument,
     *               but in double precision it adds an extra restriction since
     *               the first lookup step might have to be performed in single
     *               precision on some architectures. Note that the responsibility
     *               for checking falls on you - this routine does not check arguments.
     *
     * \return nom/denom
     *
     * \note This function does not use any masking to avoid problems with
     *       zero values in the denominator.
     */
    static inline SimdDouble gmx_simdcall
    operator/(SimdDouble nom, SimdDouble denom)
    {
        return nom*inv(denom);
    }
    
    
    /*! \brief Calculate 1/sqrt(x) for masked entries of SIMD double.
     *
     *  This routine only evaluates 1/sqrt(x) for elements for which mask is true.
     *  Illegal values in the masked-out elements will not lead to
     *  floating-point exceptions.
     *
     *  \param x Argument that must be larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX for masked-in entries.
     *           See \ref invsqrt for the discussion about argument restrictions.
     *  \param m Mask
     *  \return 1/sqrt(x). Result is undefined if your argument was invalid or
     *          entry was not masked, and 0.0 for masked-out entries.
     */
    static inline SimdDouble
    maskzInvsqrt(SimdDouble x, SimdDBool m)
    {
        SimdDouble lu = maskzRsqrt(x, m);
#if (GMX_SIMD_RSQRT_BITS < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*2 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*4 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*8 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rsqrtIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief Calculate 1/x for SIMD double, masked version.
     *
     *  \param x Argument with magnitude larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX for masked-in entries.
     *           See \ref invsqrt for the discussion about argument restrictions.
     *  \param m Mask
     *  \return 1/x for elements where m is true, or 0.0 for masked-out entries.
     */
    static inline SimdDouble gmx_simdcall
    maskzInv(SimdDouble x, SimdDBool m)
    {
        SimdDouble lu = maskzRcp(x, m);
#if (GMX_SIMD_RCP_BITS < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*2 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*4 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*8 < GMX_SIMD_ACCURACY_BITS_DOUBLE)
        lu = rcpIter(lu, x);
#endif
        return lu;
    }
    
    
    /*! \brief Calculate sqrt(x) for SIMD doubles.
     *
     *  \copydetails sqrt(SimdFloat)
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdDouble gmx_simdcall
    sqrt(SimdDouble x)
    {
        if (opt == MathOptimization::Safe)
        {
            // As we might use a float version of rsqrt, we mask out small values
            SimdDouble res = maskzInvsqrt(x, SimdDouble(GMX_FLOAT_MIN) < x);
            return res*x;
        }
        else
        {
            return x * invsqrt(x);
        }
    }
    
#if !GMX_SIMD_HAVE_NATIVE_LOG_DOUBLE
    /*! \brief SIMD double log(x). This is the natural logarithm.
     *
     * \param x Argument, should be >0.
     * \result The natural logarithm of x. Undefined if argument is invalid.
     */
    static inline SimdDouble gmx_simdcall
    log(SimdDouble x)
    {
        const SimdDouble  one(1.0);
        const SimdDouble  two(2.0);
        const SimdDouble  invsqrt2(1.0/std::sqrt(2.0));
        const SimdDouble  corr(0.693147180559945286226764);
        const SimdDouble  CL15(0.148197055177935105296783);
        const SimdDouble  CL13(0.153108178020442575739679);
        const SimdDouble  CL11(0.181837339521549679055568);
        const SimdDouble  CL9(0.22222194152736701733275);
        const SimdDouble  CL7(0.285714288030134544449368);
        const SimdDouble  CL5(0.399999999989941956712869);
        const SimdDouble  CL3(0.666666666666685503450651);
        const SimdDouble  CL1(2.0);
        SimdDouble        fExp, x2, p;
        SimdDBool         m;
        SimdDInt32        iExp;
        
        x     = frexp(x, &iExp);
        fExp  = cvtI2R(iExp);
        
        m     = x < invsqrt2;
        // Adjust to non-IEEE format for x<1/sqrt(2): exponent -= 1, mantissa *= 2.0
        fExp  = fExp - selectByMask(one, m);
        x     = x * blend(one, two, m);
        
        x     = (x-one) * inv( x+one );
        x2    = x * x;
        
        p     = fma(CL15, x2, CL13);
        p     = fma(p, x2, CL11);
        p     = fma(p, x2, CL9);
        p     = fma(p, x2, CL7);
        p     = fma(p, x2, CL5);
        p     = fma(p, x2, CL3);
        p     = fma(p, x2, CL1);
        p     = fma(p, x, corr * fExp);
        
        return p;
    }
#endif
    
#if !GMX_SIMD_HAVE_NATIVE_EXP2_DOUBLE
    /*! \brief SIMD double 2^x.
     *
     * \copydetails exp2(SimdFloat)
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdDouble gmx_simdcall
    exp2(SimdDouble x)
    {
        const SimdDouble  CE11(4.435280790452730022081181e-10);
        const SimdDouble  CE10(7.074105630863314448024247e-09);
        const SimdDouble  CE9(1.017819803432096698472621e-07);
        const SimdDouble  CE8(1.321543308956718799557863e-06);
        const SimdDouble  CE7(0.00001525273348995851746990884);
        const SimdDouble  CE6(0.0001540353046251466849082632);
        const SimdDouble  CE5(0.001333355814678995257307880);
        const SimdDouble  CE4(0.009618129107588335039176502);
        const SimdDouble  CE3(0.05550410866481992147457793);
        const SimdDouble  CE2(0.2402265069591015620470894);
        const SimdDouble  CE1(0.6931471805599453304615075);
        const SimdDouble  one(1.0);
        
        SimdDouble        intpart;
        SimdDouble        fexppart;
        SimdDouble        p;
        
        // Large negative values are valid arguments to exp2(), so there are two
        // things we need to account for:
        // 1. When the exponents reaches -1023, the (biased) exponent field will be
        //    zero and we can no longer multiply with it. There are special IEEE
        //    formats to handle this range, but for now we have to accept that
        //    we cannot handle those arguments. If input value becomes even more
        //    negative, it will start to loop and we would end up with invalid
        //    exponents. Thus, we need to limit or mask this.
        // 2. For VERY large negative values, we will have problems that the
        //    subtraction to get the fractional part loses accuracy, and then we
        //    can end up with overflows in the polynomial.
        //
        // For now, we handle this by forwarding the math optimization setting to
        // ldexp, where the routine will return zero for very small arguments.
        //
        // However, before doing that we need to make sure we do not call cvtR2I
        // with an argument that is so negative it cannot be converted to an integer.
        if (opt == MathOptimization::Safe)
        {
            x         = max(x, SimdDouble(std::numeric_limits<std::int32_t>::lowest()));
        }
        
        fexppart  = ldexp<opt>(one, cvtR2I(x));
        intpart   = round(x);
        x         = x - intpart;
        
        p         = fma(CE11, x, CE10);
        p         = fma(p, x, CE9);
        p         = fma(p, x, CE8);
        p         = fma(p, x, CE7);
        p         = fma(p, x, CE6);
        p         = fma(p, x, CE5);
        p         = fma(p, x, CE4);
        p         = fma(p, x, CE3);
        p         = fma(p, x, CE2);
        p         = fma(p, x, CE1);
        p         = fma(p, x, one);
        x         = p * fexppart;
        return x;
    }
#endif
    
#if !GMX_SIMD_HAVE_NATIVE_EXP_DOUBLE
    /*! \brief SIMD double exp(x).
     *
     * \copydetails exp(SimdFloat)
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdDouble gmx_simdcall
    exp(SimdDouble x)
    {
        const SimdDouble  argscale(1.44269504088896340735992468100);
        const SimdDouble  invargscale0(-0.69314718055966295651160180568695068359375);
        const SimdDouble  invargscale1(-2.8235290563031577122588448175013436025525412068e-13);
        const SimdDouble  CE12(2.078375306791423699350304e-09);
        const SimdDouble  CE11(2.518173854179933105218635e-08);
        const SimdDouble  CE10(2.755842049600488770111608e-07);
        const SimdDouble  CE9(2.755691815216689746619849e-06);
        const SimdDouble  CE8(2.480158383706245033920920e-05);
        const SimdDouble  CE7(0.0001984127043518048611841321);
        const SimdDouble  CE6(0.001388888889360258341755930);
        const SimdDouble  CE5(0.008333333332907368102819109);
        const SimdDouble  CE4(0.04166666666663836745814631);
        const SimdDouble  CE3(0.1666666666666796929434570);
        const SimdDouble  CE2(0.5);
        const SimdDouble  one(1.0);
        SimdDouble        fexppart;
        SimdDouble        intpart;
        SimdDouble        y, p;
        
        // Large negative values are valid arguments to exp2(), so there are two
        // things we need to account for:
        // 1. When the exponents reaches -1023, the (biased) exponent field will be
        //    zero and we can no longer multiply with it. There are special IEEE
        //    formats to handle this range, but for now we have to accept that
        //    we cannot handle those arguments. If input value becomes even more
        //    negative, it will start to loop and we would end up with invalid
        //    exponents. Thus, we need to limit or mask this.
        // 2. For VERY large negative values, we will have problems that the
        //    subtraction to get the fractional part loses accuracy, and then we
        //    can end up with overflows in the polynomial.
        //
        // For now, we handle this by forwarding the math optimization setting to
        // ldexp, where the routine will return zero for very small arguments.
        //
        // However, before doing that we need to make sure we do not call cvtR2I
        // with an argument that is so negative it cannot be converted to an integer
        // after being multiplied by argscale.
        
        if (opt == MathOptimization::Safe)
        {
            x         = max(x, SimdDouble(std::numeric_limits<std::int32_t>::lowest())/argscale);
        }
        
        y         = x * argscale;
        
        fexppart  = ldexp<opt>(one, cvtR2I(y));
        intpart   = round(y);
        
        // Extended precision arithmetics
        x         = fma(invargscale0, intpart, x);
        x         = fma(invargscale1, intpart, x);
        
        p         = fma(CE12, x, CE11);
        p         = fma(p, x, CE10);
        p         = fma(p, x, CE9);
        p         = fma(p, x, CE8);
        p         = fma(p, x, CE7);
        p         = fma(p, x, CE6);
        p         = fma(p, x, CE5);
        p         = fma(p, x, CE4);
        p         = fma(p, x, CE3);
        p         = fma(p, x, CE2);
        p         = fma(p, x * x, x);
        x         = fma(p, fexppart, fexppart);
        
        return x;
    }
#endif
    
    /*! \brief SIMD double asin(x).
     *
     * \param x The argument to evaluate atan for
     * \result Atan(x), same argument/value range as standard math library.
     */
    static inline SimdDouble gmx_simdcall
    atan(SimdDouble x)
    {
        // Same algorithm as cephes library
        const SimdDouble limit1(0.66);
        const SimdDouble limit2(2.41421356237309504880);
        const SimdDouble quarterpi(M_PI/4.0);
        const SimdDouble halfpi(M_PI/2.0);
        const SimdDouble mone(-1.0);
        const SimdDouble morebits1(0.5*6.123233995736765886130E-17);
        const SimdDouble morebits2(6.123233995736765886130E-17);
        
        const SimdDouble P4(-8.750608600031904122785E-1);
        const SimdDouble P3(-1.615753718733365076637E1);
        const SimdDouble P2(-7.500855792314704667340E1);
        const SimdDouble P1(-1.228866684490136173410E2);
        const SimdDouble P0(-6.485021904942025371773E1);
        
        const SimdDouble Q4(2.485846490142306297962E1);
        const SimdDouble Q3(1.650270098316988542046E2);
        const SimdDouble Q2(4.328810604912902668951E2);
        const SimdDouble Q1(4.853903996359136964868E2);
        const SimdDouble Q0(1.945506571482613964425E2);
        
        SimdDouble       y, xabs, t1, t2;
        SimdDouble       z, z2;
        SimdDouble       P_A, P_B, Q_A, Q_B;
        SimdDBool        mask1, mask2;
        
        xabs   = abs(x);
        
        mask1  = (limit1 < xabs);
        mask2  = (limit2 < xabs);
        
        t1     = (xabs + mone) * maskzInv(xabs - mone, mask1);
        t2     = mone * maskzInv(xabs, mask2);
        
        y      = selectByMask(quarterpi, mask1);
        y      = blend(y, halfpi, mask2);
        xabs   = blend(xabs, t1, mask1);
        xabs   = blend(xabs, t2, mask2);
        
        z      = xabs * xabs;
        z2     = z * z;
        
        P_A    = fma(P4, z2, P2);
        P_B    = fma(P3, z2, P1);
        P_A    = fma(P_A, z2, P0);
        P_A    = fma(P_B, z, P_A);
        
        // Q_A = z2
        Q_B    = fma(Q4, z2, Q2);
        Q_A    = z2 + Q3;
        Q_A    = fma(Q_A, z2, Q1);
        Q_B    = fma(Q_B, z2, Q0);
        Q_A    = fma(Q_A, z, Q_B);
        
        z      = z * P_A;
        z      = z * inv(Q_A);
        z      = fma(z, xabs, xabs);
        
        t1     = selectByMask(morebits1, mask1);
        t1     = blend(t1, morebits2, mask2);
        
        z      = z + t1;
        y      = y + z;
        
        y      = copysign(y, x);
        
        return y;
    }
    
    /*! \brief SIMD double atan2(y,x).
     *
     * \param y Y component of vector, any quartile
     * \param x X component of vector, any quartile
     * \result Atan(y,x), same argument/value range as standard math library.
     *
     * \note This routine should provide correct results for all finite
     * non-zero or positive-zero arguments. However, negative zero arguments will
     * be treated as positive zero, which means the return value will deviate from
     * the standard math library atan2(y,x) for those cases. That should not be
     * of any concern in Gromacs, and in particular it will not affect calculations
     * of angles from vectors.
     */
    static inline SimdDouble gmx_simdcall
    atan2(SimdDouble y, SimdDouble x)
    {
        const SimdDouble pi(M_PI);
        const SimdDouble halfpi(M_PI/2.0);
        SimdDouble       xinv, p, aoffset;
        SimdDBool        mask_xnz, mask_ynz, mask_xlt0, mask_ylt0;
        
        mask_xnz  = x != setZero();
        mask_ynz  = y != setZero();
        mask_xlt0 = (x < setZero());
        mask_ylt0 = (y < setZero());
        
        aoffset   = selectByNotMask(halfpi, mask_xnz);
        aoffset   = selectByMask(aoffset, mask_ynz);
        
        aoffset   = blend(aoffset, pi, mask_xlt0);
        aoffset   = blend(aoffset, -aoffset, mask_ylt0);
        
        xinv      = maskzInv(x, mask_xnz);
        p         = y * xinv;
        p         = atan(p);
        p         = p + aoffset;
        
        return p;
    }
    
    /*! \} */
    
    
    /*! \name SIMD math functions for double prec. data, single prec. accuracy
     *
     *  \note In some cases we do not need full double accuracy of individual
     *        SIMD math functions, although the data is stored in double precision
     *        SIMD registers. This might be the case for special algorithms, or
     *        if the architecture does not support single precision.
     *        Since the full double precision evaluation of math functions
     *        typically require much more expensive polynomial approximations
     *        these functions implement the algorithms used in the single precision
     *        SIMD math functions, but they operate on double precision
     *        SIMD variables.
     *
     *  \{
     */
    
    /*********************************************************************
     * SIMD MATH FUNCTIONS WITH DOUBLE PREC. DATA, SINGLE PREC. ACCURACY *
     *********************************************************************/
    
    /*! \brief Calculate 1/sqrt(x) for SIMD double, but in single accuracy.
     *
     *  \param x Argument that must be larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *
     *  \return 1/sqrt(x). Result is undefined if your argument was invalid.
     */
    static inline SimdDouble gmx_simdcall
    invsqrtSingleAccuracy(SimdDouble x)
    {
        SimdDouble lu = rsqrt(x);
#if (GMX_SIMD_RSQRT_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief 1/sqrt(x) for masked-in entries of SIMD double, but in single accuracy.
     *
     *  This routine only evaluates 1/sqrt(x) for elements for which mask is true.
     *  Illegal values in the masked-out elements will not lead to
     *  floating-point exceptions.
     *
     *  \param x Argument that must be larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *
     *  \param m Mask
     *  \return 1/sqrt(x). Result is undefined if your argument was invalid or
     *          entry was not masked, and 0.0 for masked-out entries.
     */
    static inline SimdDouble
    maskzInvsqrtSingleAccuracy(SimdDouble x, SimdDBool m)
    {
        SimdDouble lu = maskzRsqrt(x, m);
#if (GMX_SIMD_RSQRT_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
#if (GMX_SIMD_RSQRT_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rsqrtIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief Calculate 1/sqrt(x) for two SIMD doubles, but single accuracy.
     *
     * \param x0  First set of arguments, x0 must be in single range (see below).
     * \param x1  Second set of arguments, x1 must be in single range (see below).
     * \param[out] out0  Result 1/sqrt(x0)
     * \param[out] out1  Result 1/sqrt(x1)
     *
     *  In particular for double precision we can sometimes calculate square root
     *  pairs slightly faster by using single precision until the very last step.
     *
     * \note Both arguments must be larger than GMX_FLOAT_MIN and smaller than
     *       GMX_FLOAT_MAX, i.e. within the range of single precision.
     *       For the single precision implementation this is obviously always
     *       true for positive values, but for double precision it adds an
     *       extra restriction since the first lookup step might have to be
     *       performed in single precision on some architectures. Note that the
     *       responsibility for checking falls on you - this routine does not
     *       check arguments.
     */
    static inline void gmx_simdcall
    invsqrtPairSingleAccuracy(SimdDouble x0,    SimdDouble x1,
                              SimdDouble *out0, SimdDouble *out1)
    {
#if GMX_SIMD_HAVE_FLOAT && (GMX_SIMD_FLOAT_WIDTH == 2*GMX_SIMD_DOUBLE_WIDTH) && (GMX_SIMD_RSQRT_BITS < 22)
        SimdFloat  xf  = cvtDD2F(x0, x1);
        SimdFloat  luf = rsqrt(xf);
        SimdDouble lu0, lu1;
        // Intermediate target is single - mantissa+1 bits
#if (GMX_SIMD_RSQRT_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        luf = rsqrtIter(luf, xf);
#endif
#if (GMX_SIMD_RSQRT_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        luf = rsqrtIter(luf, xf);
#endif
#if (GMX_SIMD_RSQRT_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        luf = rsqrtIter(luf, xf);
#endif
        cvtF2DD(luf, &lu0, &lu1);
        // We now have single-precision accuracy values in lu0/lu1
        *out0 = lu0;
        *out1 = lu1;
#else
        *out0 = invsqrtSingleAccuracy(x0);
        *out1 = invsqrtSingleAccuracy(x1);
#endif
    }
    
    /*! \brief Calculate 1/x for SIMD double, but in single accuracy.
     *
     *  \param x Argument with magnitude larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *
     *  \return 1/x. Result is undefined if your argument was invalid.
     */
    static inline SimdDouble gmx_simdcall
    invSingleAccuracy(SimdDouble x)
    {
        SimdDouble lu = rcp(x);
#if (GMX_SIMD_RCP_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
        return lu;
    }
    
    /*! \brief 1/x for masked entries of SIMD double, single accuracy.
     *
     *  \param x Argument with magnitude larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *
     *  \param m Mask
     *  \return 1/x for elements where m is true, or 0.0 for masked-out entries.
     */
    static inline SimdDouble gmx_simdcall
    maskzInvSingleAccuracy(SimdDouble x, SimdDBool m)
    {
        SimdDouble lu = maskzRcp(x, m);
#if (GMX_SIMD_RCP_BITS < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*2 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
#if (GMX_SIMD_RCP_BITS*4 < GMX_SIMD_ACCURACY_BITS_SINGLE)
        lu = rcpIter(lu, x);
#endif
        return lu;
    }
    
    
    /*! \brief Calculate sqrt(x) (correct for 0.0) for SIMD double, with single accuracy.
     *
     *  \copydetails sqrt(SimdFloat)
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdDouble gmx_simdcall
    sqrtSingleAccuracy(SimdDouble x)
    {
        if (opt == MathOptimization::Safe)
        {
            SimdDouble res = maskzInvsqrt(x, SimdDouble(GMX_FLOAT_MIN) < x);
            return res*x;
        }
        else
        {
            return x * invsqrtSingleAccuracy(x);
        }
    }
    
    
    /*! \brief SIMD log(x). Double precision SIMD data, single accuracy.
     *
     * \param x Argument, should be >0.
     * \result The natural logarithm of x. Undefined if argument is invalid.
     */
    static inline SimdDouble gmx_simdcall
    logSingleAccuracy(SimdDouble x)
    {
        const SimdDouble  one(1.0);
        const SimdDouble  two(2.0);
        const SimdDouble  sqrt2(std::sqrt(2.0));
        const SimdDouble  corr(0.693147180559945286226764);
        const SimdDouble  CL9(0.2371599674224853515625);
        const SimdDouble  CL7(0.285279005765914916992188);
        const SimdDouble  CL5(0.400005519390106201171875);
        const SimdDouble  CL3(0.666666567325592041015625);
        const SimdDouble  CL1(2.0);
        SimdDouble        fexp, x2, p;
        SimdDInt32        iexp;
        SimdDBool         mask;
        
        x     = frexp(x, &iexp);
        fexp  = cvtI2R(iexp);
        
        mask  = (x < sqrt2);
        // Adjust to non-IEEE format for x<sqrt(2): exponent -= 1, mantissa *= 2.0
        fexp  = fexp - selectByMask(one, mask);
        x     = x * blend(one, two, mask);
        
        x     = (x - one) * invSingleAccuracy( x + one );
        x2    = x * x;
        
        p     = fma(CL9, x2, CL7);
        p     = fma(p, x2, CL5);
        p     = fma(p, x2, CL3);
        p     = fma(p, x2, CL1);
        p     = fma(p, x, corr * fexp);
        
        return p;
    }
    
    /*! \brief SIMD 2^x. Double precision SIMD, single accuracy.
     *
     * \copydetails exp2(SimdFloat)
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdDouble gmx_simdcall
    exp2SingleAccuracy(SimdDouble x)
    {
        const SimdDouble  CC6(0.0001534581200287996416911311);
        const SimdDouble  CC5(0.001339993121934088894618990);
        const SimdDouble  CC4(0.009618488957115180159497841);
        const SimdDouble  CC3(0.05550328776964726865751735);
        const SimdDouble  CC2(0.2402264689063408646490722);
        const SimdDouble  CC1(0.6931472057372680777553816);
        const SimdDouble  one(1.0);
        
        SimdDouble        intpart;
        SimdDouble        p;
        SimdDInt32        ix;
        
        // Large negative values are valid arguments to exp2(), so there are two
        // things we need to account for:
        // 1. When the exponents reaches -1023, the (biased) exponent field will be
        //    zero and we can no longer multiply with it. There are special IEEE
        //    formats to handle this range, but for now we have to accept that
        //    we cannot handle those arguments. If input value becomes even more
        //    negative, it will start to loop and we would end up with invalid
        //    exponents. Thus, we need to limit or mask this.
        // 2. For VERY large negative values, we will have problems that the
        //    subtraction to get the fractional part loses accuracy, and then we
        //    can end up with overflows in the polynomial.
        //
        // For now, we handle this by forwarding the math optimization setting to
        // ldexp, where the routine will return zero for very small arguments.
        //
        // However, before doing that we need to make sure we do not call cvtR2I
        // with an argument that is so negative it cannot be converted to an integer.
        if (opt == MathOptimization::Safe)
        {
            x         = max(x, SimdDouble(std::numeric_limits<std::int32_t>::lowest()));
        }
        
        ix        = cvtR2I(x);
        intpart   = round(x);
        x         = x - intpart;
        
        p         = fma(CC6, x, CC5);
        p         = fma(p, x, CC4);
        p         = fma(p, x, CC3);
        p         = fma(p, x, CC2);
        p         = fma(p, x, CC1);
        p         = fma(p, x, one);
        x         = ldexp<opt>(p, ix);
        
        return x;
    }
    
    
    
    /*! \brief SIMD exp(x). Double precision SIMD, single accuracy.
     *
     * \copydetails exp(SimdFloat)
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdDouble gmx_simdcall
    expSingleAccuracy(SimdDouble x)
    {
        const SimdDouble  argscale(1.44269504088896341);
        // Lower bound: Clamp args that would lead to an IEEE fp exponent below -1023.
        const SimdDouble  smallArgLimit(-709.0895657128);
        const SimdDouble  invargscale(-0.69314718055994528623);
        const SimdDouble  CC4(0.00136324646882712841033936);
        const SimdDouble  CC3(0.00836596917361021041870117);
        const SimdDouble  CC2(0.0416710823774337768554688);
        const SimdDouble  CC1(0.166665524244308471679688);
        const SimdDouble  CC0(0.499999850988388061523438);
        const SimdDouble  one(1.0);
        SimdDouble        intpart;
        SimdDouble        y, p;
        SimdDInt32        iy;
        
        // Large negative values are valid arguments to exp2(), so there are two
        // things we need to account for:
        // 1. When the exponents reaches -1023, the (biased) exponent field will be
        //    zero and we can no longer multiply with it. There are special IEEE
        //    formats to handle this range, but for now we have to accept that
        //    we cannot handle those arguments. If input value becomes even more
        //    negative, it will start to loop and we would end up with invalid
        //    exponents. Thus, we need to limit or mask this.
        // 2. For VERY large negative values, we will have problems that the
        //    subtraction to get the fractional part loses accuracy, and then we
        //    can end up with overflows in the polynomial.
        //
        // For now, we handle this by forwarding the math optimization setting to
        // ldexp, where the routine will return zero for very small arguments.
        //
        // However, before doing that we need to make sure we do not call cvtR2I
        // with an argument that is so negative it cannot be converted to an integer
        // after being multiplied by argscale.
        
        if (opt == MathOptimization::Safe)
        {
            x         = max(x, SimdDouble(std::numeric_limits<std::int32_t>::lowest())/argscale);
        }
        
        y         = x * argscale;
        
        iy        = cvtR2I(y);
        intpart   = round(y);        // use same rounding algorithm here
        
        // Extended precision arithmetics not needed since
        // we have double precision and only need single accuracy.
        x         = fma(invargscale, intpart, x);
        
        p         = fma(CC4, x, CC3);
        p         = fma(p, x, CC2);
        p         = fma(p, x, CC1);
        p         = fma(p, x, CC0);
        p         = fma(x*x, p, x);
        p         = p + one;
        x         = ldexp<opt>(p, iy);
        return x;
    }
    
    /*! \brief SIMD asin(x). Double precision SIMD data, single accuracy.
     *
     * \param x The argument to evaluate atan for
     * \result Atan(x), same argument/value range as standard math library.
     */
    static inline SimdDouble gmx_simdcall
    atanSingleAccuracy(SimdDouble x)
    {
        const SimdDouble halfpi(M_PI/2);
        const SimdDouble CA17(0.002823638962581753730774);
        const SimdDouble CA15(-0.01595690287649631500244);
        const SimdDouble CA13(0.04250498861074447631836);
        const SimdDouble CA11(-0.07489009201526641845703);
        const SimdDouble CA9(0.1063479334115982055664);
        const SimdDouble CA7(-0.1420273631811141967773);
        const SimdDouble CA5(0.1999269574880599975585);
        const SimdDouble CA3(-0.3333310186862945556640);
        SimdDouble       x2, x3, x4, pA, pB;
        SimdDBool        mask, mask2;
        
        mask  = (x < setZero());
        x     = abs(x);
        mask2 = (SimdDouble(1.0) < x);
        x     = blend(x, maskzInvSingleAccuracy(x, mask2), mask2);
        
        x2    = x * x;
        x3    = x2 * x;
        x4    = x2 * x2;
        pA    = fma(CA17, x4, CA13);
        pB    = fma(CA15, x4, CA11);
        pA    = fma(pA, x4, CA9);
        pB    = fma(pB, x4, CA7);
        pA    = fma(pA, x4, CA5);
        pB    = fma(pB, x4, CA3);
        pA    = fma(pA, x2, pB);
        pA    = fma(pA, x3, x);
        
        pA    = blend(pA, halfpi - pA, mask2);
        pA    = blend(pA, -pA, mask);
        
        return pA;
    }
    
    /*! \brief SIMD atan2(y,x). Double precision SIMD data, single accuracy.
     *
     * \param y Y component of vector, any quartile
     * \param x X component of vector, any quartile
     * \result Atan(y,x), same argument/value range as standard math library.
     *
     * \note This routine should provide correct results for all finite
     * non-zero or positive-zero arguments. However, negative zero arguments will
     * be treated as positive zero, which means the return value will deviate from
     * the standard math library atan2(y,x) for those cases. That should not be
     * of any concern in Gromacs, and in particular it will not affect calculations
     * of angles from vectors.
     */
    static inline SimdDouble gmx_simdcall
    atan2SingleAccuracy(SimdDouble y, SimdDouble x)
    {
        const SimdDouble pi(M_PI);
        const SimdDouble halfpi(M_PI/2.0);
        SimdDouble       xinv, p, aoffset;
        SimdDBool        mask_xnz, mask_ynz, mask_xlt0, mask_ylt0;
        
        mask_xnz  = x != setZero();
        mask_ynz  = y != setZero();
        mask_xlt0 = (x < setZero());
        mask_ylt0 = (y < setZero());
        
        aoffset   = selectByNotMask(halfpi, mask_xnz);
        aoffset   = selectByMask(aoffset, mask_ynz);
        
        aoffset   = blend(aoffset, pi, mask_xlt0);
        aoffset   = blend(aoffset, -aoffset, mask_ylt0);
        
        xinv      = maskzInvSingleAccuracy(x, mask_xnz);
        p         = y * xinv;
        p         = atanSingleAccuracy(p);
        p         = p + aoffset;
        
        return p;
    }
    
#endif
    
#if GMX_SIMD_HAVE_FLOAT
    /*! \brief Calculate 1/sqrt(x) for SIMD float, only targeting single accuracy.
     *
     *  \param x Argument that must be larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *  \return  1/sqrt(x). Result is undefined if your argument was invalid.
     */
    static inline SimdFloat gmx_simdcall
    invsqrtSingleAccuracy(SimdFloat x)
    {
        return invsqrt(x);
    }
    
    /*! \brief Calculate 1/sqrt(x) for masked SIMD floats, only targeting single accuracy.
     *
     *  This routine only evaluates 1/sqrt(x) for elements for which mask is true.
     *  Illegal values in the masked-out elements will not lead to
     *  floating-point exceptions.
     *
     *  \param x Argument that must be larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *  \param m Mask
     *  \return  1/sqrt(x). Result is undefined if your argument was invalid or
     *           entry was not masked, and 0.0 for masked-out entries.
     */
    static inline SimdFloat
    maskzInvsqrtSingleAccuracy(SimdFloat x, SimdFBool m)
    {
        return maskzInvsqrt(x, m);
    }
    
    /*! \brief Calculate 1/sqrt(x) for two SIMD floats, only targeting single accuracy.
     *
     * \param x0  First set of arguments, x0 must be in single range (see below).
     * \param x1  Second set of arguments, x1 must be in single range (see below).
     * \param[out] out0  Result 1/sqrt(x0)
     * \param[out] out1  Result 1/sqrt(x1)
     *
     *  In particular for double precision we can sometimes calculate square root
     *  pairs slightly faster by using single precision until the very last step.
     *
     * \note Both arguments must be larger than GMX_FLOAT_MIN and smaller than
     *       GMX_FLOAT_MAX, i.e. within the range of single precision.
     *       For the single precision implementation this is obviously always
     *       true for positive values, but for double precision it adds an
     *       extra restriction since the first lookup step might have to be
     *       performed in single precision on some architectures. Note that the
     *       responsibility for checking falls on you - this routine does not
     *       check arguments.
     */
    static inline void gmx_simdcall
    invsqrtPairSingleAccuracy(SimdFloat x0,    SimdFloat x1,
                              SimdFloat *out0, SimdFloat *out1)
    {
        return invsqrtPair(x0, x1, out0, out1);
    }
    
    /*! \brief Calculate 1/x for SIMD float, only targeting single accuracy.
     *
     *  \param x Argument with magnitude larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *  \return  1/x. Result is undefined if your argument was invalid.
     */
    static inline SimdFloat gmx_simdcall
    invSingleAccuracy(SimdFloat x)
    {
        return inv(x);
    }
    
    
    /*! \brief Calculate 1/x for masked SIMD floats, only targeting single accuracy.
     *
     *  \param x Argument with magnitude larger than GMX_FLOAT_MIN and smaller than
     *           GMX_FLOAT_MAX, i.e. within the range of single precision.
     *           For the single precision implementation this is obviously always
     *           true for positive values, but for double precision it adds an
     *           extra restriction since the first lookup step might have to be
     *           performed in single precision on some architectures. Note that the
     *           responsibility for checking falls on you - this routine does not
     *           check arguments.
     *  \param m Mask
     *  \return  1/x for elements where m is true, or 0.0 for masked-out entries.
     */
    static inline SimdFloat
    maskzInvSingleAccuracy(SimdFloat x, SimdFBool m)
    {
        return maskzInv(x, m);
    }
    
    /*! \brief Calculate sqrt(x) for SIMD float, always targeting single accuracy.
     *
     * \copydetails sqrt(SimdFloat)
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdFloat gmx_simdcall
    sqrtSingleAccuracy(SimdFloat x)
    {
        return sqrt<opt>(x);
    }
    
    /*! \brief SIMD float log(x), only targeting single accuracy. This is the natural logarithm.
     *
     * \param x Argument, should be >0.
     * \result The natural logarithm of x. Undefined if argument is invalid.
     */
    static inline SimdFloat gmx_simdcall
    logSingleAccuracy(SimdFloat x)
    {
        return log(x);
    }
    
    /*! \brief SIMD float 2^x, only targeting single accuracy.
     *
     * \copydetails exp2(SimdFloat)
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdFloat gmx_simdcall
    exp2SingleAccuracy(SimdFloat x)
    {
        return exp2<opt>(x);
    }
    
    /*! \brief SIMD float e^x, only targeting single accuracy.
     *
     * \copydetails exp(SimdFloat)
     */
    template <MathOptimization opt = MathOptimization::Safe>
    static inline SimdFloat gmx_simdcall
    expSingleAccuracy(SimdFloat x)
    {
        return exp<opt>(x);
    }
    
    /*! \brief SIMD float atan(x), only targeting single accuracy.
     *
     * \param x The argument to evaluate atan for
     * \result Atan(x), same argument/value range as standard math library.
     */
    static inline SimdFloat gmx_simdcall
    atanSingleAccuracy(SimdFloat x)
    {
        return atan(x);
    }
    
    /*! \brief SIMD float atan2(y,x), only targeting single accuracy.
     *
     * \param y Y component of vector, any quartile
     * \param x X component of vector, any quartile
     * \result Atan(y,x), same argument/value range as standard math library.
     *
     * \note This routine should provide correct results for all finite
     * non-zero or positive-zero arguments. However, negative zero arguments will
     * be treated as positive zero, which means the return value will deviate from
     * the standard math library atan2(y,x) for those cases. That should not be
     * of any concern in Gromacs, and in particular it will not affect calculations
     * of angles from vectors.
     */
    static inline SimdFloat gmx_simdcall
    atan2SingleAccuracy(SimdFloat y, SimdFloat x)
    {
        return atan2(y, x);
    }
    
#endif // GMX_SIMD_HAVE_FLOAT
    

    /*! \}   end of addtogroup module_simd */
    /*! \endcond  end of condition libabl */
    
#endif // GMX_SIMD
    
}      // namespace gmx

#endif // GMX_SIMD_SIMD_MATH_H

#endif
