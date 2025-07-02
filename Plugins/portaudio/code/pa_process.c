/*
 * $Id$
 * Portable Audio I/O Library
 * streamCallback <-> host buffer processing adapter
 *
 * Based on the Open Source API proposed by Ross Bencina
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
 @ingroup common_src

 @brief Buffer Processor implementation.
*/


#include <assert.h>
#include <string.h> /* memset() */

#include "pa_process.h"
#include "pa_util.h"


#define PA_FRAMES_PER_TEMP_BUFFER_WHEN_HOST_BUFFER_SIZE_IS_UNKNOWN_    1024

#define PA_MIN_( a, b ) ( ((a)<(b)) ? (a) : (b) )


/* greatest common divisor - PGCD in French */
static u64 GCD( u64 a, u64 b )
{
    return (b==0) ? a : GCD( b, a%b);
}

/* least common multiple - PPCM in French */
static u64 LCM( u64 a, u64 b )
{
    return (a*b) / GCD(a,b);
}

#define PA_MAX_( a, b ) (((a) > (b)) ? (a) : (b))

static u64 CalculateFrameShift( u64 M, u64 N )
{
    u64 result = 0;
    u64 i;
    u64 lcm;

    assert( M > 0 );
    assert( N > 0 );

    lcm = LCM( M, N );
    for( i = M; i < lcm; i += M )
        result = PA_MAX_( result, i % N );

    return result;
}


PaError PaUtil_InitializeBufferProcessor( PaUtilBufferProcessor* bp,
        i32 inputChannelCount, PaSampleFormat userInputSampleFormat,
        PaSampleFormat hostInputSampleFormat,
        i32 outputChannelCount, PaSampleFormat userOutputSampleFormat,
        PaSampleFormat hostOutputSampleFormat,
        double sampleRate,
        PaStreamFlags streamFlags,
        u64 framesPerUserBuffer,
        u64 framesPerHostBuffer,
        PaUtilHostBufferSizeMode hostBufferSizeMode,
        PaStreamCallback *streamCallback, uk userData )
{
    PaError result = paNoError;
    PaError bytesPerSample;
    u64 tempInputBufferSize, tempOutputBufferSize;
    PaStreamFlags tempInputStreamFlags;

    if( streamFlags & paNeverDropInput )
    {
        /* paNeverDropInput is only valid for full-duplex callback streams, with an unspecified number of frames per buffer. */
        if( !streamCallback || !(inputChannelCount > 0 && outputChannelCount > 0) ||
                framesPerUserBuffer != paFramesPerBufferUnspecified )
            return paInvalidFlag;
    }

    /* initialize buffer ptrs to zero so they can be freed if necessary in error */
    bp->tempInputBuffer = 0;
    bp->tempInputBufferPtrs = 0;
    bp->tempOutputBuffer = 0;
    bp->tempOutputBufferPtrs = 0;

    bp->framesPerUserBuffer = framesPerUserBuffer;
    bp->framesPerHostBuffer = framesPerHostBuffer;

    bp->inputChannelCount = inputChannelCount;
    bp->outputChannelCount = outputChannelCount;

    bp->hostBufferSizeMode = hostBufferSizeMode;

    bp->hostInputChannels[0] = bp->hostInputChannels[1] = 0;
    bp->hostOutputChannels[0] = bp->hostOutputChannels[1] = 0;

    if( framesPerUserBuffer == 0 ) /* streamCallback will accept any buffer size */
    {
        bp->useNonAdaptingProcess = 1;
        bp->initialFramesInTempInputBuffer = 0;
        bp->initialFramesInTempOutputBuffer = 0;

        if( hostBufferSizeMode == paUtilFixedHostBufferSize
                || hostBufferSizeMode == paUtilBoundedHostBufferSize )
        {
            bp->framesPerTempBuffer = framesPerHostBuffer;
        }
        else /* unknown host buffer size */
        {
            bp->framesPerTempBuffer = PA_FRAMES_PER_TEMP_BUFFER_WHEN_HOST_BUFFER_SIZE_IS_UNKNOWN_;
        }
    }
    else
    {
        bp->framesPerTempBuffer = framesPerUserBuffer;

        if( hostBufferSizeMode == paUtilFixedHostBufferSize
                && framesPerHostBuffer % framesPerUserBuffer == 0 )
        {
            bp->useNonAdaptingProcess = 1;
            bp->initialFramesInTempInputBuffer = 0;
            bp->initialFramesInTempOutputBuffer = 0;
        }
        else
        {
            bp->useNonAdaptingProcess = 0;

            if( inputChannelCount > 0 && outputChannelCount > 0 )
            {
                /* full duplex */
                if( hostBufferSizeMode == paUtilFixedHostBufferSize )
                {
                    u64 frameShift =
                        CalculateFrameShift( framesPerHostBuffer, framesPerUserBuffer );

                    if( framesPerUserBuffer > framesPerHostBuffer )
                    {
                        bp->initialFramesInTempInputBuffer = frameShift;
                        bp->initialFramesInTempOutputBuffer = 0;
                    }
                    else
                    {
                        bp->initialFramesInTempInputBuffer = 0;
                        bp->initialFramesInTempOutputBuffer = frameShift;
                    }
                }
                else /* variable host buffer size, add framesPerUserBuffer latency */
                {
                    bp->initialFramesInTempInputBuffer = 0;
                    bp->initialFramesInTempOutputBuffer = framesPerUserBuffer;
                }
            }
            else
            {
                /* half duplex */
                bp->initialFramesInTempInputBuffer = 0;
                bp->initialFramesInTempOutputBuffer = 0;
            }
        }
    }


    bp->framesInTempInputBuffer = bp->initialFramesInTempInputBuffer;
    bp->framesInTempOutputBuffer = bp->initialFramesInTempOutputBuffer;


    if( inputChannelCount > 0 )
    {
        bytesPerSample = Pa_GetSampleSize( hostInputSampleFormat );
        if( bytesPerSample > 0 )
        {
            bp->bytesPerHostInputSample = bytesPerSample;
        }
        else
        {
            result = bytesPerSample;
            goto error;
        }

        bytesPerSample = Pa_GetSampleSize( userInputSampleFormat );
        if( bytesPerSample > 0 )
        {
            bp->bytesPerUserInputSample = bytesPerSample;
        }
        else
        {
            result = bytesPerSample;
            goto error;
        }

        /* Under the assumption that no ADC in existence delivers better than 24bits resolution,
            we disable dithering when host input format is paInt32 and user format is paInt24,
            since the host samples will just be padded with zeros anyway. */

        tempInputStreamFlags = streamFlags;
        if( !(tempInputStreamFlags & paDitherOff) /* dither is on */
                && (hostInputSampleFormat & paInt32) /* host input format is i32 */
                && (userInputSampleFormat & paInt24) /* user requested format is int24 */ ){

            tempInputStreamFlags = tempInputStreamFlags | paDitherOff;
        }

        bp->inputConverter =
            PaUtil_SelectConverter( hostInputSampleFormat, userInputSampleFormat, tempInputStreamFlags );

        bp->inputZeroer = PaUtil_SelectZeroer( userInputSampleFormat );

        bp->userInputIsInterleaved = (userInputSampleFormat & paNonInterleaved)?0:1;

        bp->hostInputIsInterleaved = (hostInputSampleFormat & paNonInterleaved)?0:1;

        bp->userInputSampleFormatIsEqualToHost = ((userInputSampleFormat & ~paNonInterleaved) == (hostInputSampleFormat & ~paNonInterleaved));

        tempInputBufferSize =
            bp->framesPerTempBuffer * bp->bytesPerUserInputSample * inputChannelCount;

        bp->tempInputBuffer = PaUtil_AllocateMemory( tempInputBufferSize );
        if( bp->tempInputBuffer == 0 )
        {
            result = paInsufficientMemory;
            goto error;
        }

        if( bp->framesInTempInputBuffer > 0 )
            memset( bp->tempInputBuffer, 0, tempInputBufferSize );

        if( userInputSampleFormat & paNonInterleaved )
        {
            bp->tempInputBufferPtrs =
                (uk *)PaUtil_AllocateMemory( sizeof(uk )*inputChannelCount );
            if( bp->tempInputBufferPtrs == 0 )
            {
                result = paInsufficientMemory;
                goto error;
            }
        }

        bp->hostInputChannels[0] = (PaUtilChannelDescriptor*)
                PaUtil_AllocateMemory( sizeof(PaUtilChannelDescriptor) * inputChannelCount * 2);
        if( bp->hostInputChannels[0] == 0 )
        {
            result = paInsufficientMemory;
            goto error;
        }

        bp->hostInputChannels[1] = &bp->hostInputChannels[0][inputChannelCount];
    }

    if( outputChannelCount > 0 )
    {
        bytesPerSample = Pa_GetSampleSize( hostOutputSampleFormat );
        if( bytesPerSample > 0 )
        {
            bp->bytesPerHostOutputSample = bytesPerSample;
        }
        else
        {
            result = bytesPerSample;
            goto error;
        }

        bytesPerSample = Pa_GetSampleSize( userOutputSampleFormat );
        if( bytesPerSample > 0 )
        {
            bp->bytesPerUserOutputSample = bytesPerSample;
        }
        else
        {
            result = bytesPerSample;
            goto error;
        }

        bp->outputConverter =
            PaUtil_SelectConverter( userOutputSampleFormat, hostOutputSampleFormat, streamFlags );

        bp->outputZeroer = PaUtil_SelectZeroer( hostOutputSampleFormat );

        bp->userOutputIsInterleaved = (userOutputSampleFormat & paNonInterleaved)?0:1;

        bp->hostOutputIsInterleaved = (hostOutputSampleFormat & paNonInterleaved)?0:1;

        bp->userOutputSampleFormatIsEqualToHost = ((userOutputSampleFormat & ~paNonInterleaved) == (hostOutputSampleFormat & ~paNonInterleaved));

        tempOutputBufferSize =
                bp->framesPerTempBuffer * bp->bytesPerUserOutputSample * outputChannelCount;

        bp->tempOutputBuffer = PaUtil_AllocateMemory( tempOutputBufferSize );
        if( bp->tempOutputBuffer == 0 )
        {
            result = paInsufficientMemory;
            goto error;
        }

        if( bp->framesInTempOutputBuffer > 0 )
            memset( bp->tempOutputBuffer, 0, tempOutputBufferSize );

        if( userOutputSampleFormat & paNonInterleaved )
        {
            bp->tempOutputBufferPtrs =
                (uk *)PaUtil_AllocateMemory( sizeof(uk )*outputChannelCount );
            if( bp->tempOutputBufferPtrs == 0 )
            {
                result = paInsufficientMemory;
                goto error;
            }
        }

        bp->hostOutputChannels[0] = (PaUtilChannelDescriptor*)
                PaUtil_AllocateMemory( sizeof(PaUtilChannelDescriptor)*outputChannelCount * 2 );
        if( bp->hostOutputChannels[0] == 0 )
        {
            result = paInsufficientMemory;
            goto error;
        }

        bp->hostOutputChannels[1] = &bp->hostOutputChannels[0][outputChannelCount];
    }

    PaUtil_InitializeTriangularDitherState( &bp->ditherGenerator );

    bp->samplePeriod = 1. / sampleRate;

    bp->streamCallback = streamCallback;
    bp->userData = userData;

    return result;

error:
    if( bp->tempInputBuffer )
        PaUtil_FreeMemory( bp->tempInputBuffer );

    if( bp->tempInputBufferPtrs )
        PaUtil_FreeMemory( bp->tempInputBufferPtrs );

    if( bp->hostInputChannels[0] )
        PaUtil_FreeMemory( bp->hostInputChannels[0] );

    if( bp->tempOutputBuffer )
        PaUtil_FreeMemory( bp->tempOutputBuffer );

    if( bp->tempOutputBufferPtrs )
        PaUtil_FreeMemory( bp->tempOutputBufferPtrs );

    if( bp->hostOutputChannels[0] )
        PaUtil_FreeMemory( bp->hostOutputChannels[0] );

    return result;
}


