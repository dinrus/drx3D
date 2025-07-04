/*
 * Plain Intel IA32 assembly implementations of PortAudio sample converter functions.
 * Copyright (c) 1999-2002 Ross Bencina, Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however,
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the
 * license above.
 */

/** @file
 @ingroup win_src
*/

#include "pa_x86_plain_converters.h"

#include "../pa_converters.h"
#include "../pa_dither.h"

/*
    the main reason these versions are faster than the equivalent C versions
    is that float -> i32 casting is expensive in C on x86 because the rounding
    mode needs to be changed for every cast. these versions only set
    the rounding mode once outside the loop.

    small additional speed gains are made by the way that clamping is
    implemented.

TODO:
    o- inline dither code
    o- implement Dither only (no-clip) versions
    o- implement int8 and u8 versions
    o- test thoroughly

    o- the packed 24 bit functions could benefit from unrolling and avoiding
        byte and word sized register access.
*/

/* -------------------------------------------------------------------------- */

/*
#define PA_CLIP_( val, min, max )\
    { val = ((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)); }
*/

/*
    the following notes were used to determine whether a floating point
    value should be saturated (ie >1 or <-1) by loading it into an integer
    register. these should be rewritten so that they make sense.

    an ieee floating point value

    1.xxxxxxxxxxxxxxxxxxxx?


    is less than  or equal to 1 and greater than or equal to -1 either:

        if the mantissa is 0 and the unbiased exponent is 0

        OR

        if the unbiased exponent < 0

    this translates to:

        if the mantissa is 0 and the biased exponent is 7F

        or

        if the biased exponent is less than 7F


    therefore the value is greater than 1 or less than -1 if

        the mantissa is not 0 and the biased exponent is 7F

        or

        if the biased exponent is greater than 7F


    in other words, if we mask out the sign bit, the value is
    greater than 1 or less than -1 if its integer representation is greater than:

    0 01111111 0000 0000 0000 0000 0000 000

    0011 1111 1000 0000 0000 0000 0000 0000 => 0x3F800000
*/

#if defined(_WIN64) || defined(_WIN32_WCE)

/*
    -EMT64/AMD64 uses different asm
    -VC2005 doesn't allow _WIN64 with inline assembly either!
 */
void PaUtil_InitializeX86PlainConverters( void )
{
}

#else

/* -------------------------------------------------------------------------- */

static const short fpuControlWord_ = 0x033F; /*round to nearest, 64 bit precision, all exceptions masked*/
static const double int32Scaler_ = 0x7FFFFFFF;
static const double ditheredInt32Scaler_ = 0x7FFFFFFE;
static const double int24Scaler_ = 0x7FFFFF;
static const double ditheredInt24Scaler_ = 0x7FFFFE;
static const double int16Scaler_ = 0x7FFF;
static const double ditheredInt16Scaler_ = 0x7FFE;

#define PA_DITHER_BITS_   (15)
/* Multiply by PA_FLOAT_DITHER_SCALE_ to get a float between -2.0 and +1.99999 */
#define PA_FLOAT_DITHER_SCALE_  (1.0F / ((1<<PA_DITHER_BITS_)-1))
static const float const_float_dither_scale_ = PA_FLOAT_DITHER_SCALE_;
#define PA_DITHER_SHIFT_  ((32 - PA_DITHER_BITS_) + 1)

/* -------------------------------------------------------------------------- */