void PaUtil_TerminateBufferProcessor( PaUtilBufferProcessor* bp )
{
    if( bp->tempInputBuffer )
        PaUtil_FreeMemory( bp->tempInputBuffer );

    if( bp->tempInputBufferPtrs )
        PaUtil_FreeMemory( bp->tempInputBufferPtrs );

    if( bp->hostInputChannels[0] )
        PaUtil_FreeMemory( bp->hostInputChannels[0] );

    if( bp->tempOutputBuffer )
        PaUtil_FreeMemory( bp->tempOutputBuffer );

    if( bp->tempOutputBufferPtrs )
        PaUtil_FreeMemory( bp->tempOutputBufferPtrs );

    if( bp->hostOutputChannels[0] )
        PaUtil_FreeMemory( bp->hostOutputChannels[0] );
}


void PaUtil_ResetBufferProcessor( PaUtilBufferProcessor* bp )
{
    u64 tempInputBufferSize, tempOutputBufferSize;

    bp->framesInTempInputBuffer = bp->initialFramesInTempInputBuffer;
    bp->framesInTempOutputBuffer = bp->initialFramesInTempOutputBuffer;

    if( bp->framesInTempInputBuffer > 0 )
    {
        tempInputBufferSize =
            bp->framesPerTempBuffer * bp->bytesPerUserInputSample * bp->inputChannelCount;
        memset( bp->tempInputBuffer, 0, tempInputBufferSize );
    }

    if( bp->framesInTempOutputBuffer > 0 )
    {
        tempOutputBufferSize =
            bp->framesPerTempBuffer * bp->bytesPerUserOutputSample * bp->outputChannelCount;
        memset( bp->tempOutputBuffer, 0, tempOutputBufferSize );
    }
}


u64 PaUtil_GetBufferProcessorInputLatencyFrames( PaUtilBufferProcessor* bp )
{
    return bp->initialFramesInTempInputBuffer;
}


u64 PaUtil_GetBufferProcessorOutputLatencyFrames( PaUtilBufferProcessor* bp )
{
    return bp->initialFramesInTempOutputBuffer;
}


void PaUtil_SetInputFrameCount( PaUtilBufferProcessor* bp,
        u64 frameCount )
{
    if( frameCount == 0 )
        bp->hostInputFrameCount[0] = bp->framesPerHostBuffer;
    else
        bp->hostInputFrameCount[0] = frameCount;
}


void PaUtil_SetNoInput( PaUtilBufferProcessor* bp )
{
    assert( bp->inputChannelCount > 0 );

    bp->hostInputChannels[0][0].data = 0;
}


void PaUtil_SetInputChannel( PaUtilBufferProcessor* bp,
        u32 channel, uk data, u32 stride )
{
    assert( channel < bp->inputChannelCount );

    bp->hostInputChannels[0][channel].data = data;
    bp->hostInputChannels[0][channel].stride = stride;
}


void PaUtil_SetInterleavedInputChannels( PaUtilBufferProcessor* bp,
        u32 firstChannel, uk data, u32 channelCount )
{
    u32 i;
    u32 channel = firstChannel;
    u8 *p = (u8*)data;

    if( channelCount == 0 )
        channelCount = bp->inputChannelCount;

    assert( firstChannel < bp->inputChannelCount );
    assert( firstChannel + channelCount <= bp->inputChannelCount );
    assert( bp->hostInputIsInterleaved );

    for( i=0; i< channelCount; ++i )
    {
        bp->hostInputChannels[0][channel+i].data = p;
        p += bp->bytesPerHostInputSample;
        bp->hostInputChannels[0][channel+i].stride = channelCount;
    }
}


void PaUtil_SetNonInterleavedInputChannel( PaUtilBufferProcessor* bp,
        u32 channel, uk data )
{
    assert( channel < bp->inputChannelCount );
    assert( !bp->hostInputIsInterleaved );

    bp->hostInputChannels[0][channel].data = data;
    bp->hostInputChannels[0][channel].stride = 1;
}


void PaUtil_Set2ndInputFrameCount( PaUtilBufferProcessor* bp,
        u64 frameCount )
{
    bp->hostInputFrameCount[1] = frameCount;
}


void PaUtil_Set2ndInputChannel( PaUtilBufferProcessor* bp,
        u32 channel, uk data, u32 stride )
{
    assert( channel < bp->inputChannelCount );

    bp->hostInputChannels[1][channel].data = data;
    bp->hostInputChannels[1][channel].stride = stride;
}


void PaUtil_Set2ndInterleavedInputChannels( PaUtilBufferProcessor* bp,
        u32 firstChannel, uk data, u32 channelCount )
{
    u32 i;
    u32 channel = firstChannel;
    u8 *p = (u8*)data;

    if( channelCount == 0 )
        channelCount = bp->inputChannelCount;

    assert( firstChannel < bp->inputChannelCount );
    assert( firstChannel + channelCount <= bp->inputChannelCount );
    assert( bp->hostInputIsInterleaved );

    for( i=0; i< channelCount; ++i )
    {
        bp->hostInputChannels[1][channel+i].data = p;
        p += bp->bytesPerHostInputSample;
        bp->hostInputChannels[1][channel+i].stride = channelCount;
    }
}


void PaUtil_Set2ndNonInterleavedInputChannel( PaUtilBufferProcessor* bp,
        u32 channel, uk data )
{
    assert( channel < bp->inputChannelCount );
    assert( !bp->hostInputIsInterleaved );

    bp->hostInputChannels[1][channel].data = data;
    bp->hostInputChannels[1][channel].stride = 1;
}


void PaUtil_SetOutputFrameCount( PaUtilBufferProcessor* bp,
        u64 frameCount )
{
    if( frameCount == 0 )
        bp->hostOutputFrameCount[0] = bp->framesPerHostBuffer;
    else
        bp->hostOutputFrameCount[0] = frameCount;
}


void PaUtil_SetNoOutput( PaUtilBufferProcessor* bp )
{
    assert( bp->outputChannelCount > 0 );

    bp->hostOutputChannels[0][0].data = 0;

    /* note that only NonAdaptingProcess is able to deal with no output at this stage. not implemented for AdaptingProcess */
}


void PaUtil_SetOutputChannel( PaUtilBufferProcessor* bp,
        u32 channel, uk data, u32 stride )
{
    assert( channel < bp->outputChannelCount );
    assert( data != NULL );

    bp->hostOutputChannels[0][channel].data = data;
    bp->hostOutputChannels[0][channel].stride = stride;
}


void PaUtil_SetInterleavedOutputChannels( PaUtilBufferProcessor* bp,
        u32 firstChannel, uk data, u32 channelCount )
{
    u32 i;
    u32 channel = firstChannel;
    u8 *p = (u8*)data;

    if( channelCount == 0 )
        channelCount = bp->outputChannelCount;

    assert( firstChannel < bp->outputChannelCount );
    assert( firstChannel + channelCount <= bp->outputChannelCount );
    assert( bp->hostOutputIsInterleaved );

    for( i=0; i< channelCount; ++i )
    {
        PaUtil_SetOutputChannel( bp, channel + i, p, channelCount );
        p += bp->bytesPerHostOutputSample;
    }
}


void PaUtil_SetNonInterleavedOutputChannel( PaUtilBufferProcessor* bp,
        u32 channel, uk data )
{
    assert( channel < bp->outputChannelCount );
    assert( !bp->hostOutputIsInterleaved );

    PaUtil_SetOutputChannel( bp, channel, data, 1 );
}


void PaUtil_Set2ndOutputFrameCount( PaUtilBufferProcessor* bp,
        u64 frameCount )
{
    bp->hostOutputFrameCount[1] = frameCount;
}


void PaUtil_Set2ndOutputChannel( PaUtilBufferProcessor* bp,
        u32 channel, uk data, u32 stride )
{
    assert( channel < bp->outputChannelCount );
    assert( data != NULL );

    bp->hostOutputChannels[1][channel].data = data;
    bp->hostOutputChannels[1][channel].stride = stride;
}


void PaUtil_Set2ndInterleavedOutputChannels( PaUtilBufferProcessor* bp,
        u32 firstChannel, uk data, u32 channelCount )
{
    u32 i;
    u32 channel = firstChannel;
    u8 *p = (u8*)data;

    if( channelCount == 0 )
        channelCount = bp->outputChannelCount;

    assert( firstChannel < bp->outputChannelCount );
    assert( firstChannel + channelCount <= bp->outputChannelCount );
    assert( bp->hostOutputIsInterleaved );

    for( i=0; i< channelCount; ++i )
    {
        PaUtil_Set2ndOutputChannel( bp, channel + i, p, channelCount );
        p += bp->bytesPerHostOutputSample;
    }
}


void PaUtil_Set2ndNonInterleavedOutputChannel( PaUtilBufferProcessor* bp,
        u32 channel, uk data )
{
    assert( channel < bp->outputChannelCount );
    assert( !bp->hostOutputIsInterleaved );

    PaUtil_Set2ndOutputChannel( bp, channel, data, 1 );
}


void PaUtil_BeginBufferProcessing( PaUtilBufferProcessor* bp,
        PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags callbackStatusFlags )
{
    bp->timeInfo = timeInfo;

    /* the first streamCallback will be called to process samples which are
        currently in the input buffer before the ones starting at the timeInfo time */

    bp->timeInfo->inputBufferAdcTime -= bp->framesInTempInputBuffer * bp->samplePeriod;

    /* We just pass through timeInfo->currentTime provided by the caller. This is
        not strictly conformant to the word of the spec, since the buffer processor
        might call the callback multiple times, and we never refresh currentTime. */

    /* the first streamCallback will be called to generate samples which will be
        outputted after the frames currently in the output buffer have been
        outputted. */
    bp->timeInfo->outputBufferDacTime += bp->framesInTempOutputBuffer * bp->samplePeriod;

    bp->callbackStatusFlags = callbackStatusFlags;

    bp->hostInputFrameCount[1] = 0;
    bp->hostOutputFrameCount[1] = 0;
}