static void Float32_To_Int32(
    uk destinationBuffer, i32 destinationStride,
    uk sourceBuffer, i32 sourceStride,
    u32 count, PaUtilTriangularDitherGenerator *ditherGenerator )
{
/*
    float *src = (float*)sourceBuffer;
    i64 *dest =  (i64*)destinationBuffer;
    (void)ditherGenerator; // unused parameter

    while( count-- )
    {
        // REVIEW
        double scaled = *src * 0x7FFFFFFF;
        *dest = (i64) scaled;

        src += sourceStride;
        dest += destinationStride;
    }
*/

    short savedFpuControlWord;

    (void) ditherGenerator; /* unused parameter */


    __asm{
        // esi -> source ptr
        // eax -> source byte stride
        // edi -> destination ptr
        // ebx -> destination byte stride
        // ecx -> source end ptr
        // edx -> temp

        mov     esi, sourceBuffer

        mov     edx, 4                  // sizeof float32 and i32
        mov     eax, sourceStride
        imul    eax, edx

        mov     ecx, count
        imul    ecx, eax
        add     ecx, esi

        mov     edi, destinationBuffer

        mov     ebx, destinationStride
        imul    ebx, edx

        fwait
        fstcw   savedFpuControlWord
        fldcw   fpuControlWord_

        fld     int32Scaler_            // stack:  (i32)0x7FFFFFFF

    Float32_To_Int32_loop:

        // load unscaled value into st(0)
        fld     dword ptr [esi]         // stack:  value, (i32)0x7FFFFFFF
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        fmul    st(0), st(1)            // st(0) *= st(1), stack:  value*0x7FFFFFFF, (i32)0x7FFFFFFF
        /*
            note: we could store to a temporary qword here which would cause
            wraparound distortion instead of i32 indefinite 0x10. that would
            be more work, and given that not enabling clipping is only advisable
            when you know that your signal isn't going to clip it isn't worth it.
        */
        fistp   dword ptr [edi]         // pop st(0) into dest, stack:  (i32)0x7FFFFFFF

        add     edi, ebx                // increment destination ptr
        //lea     edi, [edi+ebx]

        cmp     esi, ecx                // has src ptr reached end?
        jne     Float32_To_Int32_loop

        ffree   st(0)
        fincstp

        fwait
        fnclex
        fldcw   savedFpuControlWord
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int32_Clip(
    uk destinationBuffer, i32 destinationStride,
    uk sourceBuffer, i32 sourceStride,
    u32 count, PaUtilTriangularDitherGenerator *ditherGenerator )
{
/*
    float *src = (float*)sourceBuffer;
    i64 *dest =  (i64*)destinationBuffer;
    (void) ditherGenerator; // unused parameter

    while( count-- )
    {
        // REVIEW
        double scaled = *src * 0x7FFFFFFF;
        PA_CLIP_( scaled, -2147483648., 2147483647.  );
        *dest = (i64) scaled;

        src += sourceStride;
        dest += destinationStride;
    }
*/

    short savedFpuControlWord;

    (void) ditherGenerator; /* unused parameter */

    __asm{
        // esi -> source ptr
        // eax -> source byte stride
        // edi -> destination ptr
        // ebx -> destination byte stride
        // ecx -> source end ptr
        // edx -> temp

        mov     esi, sourceBuffer

        mov     edx, 4                  // sizeof float32 and i32
        mov     eax, sourceStride
        imul    eax, edx

        mov     ecx, count
        imul    ecx, eax
        add     ecx, esi

        mov     edi, destinationBuffer

        mov     ebx, destinationStride
        imul    ebx, edx

        fwait
        fstcw   savedFpuControlWord
        fldcw   fpuControlWord_

        fld     int32Scaler_            // stack:  (i32)0x7FFFFFFF

    Float32_To_Int32_Clip_loop:

        mov     edx, dword ptr [esi]    // load floating point value into integer register

        and     edx, 0x7FFFFFFF         // mask off sign
        cmp     edx, 0x3F800000         // greater than 1.0 or less than -1.0

        jg      Float32_To_Int32_Clip_clamp

        // load unscaled value into st(0)
        fld     dword ptr [esi]         // stack:  value, (i32)0x7FFFFFFF
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        fmul    st(0), st(1)            // st(0) *= st(1), stack:  value*0x7FFFFFFF, (i32)0x7FFFFFFF
        fistp   dword ptr [edi]         // pop st(0) into dest, stack:  (i32)0x7FFFFFFF
        jmp     Float32_To_Int32_Clip_stored

    Float32_To_Int32_Clip_clamp:
        mov     edx, dword ptr [esi]    // load floating point value into integer register
        shr     edx, 31                 // move sign bit into bit 0
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        add     edx, 0x7FFFFFFF         // convert to maximum range integers
        mov     dword ptr [edi], edx

    Float32_To_Int32_Clip_stored:

        //add     edi, ebx              // increment destination ptr
        lea     edi, [edi+ebx]

        cmp     esi, ecx                // has src ptr reached end?
        jne     Float32_To_Int32_Clip_loop

        ffree   st(0)
        fincstp

        fwait
        fnclex
        fldcw   savedFpuControlWord
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int32_DitherClip(
    uk destinationBuffer, i32 destinationStride,
    uk sourceBuffer, i32 sourceStride,
    u32 count, PaUtilTriangularDitherGenerator *ditherGenerator )
{
    /*
    float *src = (float*)sourceBuffer;
    i64 *dest =  (i64*)destinationBuffer;

    while( count-- )
    {
        // REVIEW
        double dither  = PaUtil_GenerateFloatTriangularDither( ditherGenerator );
        // use smaller scaler to prevent overflow when we add the dither
        double dithered = ((double)*src * (2147483646.0)) + dither;
        PA_CLIP_( dithered, -2147483648., 2147483647.  );
        *dest = (i64) dithered;


        src += sourceStride;
        dest += destinationStride;
    }
    */

    short savedFpuControlWord;

    // spill storage:
    i64 sourceByteStride;
    i64 highpassedDither;

    // dither state:
    u64 ditherPrevious = ditherGenerator->previous;
    u64 ditherRandSeed1 = ditherGenerator->randSeed1;
    u64 ditherRandSeed2 = ditherGenerator->randSeed2;

    __asm{
        // esi -> source ptr
        // eax -> source byte stride
        // edi -> destination ptr
        // ebx -> destination byte stride
        // ecx -> source end ptr
        // edx -> temp

        mov     esi, sourceBuffer

        mov     edx, 4                  // sizeof float32 and i32
        mov     eax, sourceStride
        imul    eax, edx

        mov     ecx, count
        imul    ecx, eax
        add     ecx, esi

        mov     edi, destinationBuffer

        mov     ebx, destinationStride
        imul    ebx, edx

        fwait
        fstcw   savedFpuControlWord
        fldcw   fpuControlWord_

        fld     ditheredInt32Scaler_    // stack:  i32 scaler

    Float32_To_Int32_DitherClip_loop:

        mov     edx, dword ptr [esi]    // load floating point value into integer register

        and     edx, 0x7FFFFFFF         // mask off sign
        cmp     edx, 0x3F800000         // greater than 1.0 or less than -1.0

        jg      Float32_To_Int32_DitherClip_clamp

        // load unscaled value into st(0)
        fld     dword ptr [esi]         // stack:  value, i32 scaler
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        fmul    st(0), st(1)            // st(0) *= st(1), stack:  value*(i32 scaler), i32 scaler

        /*
        // call PaUtil_GenerateFloatTriangularDither with C calling convention
        mov     sourceByteStride, eax   // save eax
        mov     sourceEnd, ecx          // save ecx
        push    ditherGenerator         // pass ditherGenerator parameter on stack
        call    PaUtil_GenerateFloatTriangularDither  // stack:  dither, value*(i32 scaler), i32 scaler
        pop     edx                     // clear parameter off stack
        mov     ecx, sourceEnd          // restore ecx
        mov     eax, sourceByteStride   // restore eax
        */

    // generate dither
        mov     sourceByteStride, eax   // save eax
        mov     edx, 196314165
        mov     eax, ditherRandSeed1
        mul     edx                     // eax:edx = eax * 196314165
        //add     eax, 907633515
        lea     eax, [eax+907633515]
        mov     ditherRandSeed1, eax
        mov     edx, 196314165
        mov     eax, ditherRandSeed2
        mul     edx                     // eax:edx = eax * 196314165
        //add     eax, 907633515
        lea     eax, [eax+907633515]
        mov     edx, ditherRandSeed1
        shr     edx, PA_DITHER_SHIFT_
        mov     ditherRandSeed2, eax
        shr     eax, PA_DITHER_SHIFT_
        //add     eax, edx              // eax -> current
        lea     eax, [eax+edx]
        mov     edx, ditherPrevious
        neg     edx
        lea     edx, [eax+edx]          // highpass = current - previous
        mov     highpassedDither, edx
        mov     ditherPrevious, eax     // previous = current
        mov     eax, sourceByteStride   // restore eax
        fild    highpassedDither
        fmul    const_float_dither_scale_
    // end generate dither, dither signal in st(0)

        faddp   st(1), st(0)            // stack: dither + value*(i32 scaler), i32 scaler
        fistp   dword ptr [edi]         // pop st(0) into dest, stack:  i32 scaler
        jmp     Float32_To_Int32_DitherClip_stored

    Float32_To_Int32_DitherClip_clamp:
        mov     edx, dword ptr [esi]    // load floating point value into integer register
        shr     edx, 31                 // move sign bit into bit 0
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        add     edx, 0x7FFFFFFF         // convert to maximum range integers
        mov     dword ptr [edi], edx

    Float32_To_Int32_DitherClip_stored:

        //add     edi, ebx              // increment destination ptr
        lea     edi, [edi+ebx]

        cmp     esi, ecx                // has src ptr reached end?
        jne     Float32_To_Int32_DitherClip_loop

        ffree   st(0)
        fincstp

        fwait
        fnclex
        fldcw   savedFpuControlWord
    }

    ditherGenerator->previous = ditherPrevious;
    ditherGenerator->randSeed1 = ditherRandSeed1;
    ditherGenerator->randSeed2 = ditherRandSeed2;
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int24(
    uk destinationBuffer, i32 destinationStride,
    uk sourceBuffer, i32 sourceStride,
    u32 count, PaUtilTriangularDitherGenerator *ditherGenerator )
{
/*
    float *src = (float*)sourceBuffer;
    u8 *dest = (u8*)destinationBuffer;
    i64 temp;

    (void) ditherGenerator; // unused parameter

    while( count-- )
    {
        // convert to 32 bit and drop the low 8 bits
        double scaled = *src * 0x7FFFFFFF;
        temp = (i64) scaled;

        dest[0] = (u8)(temp >> 8);
        dest[1] = (u8)(temp >> 16);
        dest[2] = (u8)(temp >> 24);

        src += sourceStride;
        dest += destinationStride * 3;
    }
*/

    short savedFpuControlWord;

    i64 tempInt32;

    (void) ditherGenerator; /* unused parameter */

    __asm{
        // esi -> source ptr
        // eax -> source byte stride
        // edi -> destination ptr
        // ebx -> destination byte stride
        // ecx -> source end ptr
        // edx -> temp

        mov     esi, sourceBuffer

        mov     edx, 4                  // sizeof float32
        mov     eax, sourceStride
        imul    eax, edx

        mov     ecx, count
        imul    ecx, eax
        add     ecx, esi

        mov     edi, destinationBuffer

        mov     edx, 3                  // sizeof int24
        mov     ebx, destinationStride
        imul    ebx, edx

        fwait
        fstcw   savedFpuControlWord
        fldcw   fpuControlWord_

        fld     int24Scaler_            // stack:  (i32)0x7FFFFF

    Float32_To_Int24_loop:

        // load unscaled value into st(0)
        fld     dword ptr [esi]         // stack:  value, (i32)0x7FFFFF
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        fmul    st(0), st(1)            // st(0) *= st(1), stack:  value*0x7FFFFF, (i32)0x7FFFFF
        fistp   tempInt32               // pop st(0) into tempInt32, stack:  (i32)0x7FFFFF
        mov     edx, tempInt32

        mov     byte ptr [edi], DL
        shr     edx, 8
        //mov     byte ptr [edi+1], DL
        //mov     byte ptr [edi+2], DH
        mov     word ptr [edi+1], DX

        //add     edi, ebx              // increment destination ptr
        lea     edi, [edi+ebx]

        cmp     esi, ecx                // has src ptr reached end?
        jne     Float32_To_Int24_loop

        ffree   st(0)
        fincstp

        fwait
        fnclex
        fldcw   savedFpuControlWord
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int24_Clip(
    uk destinationBuffer, i32 destinationStride,
    uk sourceBuffer, i32 sourceStride,
    u32 count, PaUtilTriangularDitherGenerator *ditherGenerator )
{
/*
    float *src = (float*)sourceBuffer;
    u8 *dest = (u8*)destinationBuffer;
    i64 temp;

    (void) ditherGenerator; // unused parameter

    while( count-- )
    {
        // convert to 32 bit and drop the low 8 bits
        double scaled = *src * 0x7FFFFFFF;
        PA_CLIP_( scaled, -2147483648., 2147483647.  );
        temp = (i64) scaled;

        dest[0] = (u8)(temp >> 8);
        dest[1] = (u8)(temp >> 16);
        dest[2] = (u8)(temp >> 24);

        src += sourceStride;
        dest += destinationStride * 3;
    }
*/

    short savedFpuControlWord;

    i64 tempInt32;

    (void) ditherGenerator; /* unused parameter */

    __asm{
        // esi -> source ptr
        // eax -> source byte stride
        // edi -> destination ptr
        // ebx -> destination byte stride
        // ecx -> source end ptr
        // edx -> temp

        mov     esi, sourceBuffer

        mov     edx, 4                  // sizeof float32
        mov     eax, sourceStride
        imul    eax, edx

        mov     ecx, count
        imul    ecx, eax
        add     ecx, esi

        mov     edi, destinationBuffer

        mov     edx, 3                  // sizeof int24
        mov     ebx, destinationStride
        imul    ebx, edx

        fwait
        fstcw   savedFpuControlWord
        fldcw   fpuControlWord_

        fld     int24Scaler_            // stack:  (i32)0x7FFFFF

    Float32_To_Int24_Clip_loop:

        mov     edx, dword ptr [esi]    // load floating point value into integer register

        and     edx, 0x7FFFFFFF         // mask off sign
        cmp     edx, 0x3F800000         // greater than 1.0 or less than -1.0

        jg      Float32_To_Int24_Clip_clamp

        // load unscaled value into st(0)
        fld     dword ptr [esi]         // stack:  value, (i32)0x7FFFFF
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        fmul    st(0), st(1)            // st(0) *= st(1), stack:  value*0x7FFFFF, (i32)0x7FFFFF
        fistp   tempInt32               // pop st(0) into tempInt32, stack:  (i32)0x7FFFFF
        mov     edx, tempInt32
        jmp     Float32_To_Int24_Clip_store

    Float32_To_Int24_Clip_clamp:
        mov     edx, dword ptr [esi]    // load floating point value into integer register
        shr     edx, 31                 // move sign bit into bit 0
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        add     edx, 0x7FFFFF           // convert to maximum range integers

    Float32_To_Int24_Clip_store:

        mov     byte ptr [edi], DL
        shr     edx, 8
        //mov     byte ptr [edi+1], DL
        //mov     byte ptr [edi+2], DH
        mov     word ptr [edi+1], DX

        //add     edi, ebx              // increment destination ptr
        lea     edi, [edi+ebx]

        cmp     esi, ecx                // has src ptr reached end?
        jne     Float32_To_Int24_Clip_loop

        ffree   st(0)
        fincstp

        fwait
        fnclex
        fldcw   savedFpuControlWord
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int24_DitherClip(
    uk destinationBuffer, i32 destinationStride,
    uk sourceBuffer, i32 sourceStride,
    u32 count, PaUtilTriangularDitherGenerator *ditherGenerator )
{
/*
    float *src = (float*)sourceBuffer;
    u8 *dest = (u8*)destinationBuffer;
    i64 temp;

    while( count-- )
    {
        // convert to 32 bit and drop the low 8 bits

        // FIXME: the dither amplitude here appears to be too small by 8 bits
        double dither  = PaUtil_GenerateFloatTriangularDither( ditherGenerator );
        // use smaller scaler to prevent overflow when we add the dither
        double dithered = ((double)*src * (2147483646.0)) + dither;
        PA_CLIP_( dithered, -2147483648., 2147483647.  );

        temp = (i64) dithered;

        dest[0] = (u8)(temp >> 8);
        dest[1] = (u8)(temp >> 16);
        dest[2] = (u8)(temp >> 24);

        src += sourceStride;
        dest += destinationStride * 3;
    }
*/

    short savedFpuControlWord;

    // spill storage:
    i64 sourceByteStride;
    i64 highpassedDither;

    // dither state:
    u64 ditherPrevious = ditherGenerator->previous;
    u64 ditherRandSeed1 = ditherGenerator->randSeed1;
    u64 ditherRandSeed2 = ditherGenerator->randSeed2;

    i64 tempInt32;

    __asm{
        // esi -> source ptr
        // eax -> source byte stride
        // edi -> destination ptr
        // ebx -> destination byte stride
        // ecx -> source end ptr
        // edx -> temp

        mov     esi, sourceBuffer

        mov     edx, 4                  // sizeof float32
        mov     eax, sourceStride
        imul    eax, edx

        mov     ecx, count
        imul    ecx, eax
        add     ecx, esi

        mov     edi, destinationBuffer

        mov     edx, 3                  // sizeof int24
        mov     ebx, destinationStride
        imul    ebx, edx

        fwait
        fstcw   savedFpuControlWord
        fldcw   fpuControlWord_

        fld     ditheredInt24Scaler_    // stack:  i32 scaler

    Float32_To_Int24_DitherClip_loop:

        mov     edx, dword ptr [esi]    // load floating point value into integer register

        and     edx, 0x7FFFFFFF         // mask off sign
        cmp     edx, 0x3F800000         // greater than 1.0 or less than -1.0

        jg      Float32_To_Int24_DitherClip_clamp

        // load unscaled value into st(0)
        fld     dword ptr [esi]         // stack:  value, i32 scaler
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        fmul    st(0), st(1)            // st(0) *= st(1), stack:  value*(i32 scaler), i32 scaler

    /*
        // call PaUtil_GenerateFloatTriangularDither with C calling convention
        mov     sourceByteStride, eax   // save eax
        mov     sourceEnd, ecx          // save ecx
        push    ditherGenerator         // pass ditherGenerator parameter on stack
        call    PaUtil_GenerateFloatTriangularDither  // stack:  dither, value*(i32 scaler), i32 scaler
        pop     edx                     // clear parameter off stack
        mov     ecx, sourceEnd          // restore ecx
        mov     eax, sourceByteStride   // restore eax
    */

    // generate dither
        mov     sourceByteStride, eax   // save eax
        mov     edx, 196314165
        mov     eax, ditherRandSeed1
        mul     edx                     // eax:edx = eax * 196314165
        //add     eax, 907633515
        lea     eax, [eax+907633515]
        mov     ditherRandSeed1, eax
        mov     edx, 196314165
        mov     eax, ditherRandSeed2
        mul     edx                     // eax:edx = eax * 196314165
        //add     eax, 907633515
        lea     eax, [eax+907633515]
        mov     edx, ditherRandSeed1
        shr     edx, PA_DITHER_SHIFT_
        mov     ditherRandSeed2, eax
        shr     eax, PA_DITHER_SHIFT_
        //add     eax, edx              // eax -> current
        lea     eax, [eax+edx]
        mov     edx, ditherPrevious
        neg     edx
        lea     edx, [eax+edx]          // highpass = current - previous
        mov     highpassedDither, edx
        mov     ditherPrevious, eax     // previous = current
        mov     eax, sourceByteStride   // restore eax
        fild    highpassedDither
        fmul    const_float_dither_scale_
    // end generate dither, dither signal in st(0)

        faddp   st(1), st(0)            // stack: dither * value*(i32 scaler), i32 scaler
        fistp   tempInt32               // pop st(0) into tempInt32, stack:  i32 scaler
        mov     edx, tempInt32
        jmp     Float32_To_Int24_DitherClip_store

    Float32_To_Int24_DitherClip_clamp:
        mov     edx, dword ptr [esi]    // load floating point value into integer register
        shr     edx, 31                 // move sign bit into bit 0
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        add     edx, 0x7FFFFF           // convert to maximum range integers

    Float32_To_Int24_DitherClip_store:

        mov     byte ptr [edi], DL
        shr     edx, 8
        //mov     byte ptr [edi+1], DL
        //mov     byte ptr [edi+2], DH
        mov     word ptr [edi+1], DX

        //add     edi, ebx              // increment destination ptr
        lea     edi, [edi+ebx]

        cmp     esi, ecx                // has src ptr reached end?
        jne     Float32_To_Int24_DitherClip_loop

        ffree   st(0)
        fincstp

        fwait
        fnclex
        fldcw   savedFpuControlWord
    }

    ditherGenerator->previous = ditherPrevious;
    ditherGenerator->randSeed1 = ditherRandSeed1;
    ditherGenerator->randSeed2 = ditherRandSeed2;
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int16(
    uk destinationBuffer, i32 destinationStride,
    uk sourceBuffer, i32 sourceStride,
    u32 count, PaUtilTriangularDitherGenerator *ditherGenerator )
{
/*
    float *src = (float*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    (void)ditherGenerator; // unused parameter

    while( count-- )
    {

        short samp = (short) (*src * (32767.0f));
        *dest = samp;

        src += sourceStride;
        dest += destinationStride;
    }
*/

    short savedFpuControlWord;

    (void) ditherGenerator; /* unused parameter */

    __asm{
        // esi -> source ptr
        // eax -> source byte stride
        // edi -> destination ptr
        // ebx -> destination byte stride
        // ecx -> source end ptr
        // edx -> temp

        mov     esi, sourceBuffer

        mov     edx, 4                  // sizeof float32
        mov     eax, sourceStride
        imul    eax, edx                // source byte stride

        mov     ecx, count
        imul    ecx, eax
        add     ecx, esi                // source end ptr = count * source byte stride + source ptr

        mov     edi, destinationBuffer

        mov     edx, 2                  // sizeof i16
        mov     ebx, destinationStride
        imul    ebx, edx                // destination byte stride

        fwait
        fstcw   savedFpuControlWord
        fldcw   fpuControlWord_

        fld     int16Scaler_            // stack:  (i32)0x7FFF

    Float32_To_Int16_loop:

        // load unscaled value into st(0)
        fld     dword ptr [esi]         // stack:  value, (i32)0x7FFF
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        fmul    st(0), st(1)            // st(0) *= st(1), stack:  value*0x7FFF, (i32)0x7FFF
        fistp   word ptr [edi]          // store scaled i32 into dest, stack:  (i32)0x7FFF

        add     edi, ebx                // increment destination ptr
        //lea     edi, [edi+ebx]

        cmp     esi, ecx                // has src ptr reached end?
        jne     Float32_To_Int16_loop

        ffree   st(0)
        fincstp

        fwait
        fnclex
        fldcw   savedFpuControlWord
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int16_Clip(
    uk destinationBuffer, i32 destinationStride,
    uk sourceBuffer, i32 sourceStride,
    u32 count, PaUtilTriangularDitherGenerator *ditherGenerator )
{
/*
    float *src = (float*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    (void)ditherGenerator; // unused parameter

    while( count-- )
    {
        long samp = (i64) (*src * (32767.0f));
        PA_CLIP_( samp, -0x8000, 0x7FFF );
        *dest = (signed short) samp;

        src += sourceStride;
        dest += destinationStride;
    }
*/

    short savedFpuControlWord;

    (void) ditherGenerator; /* unused parameter */

    __asm{
        // esi -> source ptr
        // eax -> source byte stride
        // edi -> destination ptr
        // ebx -> destination byte stride
        // ecx -> source end ptr
        // edx -> temp

        mov     esi, sourceBuffer

        mov     edx, 4                  // sizeof float32
        mov     eax, sourceStride
        imul    eax, edx                // source byte stride

        mov     ecx, count
        imul    ecx, eax
        add     ecx, esi                // source end ptr = count * source byte stride + source ptr

        mov     edi, destinationBuffer

        mov     edx, 2                  // sizeof i16
        mov     ebx, destinationStride
        imul    ebx, edx                // destination byte stride

        fwait
        fstcw   savedFpuControlWord
        fldcw   fpuControlWord_

        fld     int16Scaler_            // stack:  (i32)0x7FFF

    Float32_To_Int16_Clip_loop:

        mov     edx, dword ptr [esi]    // load floating point value into integer register

        and     edx, 0x7FFFFFFF         // mask off sign
        cmp     edx, 0x3F800000         // greater than 1.0 or less than -1.0

        jg      Float32_To_Int16_Clip_clamp

        // load unscaled value into st(0)
        fld     dword ptr [esi]         // stack:  value, (i32)0x7FFF
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        fmul    st(0), st(1)            // st(0) *= st(1), stack:  value*0x7FFF, (i32)0x7FFF
        fistp   word ptr [edi]          // store scaled i32 into dest, stack:  (i32)0x7FFF
        jmp     Float32_To_Int16_Clip_stored

    Float32_To_Int16_Clip_clamp:
        mov     edx, dword ptr [esi]    // load floating point value into integer register
        shr     edx, 31                 // move sign bit into bit 0
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        add     dx, 0x7FFF              // convert to maximum range integers
        mov     word ptr [edi], dx      // store clamped into into dest

    Float32_To_Int16_Clip_stored:

        add     edi, ebx                // increment destination ptr
        //lea     edi, [edi+ebx]

        cmp     esi, ecx                // has src ptr reached end?
        jne     Float32_To_Int16_Clip_loop

        ffree   st(0)
        fincstp

        fwait
        fnclex
        fldcw   savedFpuControlWord
    }
}

/* -------------------------------------------------------------------------- */

static void Float32_To_Int16_DitherClip(
    uk destinationBuffer, i32 destinationStride,
    uk sourceBuffer, i32 sourceStride,
    u32 count, PaUtilTriangularDitherGenerator *ditherGenerator )
{
/*
    float *src = (float*)sourceBuffer;
    signed short *dest =  (signed short*)destinationBuffer;
    (void)ditherGenerator; // unused parameter

    while( count-- )
    {

        float dither  = PaUtil_GenerateFloatTriangularDither( ditherGenerator );
        // use smaller scaler to prevent overflow when we add the dither
        float dithered = (*src * (32766.0f)) + dither;
        i64 samp = (i64) dithered;
        PA_CLIP_( samp, -0x8000, 0x7FFF );
        *dest = (signed short) samp;

        src += sourceStride;
        dest += destinationStride;
    }
*/

    short savedFpuControlWord;

    // spill storage:
    i64 sourceByteStride;
    i64 highpassedDither;

    // dither state:
    u64 ditherPrevious = ditherGenerator->previous;
    u64 ditherRandSeed1 = ditherGenerator->randSeed1;
    u64 ditherRandSeed2 = ditherGenerator->randSeed2;

    __asm{
        // esi -> source ptr
        // eax -> source byte stride
        // edi -> destination ptr
        // ebx -> destination byte stride
        // ecx -> source end ptr
        // edx -> temp

        mov     esi, sourceBuffer

        mov     edx, 4                  // sizeof float32
        mov     eax, sourceStride
        imul    eax, edx                // source byte stride

        mov     ecx, count
        imul    ecx, eax
        add     ecx, esi                // source end ptr = count * source byte stride + source ptr

        mov     edi, destinationBuffer

        mov     edx, 2                  // sizeof i16
        mov     ebx, destinationStride
        imul    ebx, edx                // destination byte stride

        fwait
        fstcw   savedFpuControlWord
        fldcw   fpuControlWord_

        fld     ditheredInt16Scaler_    // stack:  i32 scaler

    Float32_To_Int16_DitherClip_loop:

        mov     edx, dword ptr [esi]    // load floating point value into integer register

        and     edx, 0x7FFFFFFF         // mask off sign
        cmp     edx, 0x3F800000         // greater than 1.0 or less than -1.0

        jg      Float32_To_Int16_DitherClip_clamp

        // load unscaled value into st(0)
        fld     dword ptr [esi]         // stack:  value, i32 scaler
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        fmul    st(0), st(1)            // st(0) *= st(1), stack:  value*(i32 scaler), i32 scaler

        /*
        // call PaUtil_GenerateFloatTriangularDither with C calling convention
        mov     sourceByteStride, eax   // save eax
        mov     sourceEnd, ecx          // save ecx
        push    ditherGenerator         // pass ditherGenerator parameter on stack
        call    PaUtil_GenerateFloatTriangularDither  // stack:  dither, value*(i32 scaler), i32 scaler
        pop     edx                     // clear parameter off stack
        mov     ecx, sourceEnd          // restore ecx
        mov     eax, sourceByteStride   // restore eax
        */

    // generate dither
        mov     sourceByteStride, eax   // save eax
        mov     edx, 196314165
        mov     eax, ditherRandSeed1
        mul     edx                     // eax:edx = eax * 196314165
        //add     eax, 907633515
        lea     eax, [eax+907633515]
        mov     ditherRandSeed1, eax
        mov     edx, 196314165
        mov     eax, ditherRandSeed2
        mul     edx                     // eax:edx = eax * 196314165
        //add     eax, 907633515
        lea     eax, [eax+907633515]
        mov     edx, ditherRandSeed1
        shr     edx, PA_DITHER_SHIFT_
        mov     ditherRandSeed2, eax
        shr     eax, PA_DITHER_SHIFT_
        //add     eax, edx              // eax -> current
        lea     eax, [eax+edx]          // current = randSeed1>>x + randSeed2>>x
        mov     edx, ditherPrevious
        neg     edx
        lea     edx, [eax+edx]          // highpass = current - previous
        mov     highpassedDither, edx
        mov     ditherPrevious, eax     // previous = current
        mov     eax, sourceByteStride   // restore eax
        fild    highpassedDither
        fmul    const_float_dither_scale_
    // end generate dither, dither signal in st(0)

        faddp   st(1), st(0)            // stack: dither * value*(i32 scaler), i32 scaler
        fistp   word ptr [edi]          // store scaled i32 into dest, stack:  i32 scaler
        jmp     Float32_To_Int16_DitherClip_stored

    Float32_To_Int16_DitherClip_clamp:
        mov     edx, dword ptr [esi]    // load floating point value into integer register
        shr     edx, 31                 // move sign bit into bit 0
        add     esi, eax                // increment source ptr
        //lea     esi, [esi+eax]
        add     dx, 0x7FFF              // convert to maximum range integers
        mov     word ptr [edi], dx      // store clamped into into dest

    Float32_To_Int16_DitherClip_stored:

        add     edi, ebx                // increment destination ptr
        //lea     edi, [edi+ebx]

        cmp     esi, ecx                // has src ptr reached end?
        jne     Float32_To_Int16_DitherClip_loop

        ffree   st(0)
        fincstp

        fwait
        fnclex
        fldcw   savedFpuControlWord
    }

    ditherGenerator->previous = ditherPrevious;
    ditherGenerator->randSeed1 = ditherRandSeed1;
    ditherGenerator->randSeed2 = ditherRandSeed2;
}

/* -------------------------------------------------------------------------- */

void PaUtil_InitializeX86PlainConverters( void )
{
    paConverters.Float32_To_Int32 = Float32_To_Int32;
    paConverters.Float32_To_Int32_Clip = Float32_To_Int32_Clip;
    paConverters.Float32_To_Int32_DitherClip = Float32_To_Int32_DitherClip;

    paConverters.Float32_To_Int24 = Float32_To_Int24;
    paConverters.Float32_To_Int24_Clip = Float32_To_Int24_Clip;
    paConverters.Float32_To_Int24_DitherClip = Float32_To_Int24_DitherClip;

    paConverters.Float32_To_Int16 = Float32_To_Int16;
    paConverters.Float32_To_Int16_Clip = Float32_To_Int16_Clip;
    paConverters.Float32_To_Int16_DitherClip = Float32_To_Int16_DitherClip;
}

#endif

/* -------------------------------------------------------------------------- */