/*
    NonAdaptingProcess() is a simple buffer copying adaptor that can handle
    both full and half duplex copies. It processes framesToProcess frames,
    broken into blocks bp->framesPerTempBuffer long.
    This routine can be used when the streamCallback doesn't care what length
    the buffers are, or when framesToProcess is an integer multiple of
    bp->framesPerTempBuffer, in which case streamCallback will always be called
    with bp->framesPerTempBuffer samples.
*/
static u64 NonAdaptingProcess( PaUtilBufferProcessor *bp,
        i32 *streamCallbackResult,
        PaUtilChannelDescriptor *hostInputChannels,
        PaUtilChannelDescriptor *hostOutputChannels,
        u64 framesToProcess )
{
    uk userInput, *userOutput;
    u8 *srcBytePtr, *destBytePtr;
    u32 srcSampleStrideSamples; /* stride from one sample to the next within a channel, in samples */
    u32 srcChannelStrideBytes; /* stride from one channel to the next, in bytes */
    u32 destSampleStrideSamples; /* stride from one sample to the next within a channel, in samples */
    u32 destChannelStrideBytes; /* stride from one channel to the next, in bytes */
    u32 i;
    u64 frameCount;
    u64 framesToGo = framesToProcess;
    u64 framesProcessed = 0;
    i32 skipOutputConvert = 0;
    i32 skipInputConvert = 0;


    if( *streamCallbackResult == paContinue )
    {
        do
        {
            frameCount = PA_MIN_( bp->framesPerTempBuffer, framesToGo );

            /* configure user input buffer and convert input data (host -> user) */
            if( bp->inputChannelCount == 0 )
            {
                /* no input */
                userInput = 0;
            }
            else /* there are input channels */
            {

                destBytePtr = (u8*)bp->tempInputBuffer;

                if( bp->userInputIsInterleaved )
                {
                    destSampleStrideSamples = bp->inputChannelCount;
                    destChannelStrideBytes = bp->bytesPerUserInputSample;

                    /* process host buffer directly, or use temp buffer if formats differ or host buffer non-interleaved,
                     * or if num channels differs between the host (set in stride) and the user (eg with some Alsa hw:) */
                    if( bp->userInputSampleFormatIsEqualToHost && bp->hostInputIsInterleaved
                        && bp->hostInputChannels[0][0].data && bp->inputChannelCount == hostInputChannels[0].stride )
                    {
                        userInput = hostInputChannels[0].data;
                        destBytePtr = (u8*)hostInputChannels[0].data;
                        skipInputConvert = 1;
                    }
                    else
                    {
                        userInput = bp->tempInputBuffer;
                    }
                }
                else /* user input is not interleaved */
                {
                    destSampleStrideSamples = 1;
                    destChannelStrideBytes = frameCount * bp->bytesPerUserInputSample;

                    /* setup non-interleaved ptrs */
                    if( bp->userInputSampleFormatIsEqualToHost && !bp->hostInputIsInterleaved && bp->hostInputChannels[0][0].data )
                    {
                        for( i=0; i<bp->inputChannelCount; ++i )
                        {
                            bp->tempInputBufferPtrs[i] = hostInputChannels[i].data;
                        }
                        skipInputConvert = 1;
                    }
                    else
                    {
                        for( i=0; i<bp->inputChannelCount; ++i )
                        {
                            bp->tempInputBufferPtrs[i] = ((u8*)bp->tempInputBuffer) +
                                i * bp->bytesPerUserInputSample * frameCount;
                        }
                    }

                    userInput = bp->tempInputBufferPtrs;
                }

                if( !bp->hostInputChannels[0][0].data )
                {
                    /* no input was supplied (see PaUtil_SetNoInput), so
                        zero the input buffer */

                    for( i=0; i<bp->inputChannelCount; ++i )
                    {
                        bp->inputZeroer( destBytePtr, destSampleStrideSamples, frameCount );
                        destBytePtr += destChannelStrideBytes;  /* skip to next destination channel */
                    }
                }
                else
                {
                    if( skipInputConvert )
                    {
                        for( i=0; i<bp->inputChannelCount; ++i )
                        {
                            /* advance src ptr for next iteration */
                            hostInputChannels[i].data = ((u8*)hostInputChannels[i].data) +
                                    frameCount * hostInputChannels[i].stride * bp->bytesPerHostInputSample;
                        }
                    }
                    else
                    {
                        for( i=0; i<bp->inputChannelCount; ++i )
                        {
                            bp->inputConverter( destBytePtr, destSampleStrideSamples,
                                                    hostInputChannels[i].data,
                                                    hostInputChannels[i].stride,
                                                    frameCount, &bp->ditherGenerator );

                            destBytePtr += destChannelStrideBytes;  /* skip to next destination channel */

                            /* advance src ptr for next iteration */
                            hostInputChannels[i].data = ((u8*)hostInputChannels[i].data) +
                                    frameCount * hostInputChannels[i].stride * bp->bytesPerHostInputSample;
                        }
                    }
                }
            }

            /* configure user output buffer */
            if( bp->outputChannelCount == 0 )
            {
                /* no output */
                userOutput = 0;
            }
            else /* there are output channels */
            {
                if( bp->userOutputIsInterleaved )
                {
                    /* process host buffer directly, or use temp buffer if formats differ or host buffer non-interleaved,
                     * or if num channels differs between the host (set in stride) and the user (eg with some Alsa hw:) */
                    if( bp->userOutputSampleFormatIsEqualToHost && bp->hostOutputIsInterleaved
                            && bp->outputChannelCount == hostOutputChannels[0].stride )
                    {
                        userOutput = hostOutputChannels[0].data;
                        skipOutputConvert = 1;
                    }
                    else
                    {
                        userOutput = bp->tempOutputBuffer;
                    }
                }
                else /* user output is not interleaved */
                {
                    if( bp->userOutputSampleFormatIsEqualToHost && !bp->hostOutputIsInterleaved )
                    {
                        for( i=0; i<bp->outputChannelCount; ++i )
                        {
                            bp->tempOutputBufferPtrs[i] = hostOutputChannels[i].data;
                        }
                        skipOutputConvert = 1;
                    }
                    else
                    {
                        for( i=0; i<bp->outputChannelCount; ++i )
                        {
                            bp->tempOutputBufferPtrs[i] = ((u8*)bp->tempOutputBuffer) +
                                i * bp->bytesPerUserOutputSample * frameCount;
                        }
                    }

                    userOutput = bp->tempOutputBufferPtrs;
                }
            }

            *streamCallbackResult = bp->streamCallback( userInput, userOutput,
                    frameCount, bp->timeInfo, bp->callbackStatusFlags, bp->userData );

            if( *streamCallbackResult == paAbort )
            {
                /* callback returned paAbort, don't advance framesProcessed
                        and framesToGo, they will be handled below */
            }
            else
            {
                bp->timeInfo->inputBufferAdcTime += frameCount * bp->samplePeriod;
                bp->timeInfo->outputBufferDacTime += frameCount * bp->samplePeriod;

                /* convert output data (user -> host) */

                if( bp->outputChannelCount != 0 && bp->hostOutputChannels[0][0].data )
                {
                    if( skipOutputConvert )
                    {
                        for( i=0; i<bp->outputChannelCount; ++i )
                        {
                            /* advance dest ptr for next iteration */
                            hostOutputChannels[i].data = ((u8*)hostOutputChannels[i].data) +
                                    frameCount * hostOutputChannels[i].stride * bp->bytesPerHostOutputSample;
                        }
                    }
                    else
                    {

                        srcBytePtr = (u8*)bp->tempOutputBuffer;

                        if( bp->userOutputIsInterleaved )
                        {
                            srcSampleStrideSamples = bp->outputChannelCount;
                            srcChannelStrideBytes = bp->bytesPerUserOutputSample;
                        }
                        else /* user output is not interleaved */
                        {
                            srcSampleStrideSamples = 1;
                            srcChannelStrideBytes = frameCount * bp->bytesPerUserOutputSample;
                        }

                        for( i=0; i<bp->outputChannelCount; ++i )
                        {
                            bp->outputConverter(    hostOutputChannels[i].data,
                                                    hostOutputChannels[i].stride,
                                                    srcBytePtr, srcSampleStrideSamples,
                                                    frameCount, &bp->ditherGenerator );

                            srcBytePtr += srcChannelStrideBytes;  /* skip to next source channel */

                            /* advance dest ptr for next iteration */
                            hostOutputChannels[i].data = ((u8*)hostOutputChannels[i].data) +
                                        frameCount * hostOutputChannels[i].stride * bp->bytesPerHostOutputSample;
                        }
                    }
                }

                framesProcessed += frameCount;

                framesToGo -= frameCount;
            }
        }
        while( framesToGo > 0  && *streamCallbackResult == paContinue );
    }

    if( framesToGo > 0 )
    {
        /* zero any remaining frames output. There will only be remaining frames
            if the callback has returned paComplete or paAbort */

        frameCount = framesToGo;

        if( bp->outputChannelCount != 0 && bp->hostOutputChannels[0][0].data )
        {
            for( i=0; i<bp->outputChannelCount; ++i )
            {
                bp->outputZeroer(   hostOutputChannels[i].data,
                                    hostOutputChannels[i].stride,
                                    frameCount );

                /* advance dest ptr for next iteration */
                hostOutputChannels[i].data = ((u8*)hostOutputChannels[i].data) +
                        frameCount * hostOutputChannels[i].stride * bp->bytesPerHostOutputSample;
            }
        }

        framesProcessed += frameCount;
    }

    return framesProcessed;
}


/*
    AdaptingInputOnlyProcess() is a half duplex input buffer processor. It
    converts data from the input buffers into the temporary input buffer,
    when the temporary input buffer is full, it calls the streamCallback.
*/
static u64 AdaptingInputOnlyProcess( PaUtilBufferProcessor *bp,
        i32 *streamCallbackResult,
        PaUtilChannelDescriptor *hostInputChannels,
        u64 framesToProcess )
{
    uk userInput, *userOutput;
    u8 *destBytePtr;
    u32 destSampleStrideSamples; /* stride from one sample to the next within a channel, in samples */
    u32 destChannelStrideBytes; /* stride from one channel to the next, in bytes */
    u32 i;
    u64 frameCount;
    u64 framesToGo = framesToProcess;
    u64 framesProcessed = 0;

    userOutput = 0;

    do
    {
        frameCount = ( bp->framesInTempInputBuffer + framesToGo > bp->framesPerUserBuffer )
                ? ( bp->framesPerUserBuffer - bp->framesInTempInputBuffer )
                : framesToGo;

        /* convert frameCount samples into temp buffer */

        if( bp->userInputIsInterleaved )
        {
            destBytePtr = ((u8*)bp->tempInputBuffer) +
                    bp->bytesPerUserInputSample * bp->inputChannelCount *
                    bp->framesInTempInputBuffer;

            destSampleStrideSamples = bp->inputChannelCount;
            destChannelStrideBytes = bp->bytesPerUserInputSample;

            userInput = bp->tempInputBuffer;
        }
        else /* user input is not interleaved */
        {
            destBytePtr = ((u8*)bp->tempInputBuffer) +
                    bp->bytesPerUserInputSample * bp->framesInTempInputBuffer;

            destSampleStrideSamples = 1;
            destChannelStrideBytes = bp->framesPerUserBuffer * bp->bytesPerUserInputSample;

            /* setup non-interleaved ptrs */
            for( i=0; i<bp->inputChannelCount; ++i )
            {
                bp->tempInputBufferPtrs[i] = ((u8*)bp->tempInputBuffer) +
                    i * bp->bytesPerUserInputSample * bp->framesPerUserBuffer;
            }

            userInput = bp->tempInputBufferPtrs;
        }

        for( i=0; i<bp->inputChannelCount; ++i )
        {
            bp->inputConverter( destBytePtr, destSampleStrideSamples,
                                    hostInputChannels[i].data,
                                    hostInputChannels[i].stride,
                                    frameCount, &bp->ditherGenerator );

            destBytePtr += destChannelStrideBytes;  /* skip to next destination channel */

            /* advance src ptr for next iteration */
            hostInputChannels[i].data = ((u8*)hostInputChannels[i].data) +
                    frameCount * hostInputChannels[i].stride * bp->bytesPerHostInputSample;
        }

        bp->framesInTempInputBuffer += frameCount;

        if( bp->framesInTempInputBuffer == bp->framesPerUserBuffer )
        {
            /**
            @todo (non-critical optimisation)
            The conditional below implements the continue/complete/abort mechanism
            simply by continuing on iterating through the input buffer, but not
            passing the data to the callback. With care, the outer loop could be
            terminated earlier, thus some unneeded conversion cycles would be
            saved.
            */
            if( *streamCallbackResult == paContinue )
            {
                bp->timeInfo->outputBufferDacTime = 0;

                *streamCallbackResult = bp->streamCallback( userInput, userOutput,
                        bp->framesPerUserBuffer, bp->timeInfo,
                        bp->callbackStatusFlags, bp->userData );

                bp->timeInfo->inputBufferAdcTime += bp->framesPerUserBuffer * bp->samplePeriod;
            }

            bp->framesInTempInputBuffer = 0;
        }

        framesProcessed += frameCount;

        framesToGo -= frameCount;
    }while( framesToGo > 0 );

    return framesProcessed;
}


/*
    AdaptingOutputOnlyProcess() is a half duplex output buffer processor.
    It converts data from the temporary output buffer, to the output buffers,
    when the temporary output buffer is empty, it calls the streamCallback.
*/
static u64 AdaptingOutputOnlyProcess( PaUtilBufferProcessor *bp,
        i32 *streamCallbackResult,
        PaUtilChannelDescriptor *hostOutputChannels,
        u64 framesToProcess )
{
    uk userInput, *userOutput;
    u8 *srcBytePtr;
    u32 srcSampleStrideSamples; /* stride from one sample to the next within a channel, in samples */
    u32 srcChannelStrideBytes;  /* stride from one channel to the next, in bytes */
    u32 i;
    u64 frameCount;
    u64 framesToGo = framesToProcess;
    u64 framesProcessed = 0;

    do
    {
        if( bp->framesInTempOutputBuffer == 0 && *streamCallbackResult == paContinue )
        {
            userInput = 0;

            /* setup userOutput */
            if( bp->userOutputIsInterleaved )
            {
                userOutput = bp->tempOutputBuffer;
            }
            else /* user output is not interleaved */
            {
                for( i = 0; i < bp->outputChannelCount; ++i )
                {
                    bp->tempOutputBufferPtrs[i] = ((u8*)bp->tempOutputBuffer) +
                            i * bp->framesPerUserBuffer * bp->bytesPerUserOutputSample;
                }

                userOutput = bp->tempOutputBufferPtrs;
            }

            bp->timeInfo->inputBufferAdcTime = 0;

            *streamCallbackResult = bp->streamCallback( userInput, userOutput,
                    bp->framesPerUserBuffer, bp->timeInfo,
                    bp->callbackStatusFlags, bp->userData );

            if( *streamCallbackResult == paAbort )
            {
                /* if the callback returned paAbort, we disregard its output */
            }
            else
            {
                bp->timeInfo->outputBufferDacTime += bp->framesPerUserBuffer * bp->samplePeriod;

                bp->framesInTempOutputBuffer = bp->framesPerUserBuffer;
            }
        }

        if( bp->framesInTempOutputBuffer > 0 )
        {
            /* convert frameCount frames from user buffer to host buffer */

            frameCount = PA_MIN_( bp->framesInTempOutputBuffer, framesToGo );

            if( bp->userOutputIsInterleaved )
            {
                srcBytePtr = ((u8*)bp->tempOutputBuffer) +
                        bp->bytesPerUserOutputSample * bp->outputChannelCount *
                        (bp->framesPerUserBuffer - bp->framesInTempOutputBuffer);

                srcSampleStrideSamples = bp->outputChannelCount;
                srcChannelStrideBytes = bp->bytesPerUserOutputSample;
            }
            else /* user output is not interleaved */
            {
                srcBytePtr = ((u8*)bp->tempOutputBuffer) +
                        bp->bytesPerUserOutputSample *
                        (bp->framesPerUserBuffer - bp->framesInTempOutputBuffer);

                srcSampleStrideSamples = 1;
                srcChannelStrideBytes = bp->framesPerUserBuffer * bp->bytesPerUserOutputSample;
            }

            for( i=0; i<bp->outputChannelCount; ++i )
            {
                bp->outputConverter(    hostOutputChannels[i].data,
                                        hostOutputChannels[i].stride,
                                        srcBytePtr, srcSampleStrideSamples,
                                        frameCount, &bp->ditherGenerator );

                srcBytePtr += srcChannelStrideBytes;  /* skip to next source channel */

                /* advance dest ptr for next iteration */
                hostOutputChannels[i].data = ((u8*)hostOutputChannels[i].data) +
                        frameCount * hostOutputChannels[i].stride * bp->bytesPerHostOutputSample;
            }

            bp->framesInTempOutputBuffer -= frameCount;
        }
        else
        {
            /* no more user data is available because the callback has returned
                paComplete or paAbort. Fill the remainder of the host buffer
                with zeros.
            */

            frameCount = framesToGo;

            for( i=0; i<bp->outputChannelCount; ++i )
            {
                bp->outputZeroer(   hostOutputChannels[i].data,
                                    hostOutputChannels[i].stride,
                                    frameCount );

                /* advance dest ptr for next iteration */
                hostOutputChannels[i].data = ((u8*)hostOutputChannels[i].data) +
                        frameCount * hostOutputChannels[i].stride * bp->bytesPerHostOutputSample;
            }
        }

        framesProcessed += frameCount;

        framesToGo -= frameCount;

    }while( framesToGo > 0 );

    return framesProcessed;
}

/* CopyTempOutputBuffersToHostOutputBuffers is called from AdaptingProcess to copy frames from
    tempOutputBuffer to hostOutputChannels. This includes data conversion
    and interleaving.
*/
static void CopyTempOutputBuffersToHostOutputBuffers( PaUtilBufferProcessor *bp)
{
    u64 maxFramesToCopy;
    PaUtilChannelDescriptor *hostOutputChannels;
    u32 frameCount;
    u8 *srcBytePtr;
    u32 srcSampleStrideSamples; /* stride from one sample to the next within a channel, in samples */
    u32 srcChannelStrideBytes; /* stride from one channel to the next, in bytes */
    u32 i;

    /* copy frames from user to host output buffers */
    while( bp->framesInTempOutputBuffer > 0 &&
            ((bp->hostOutputFrameCount[0] + bp->hostOutputFrameCount[1]) > 0) )
    {
        maxFramesToCopy = bp->framesInTempOutputBuffer;

        /* select the output buffer set (1st or 2nd) */
        if( bp->hostOutputFrameCount[0] > 0 )
        {
            hostOutputChannels = bp->hostOutputChannels[0];
            frameCount = PA_MIN_( bp->hostOutputFrameCount[0], maxFramesToCopy );
        }
        else
        {
            hostOutputChannels = bp->hostOutputChannels[1];
            frameCount = PA_MIN_( bp->hostOutputFrameCount[1], maxFramesToCopy );
        }

        if( bp->userOutputIsInterleaved )
        {
            srcBytePtr = ((u8*)bp->tempOutputBuffer) +
                    bp->bytesPerUserOutputSample * bp->outputChannelCount *
                    (bp->framesPerUserBuffer - bp->framesInTempOutputBuffer);

            srcSampleStrideSamples = bp->outputChannelCount;
            srcChannelStrideBytes = bp->bytesPerUserOutputSample;
        }
        else /* user output is not interleaved */
        {
            srcBytePtr = ((u8*)bp->tempOutputBuffer) +
                    bp->bytesPerUserOutputSample *
                    (bp->framesPerUserBuffer - bp->framesInTempOutputBuffer);

            srcSampleStrideSamples = 1;
            srcChannelStrideBytes = bp->framesPerUserBuffer * bp->bytesPerUserOutputSample;
        }

        for( i=0; i<bp->outputChannelCount; ++i )
        {
            assert( hostOutputChannels[i].data != NULL );
            bp->outputConverter(    hostOutputChannels[i].data,
                                    hostOutputChannels[i].stride,
                                    srcBytePtr, srcSampleStrideSamples,
                                    frameCount, &bp->ditherGenerator );

            srcBytePtr += srcChannelStrideBytes;  /* skip to next source channel */

            /* advance dest ptr for next iteration */
            hostOutputChannels[i].data = ((u8*)hostOutputChannels[i].data) +
                    frameCount * hostOutputChannels[i].stride * bp->bytesPerHostOutputSample;
        }

        if( bp->hostOutputFrameCount[0] > 0 )
            bp->hostOutputFrameCount[0] -= frameCount;
        else
            bp->hostOutputFrameCount[1] -= frameCount;

        bp->framesInTempOutputBuffer -= frameCount;
    }
}

/*
    AdaptingProcess is a full duplex adapting buffer processor. It converts
    data from the temporary output buffer into the host output buffers, then
    from the host input buffers into the temporary input buffers. Calling the
    streamCallback when necessary.
    When processPartialUserBuffers is 0, all available input data will be
    consumed and all available output space will be filled. When
    processPartialUserBuffers is non-zero, as many full user buffers
    as possible will be processed, but partial buffers will not be consumed.
*/
static u64 AdaptingProcess( PaUtilBufferProcessor *bp,
        i32 *streamCallbackResult, i32 processPartialUserBuffers )
{
    uk userInput, *userOutput;
    u64 framesProcessed = 0;
    u64 framesAvailable;
    u64 endProcessingMinFrameCount;
    u64 maxFramesToCopy;
    PaUtilChannelDescriptor *hostInputChannels, *hostOutputChannels;
    u32 frameCount;
    u8 *destBytePtr;
    u32 destSampleStrideSamples; /* stride from one sample to the next within a channel, in samples */
    u32 destChannelStrideBytes; /* stride from one channel to the next, in bytes */
    u32 i, j;


    framesAvailable = bp->hostInputFrameCount[0] + bp->hostInputFrameCount[1];/* this is assumed to be the same as the output buffer's frame count */

    if( processPartialUserBuffers )
        endProcessingMinFrameCount = 0;
    else
        endProcessingMinFrameCount = (bp->framesPerUserBuffer - 1);

    /* Fill host output with remaining frames in user output (tempOutputBuffer) */
    CopyTempOutputBuffersToHostOutputBuffers( bp );

    while( framesAvailable > endProcessingMinFrameCount )
    {

        if( bp->framesInTempOutputBuffer == 0 && *streamCallbackResult != paContinue )
        {
            /* the callback will not be called any more, so zero what remains
                of the host output buffers */

            for( i=0; i<2; ++i )
            {
                frameCount = bp->hostOutputFrameCount[i];
                if( frameCount > 0 )
                {
                    hostOutputChannels = bp->hostOutputChannels[i];

                    for( j=0; j<bp->outputChannelCount; ++j )
                    {
                        bp->outputZeroer(   hostOutputChannels[j].data,
                                            hostOutputChannels[j].stride,
                                            frameCount );

                        /* advance dest ptr for next iteration  */
                        hostOutputChannels[j].data = ((u8*)hostOutputChannels[j].data) +
                                frameCount * hostOutputChannels[j].stride * bp->bytesPerHostOutputSample;
                    }
                    bp->hostOutputFrameCount[i] = 0;
                }
            }
        }


        /* copy frames from host to user input buffers */
        while( bp->framesInTempInputBuffer < bp->framesPerUserBuffer &&
                ((bp->hostInputFrameCount[0] + bp->hostInputFrameCount[1]) > 0) )
        {
            maxFramesToCopy = bp->framesPerUserBuffer - bp->framesInTempInputBuffer;

            /* select the input buffer set (1st or 2nd) */
            if( bp->hostInputFrameCount[0] > 0 )
            {
                hostInputChannels = bp->hostInputChannels[0];
                frameCount = PA_MIN_( bp->hostInputFrameCount[0], maxFramesToCopy );
            }
            else
            {
                hostInputChannels = bp->hostInputChannels[1];
                frameCount = PA_MIN_( bp->hostInputFrameCount[1], maxFramesToCopy );
            }

            /* configure conversion destination pointers */
            if( bp->userInputIsInterleaved )
            {
                destBytePtr = ((u8*)bp->tempInputBuffer) +
                        bp->bytesPerUserInputSample * bp->inputChannelCount *
                        bp->framesInTempInputBuffer;

                destSampleStrideSamples = bp->inputChannelCount;
                destChannelStrideBytes = bp->bytesPerUserInputSample;
            }
            else /* user input is not interleaved */
            {
                destBytePtr = ((u8*)bp->tempInputBuffer) +
                        bp->bytesPerUserInputSample * bp->framesInTempInputBuffer;

                destSampleStrideSamples = 1;
                destChannelStrideBytes = bp->framesPerUserBuffer * bp->bytesPerUserInputSample;
            }

            for( i=0; i<bp->inputChannelCount; ++i )
            {
                bp->inputConverter( destBytePtr, destSampleStrideSamples,
                                        hostInputChannels[i].data,
                                        hostInputChannels[i].stride,
                                        frameCount, &bp->ditherGenerator );

                destBytePtr += destChannelStrideBytes;  /* skip to next destination channel */

                /* advance src ptr for next iteration */
                hostInputChannels[i].data = ((u8*)hostInputChannels[i].data) +
                        frameCount * hostInputChannels[i].stride * bp->bytesPerHostInputSample;
            }

            if( bp->hostInputFrameCount[0] > 0 )
                bp->hostInputFrameCount[0] -= frameCount;
            else
                bp->hostInputFrameCount[1] -= frameCount;

            bp->framesInTempInputBuffer += frameCount;

            /* update framesAvailable and framesProcessed based on input consumed
                unless something is very wrong this will also correspond to the
                amount of output generated */
            framesAvailable -= frameCount;
            framesProcessed += frameCount;
        }

        /* call streamCallback */
        if( bp->framesInTempInputBuffer == bp->framesPerUserBuffer &&
            bp->framesInTempOutputBuffer == 0 )
        {
            if( *streamCallbackResult == paContinue )
            {
                /* setup userInput */
                if( bp->userInputIsInterleaved )
                {
                    userInput = bp->tempInputBuffer;
                }
                else /* user input is not interleaved */
                {
                    for( i = 0; i < bp->inputChannelCount; ++i )
                    {
                        bp->tempInputBufferPtrs[i] = ((u8*)bp->tempInputBuffer) +
                                i * bp->framesPerUserBuffer * bp->bytesPerUserInputSample;
                    }

                    userInput = bp->tempInputBufferPtrs;
                }

                /* setup userOutput */
                if( bp->userOutputIsInterleaved )
                {
                    userOutput = bp->tempOutputBuffer;
                }
                else /* user output is not interleaved */
                {
                    for( i = 0; i < bp->outputChannelCount; ++i )
                    {
                        bp->tempOutputBufferPtrs[i] = ((u8*)bp->tempOutputBuffer) +
                                i * bp->framesPerUserBuffer * bp->bytesPerUserOutputSample;
                    }

                    userOutput = bp->tempOutputBufferPtrs;
                }

                /* call streamCallback */

                *streamCallbackResult = bp->streamCallback( userInput, userOutput,
                        bp->framesPerUserBuffer, bp->timeInfo,
                        bp->callbackStatusFlags, bp->userData );

                bp->timeInfo->inputBufferAdcTime += bp->framesPerUserBuffer * bp->samplePeriod;
                bp->timeInfo->outputBufferDacTime += bp->framesPerUserBuffer * bp->samplePeriod;

                bp->framesInTempInputBuffer = 0;

                if( *streamCallbackResult == paAbort )
                    bp->framesInTempOutputBuffer = 0;
                else
                    bp->framesInTempOutputBuffer = bp->framesPerUserBuffer;
            }
            else
            {
                /* paComplete or paAbort has already been called. */

                bp->framesInTempInputBuffer = 0;
            }
        }

        /* copy frames from user (tempOutputBuffer) to host output buffers (hostOutputChannels)
           Means to process the user output provided by the callback. Has to be called after
            each callback. */
        CopyTempOutputBuffersToHostOutputBuffers( bp );

    }

    return framesProcessed;
}


u64 PaUtil_EndBufferProcessing( PaUtilBufferProcessor* bp, i32 *streamCallbackResult )
{
    u64 framesToProcess, framesToGo;
    u64 framesProcessed = 0;

    if( bp->inputChannelCount != 0 && bp->outputChannelCount != 0
            && bp->hostInputChannels[0][0].data /* input was supplied (see PaUtil_SetNoInput) */
            && bp->hostOutputChannels[0][0].data /* output was supplied (see PaUtil_SetNoOutput) */ )
    {
        assert( (bp->hostInputFrameCount[0] + bp->hostInputFrameCount[1]) ==
                (bp->hostOutputFrameCount[0] + bp->hostOutputFrameCount[1]) );
    }

    assert( *streamCallbackResult == paContinue
            || *streamCallbackResult == paComplete
            || *streamCallbackResult == paAbort ); /* don't forget to pass in a valid callback result value */

    if( bp->useNonAdaptingProcess )
    {
        if( bp->inputChannelCount != 0 && bp->outputChannelCount != 0 )
        {
            /* full duplex non-adapting process, splice buffers if they are
                different lengths */

            framesToGo = bp->hostOutputFrameCount[0] + bp->hostOutputFrameCount[1]; /* relies on assert above for input/output equivalence */

            do{
                u64 noInputInputFrameCount;
                u64 *hostInputFrameCount;
                PaUtilChannelDescriptor *hostInputChannels;
                u64 noOutputOutputFrameCount;
                u64 *hostOutputFrameCount;
                PaUtilChannelDescriptor *hostOutputChannels;
                u64 framesProcessedThisIteration;

                if( !bp->hostInputChannels[0][0].data )
                {
                    /* no input was supplied (see PaUtil_SetNoInput)
                        NonAdaptingProcess knows how to deal with this
                    */
                    noInputInputFrameCount = framesToGo;
                    hostInputFrameCount = &noInputInputFrameCount;
                    hostInputChannels = 0;
                }
                else if( bp->hostInputFrameCount[0] != 0 )
                {
                    hostInputFrameCount = &bp->hostInputFrameCount[0];
                    hostInputChannels = bp->hostInputChannels[0];
                }
                else
                {
                    hostInputFrameCount = &bp->hostInputFrameCount[1];
                    hostInputChannels = bp->hostInputChannels[1];
                }

                if( !bp->hostOutputChannels[0][0].data )
                {
                    /* no output was supplied (see PaUtil_SetNoOutput)
                        NonAdaptingProcess knows how to deal with this
                    */
                    noOutputOutputFrameCount = framesToGo;
                    hostOutputFrameCount = &noOutputOutputFrameCount;
                    hostOutputChannels = 0;
                }
                if( bp->hostOutputFrameCount[0] != 0 )
                {
                    hostOutputFrameCount = &bp->hostOutputFrameCount[0];
                    hostOutputChannels = bp->hostOutputChannels[0];
                }
                else
                {
                    hostOutputFrameCount = &bp->hostOutputFrameCount[1];
                    hostOutputChannels = bp->hostOutputChannels[1];
                }

                framesToProcess = PA_MIN_( *hostInputFrameCount,
                                       *hostOutputFrameCount );

                assert( framesToProcess != 0 );

                framesProcessedThisIteration = NonAdaptingProcess( bp, streamCallbackResult,
                        hostInputChannels, hostOutputChannels,
                        framesToProcess );

                *hostInputFrameCount -= framesProcessedThisIteration;
                *hostOutputFrameCount -= framesProcessedThisIteration;

                framesProcessed += framesProcessedThisIteration;
                framesToGo -= framesProcessedThisIteration;

            }while( framesToGo > 0 );
        }
        else
        {
            /* half duplex non-adapting process, just process 1st and 2nd buffer */
            /* process first buffer */

            framesToProcess = (bp->inputChannelCount != 0)
                            ? bp->hostInputFrameCount[0]
                            : bp->hostOutputFrameCount[0];

            framesProcessed = NonAdaptingProcess( bp, streamCallbackResult,
                        bp->hostInputChannels[0], bp->hostOutputChannels[0],
                        framesToProcess );

            /* process second buffer if provided */

            framesToProcess = (bp->inputChannelCount != 0)
                            ? bp->hostInputFrameCount[1]
                            : bp->hostOutputFrameCount[1];
            if( framesToProcess > 0 )
            {
                framesProcessed += NonAdaptingProcess( bp, streamCallbackResult,
                    bp->hostInputChannels[1], bp->hostOutputChannels[1],
                    framesToProcess );
            }
        }
    }
    else /* block adaption necessary*/
    {

        if( bp->inputChannelCount != 0 && bp->outputChannelCount != 0 )
        {
            /* full duplex */

            if( bp->hostBufferSizeMode == paUtilVariableHostBufferSizePartialUsageAllowed  )
            {
                framesProcessed = AdaptingProcess( bp, streamCallbackResult,
                        0 /* dont process partial user buffers */ );
            }
            else
            {
                framesProcessed = AdaptingProcess( bp, streamCallbackResult,
                        1 /* process partial user buffers */ );
            }
        }
        else if( bp->inputChannelCount != 0 )
        {
            /* input only */
            framesToProcess = bp->hostInputFrameCount[0];

            framesProcessed = AdaptingInputOnlyProcess( bp, streamCallbackResult,
                        bp->hostInputChannels[0], framesToProcess );

            framesToProcess = bp->hostInputFrameCount[1];
            if( framesToProcess > 0 )
            {
                framesProcessed += AdaptingInputOnlyProcess( bp, streamCallbackResult,
                        bp->hostInputChannels[1], framesToProcess );
            }
        }
        else
        {
            /* output only */
            framesToProcess = bp->hostOutputFrameCount[0];

            framesProcessed = AdaptingOutputOnlyProcess( bp, streamCallbackResult,
                        bp->hostOutputChannels[0], framesToProcess );

            framesToProcess = bp->hostOutputFrameCount[1];
            if( framesToProcess > 0 )
            {
                framesProcessed += AdaptingOutputOnlyProcess( bp, streamCallbackResult,
                        bp->hostOutputChannels[1], framesToProcess );
            }
        }
    }

    return framesProcessed;
}


i32 PaUtil_IsBufferProcessorOutputEmpty( PaUtilBufferProcessor* bp )
{
    return (bp->framesInTempOutputBuffer) ? 0 : 1;
}


u64 PaUtil_CopyInput( PaUtilBufferProcessor* bp,
        uk *buffer, u64 frameCount )
{
    PaUtilChannelDescriptor *hostInputChannels;
    u32 framesToCopy;
    u8 *destBytePtr;
    uk *nonInterleavedDestPtrs;
    u32 destSampleStrideSamples; /* stride from one sample to the next within a channel, in samples */
    u32 destChannelStrideBytes; /* stride from one channel to the next, in bytes */
    u32 i;

    hostInputChannels = bp->hostInputChannels[0];
    framesToCopy = PA_MIN_( bp->hostInputFrameCount[0], frameCount );

    if( bp->userInputIsInterleaved )
    {
        destBytePtr = (u8*)*buffer;

        destSampleStrideSamples = bp->inputChannelCount;
        destChannelStrideBytes = bp->bytesPerUserInputSample;

        for( i=0; i<bp->inputChannelCount; ++i )
        {
            bp->inputConverter( destBytePtr, destSampleStrideSamples,
                                hostInputChannels[i].data,
                                hostInputChannels[i].stride,
                                framesToCopy, &bp->ditherGenerator );

            destBytePtr += destChannelStrideBytes;  /* skip to next dest channel */

            /* advance source ptr for next iteration */
            hostInputChannels[i].data = ((u8*)hostInputChannels[i].data) +
                    framesToCopy * hostInputChannels[i].stride * bp->bytesPerHostInputSample;
        }

        /* advance callers dest pointer (buffer) */
        *buffer = ((u8*)*buffer) +
                framesToCopy * bp->inputChannelCount * bp->bytesPerUserInputSample;
    }
    else
    {
        /* user input is not interleaved */

        nonInterleavedDestPtrs = (uk *)*buffer;

        destSampleStrideSamples = 1;

        for( i=0; i<bp->inputChannelCount; ++i )
        {
            destBytePtr = (u8*)nonInterleavedDestPtrs[i];

            bp->inputConverter( destBytePtr, destSampleStrideSamples,
                                hostInputChannels[i].data,
                                hostInputChannels[i].stride,
                                framesToCopy, &bp->ditherGenerator );

            /* advance callers dest pointer (nonInterleavedDestPtrs[i]) */
            destBytePtr += bp->bytesPerUserInputSample * framesToCopy;
            nonInterleavedDestPtrs[i] = destBytePtr;

            /* advance source ptr for next iteration */
            hostInputChannels[i].data = ((u8*)hostInputChannels[i].data) +
                    framesToCopy * hostInputChannels[i].stride * bp->bytesPerHostInputSample;
        }
    }

    bp->hostInputFrameCount[0] -= framesToCopy;

    return framesToCopy;
}

u64 PaUtil_CopyOutput( PaUtilBufferProcessor* bp,
        ukk* buffer, u64 frameCount )
{
    PaUtilChannelDescriptor *hostOutputChannels;
    u32 framesToCopy;
    u8 *srcBytePtr;
    uk *nonInterleavedSrcPtrs;
    u32 srcSampleStrideSamples; /* stride from one sample to the next within a channel, in samples */
    u32 srcChannelStrideBytes; /* stride from one channel to the next, in bytes */
    u32 i;

    hostOutputChannels = bp->hostOutputChannels[0];
    framesToCopy = PA_MIN_( bp->hostOutputFrameCount[0], frameCount );

    if( bp->userOutputIsInterleaved )
    {
        srcBytePtr = (u8*)*buffer;

        srcSampleStrideSamples = bp->outputChannelCount;
        srcChannelStrideBytes = bp->bytesPerUserOutputSample;

        for( i=0; i<bp->outputChannelCount; ++i )
        {
            bp->outputConverter(    hostOutputChannels[i].data,
                                    hostOutputChannels[i].stride,
                                    srcBytePtr, srcSampleStrideSamples,
                                    framesToCopy, &bp->ditherGenerator );

            srcBytePtr += srcChannelStrideBytes;  /* skip to next source channel */

            /* advance dest ptr for next iteration */
            hostOutputChannels[i].data = ((u8*)hostOutputChannels[i].data) +
                    framesToCopy * hostOutputChannels[i].stride * bp->bytesPerHostOutputSample;
        }

        /* advance callers source pointer (buffer) */
        *buffer = ((u8*)*buffer) +
                framesToCopy * bp->outputChannelCount * bp->bytesPerUserOutputSample;

    }
    else
    {
        /* user output is not interleaved */

        nonInterleavedSrcPtrs = (uk *)*buffer;

        srcSampleStrideSamples = 1;

        for( i=0; i<bp->outputChannelCount; ++i )
        {
            srcBytePtr = (u8*)nonInterleavedSrcPtrs[i];

            bp->outputConverter(    hostOutputChannels[i].data,
                                    hostOutputChannels[i].stride,
                                    srcBytePtr, srcSampleStrideSamples,
                                    framesToCopy, &bp->ditherGenerator );


            /* advance callers source pointer (nonInterleavedSrcPtrs[i]) */
            srcBytePtr += bp->bytesPerUserOutputSample * framesToCopy;
            nonInterleavedSrcPtrs[i] = srcBytePtr;

            /* advance dest ptr for next iteration */
            hostOutputChannels[i].data = ((u8*)hostOutputChannels[i].data) +
                    framesToCopy * hostOutputChannels[i].stride * bp->bytesPerHostOutputSample;
        }
    }

    bp->hostOutputFrameCount[0] += framesToCopy;

    return framesToCopy;
}


u64 PaUtil_ZeroOutput( PaUtilBufferProcessor* bp, u64 frameCount )
{
    PaUtilChannelDescriptor *hostOutputChannels;
    u32 framesToZero;
    u32 i;

    hostOutputChannels = bp->hostOutputChannels[0];
    framesToZero = PA_MIN_( bp->hostOutputFrameCount[0], frameCount );

    for( i=0; i<bp->outputChannelCount; ++i )
    {
        bp->outputZeroer(   hostOutputChannels[i].data,
                            hostOutputChannels[i].stride,
                            framesToZero );


        /* advance dest ptr for next iteration */
        hostOutputChannels[i].data = ((u8*)hostOutputChannels[i].data) +
                framesToZero * hostOutputChannels[i].stride * bp->bytesPerHostOutputSample;
    }

    bp->hostOutputFrameCount[0] += framesToZero;

    return framesToZero;
}
