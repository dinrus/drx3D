#ifndef PA_WIN_WDMKS_H
#define PA_WIN_WDMKS_H
/*
 * $Id$
 * PortAudio Portable Real-Time Audio Library
 * WDM/KS specific extensions
 *
 * Copyright (c) 1999-2007 Ross Bencina and Phil Burk
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
 @ingroup public_header
 @brief WDM Kernel Streaming-specific PortAudio API extension header file.
*/


#include "portaudio.h"

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /** Flags to indicate valid fields in PaWinWDMKSInfo.
     @see PaWinWDMKSInfo
     @version Available as of 19.5.0.
    */
    typedef enum PaWinWDMKSFlags
    {
        /** Makes WDMKS use the supplied latency figures instead of relying on the frame size reported
         by the WaveCyclic device. Use at own risk!
        */
        paWinWDMKSOverrideFramesize   = (1 << 0),

        /** Makes WDMKS (output stream) use the given channelMask instead of the default.
          @version Available as of 19.5.0.
        */
        paWinWDMKSUseGivenChannelMask = (1 << 1),

    } PaWinWDMKSFlags;

    typedef struct PaWinWDMKSInfo{
        u64 size;             /**< sizeof(PaWinWDMKSInfo) */
        PaHostApiTypeId hostApiType;    /**< paWDMKS */
        u64 version;          /**< 1 */

        /** Flags indicate which fields are valid.
         @see PaWinWDMKSFlags
         @version Available as of 19.5.0.
        */
        u64 flags;

        /** The number of packets to use for WaveCyclic devices, range is [2, 8]. Set to zero for default value of 2. */
        unsigned noOfPackets;

        /** If paWinWDMKSUseGivenChannelMask bit is set in flags, use this as channelMask instead of default.
         @see PaWinWDMKSFlags
         @version Available as of 19.5.0.
        */
        unsigned channelMask;
    } PaWinWDMKSInfo;

    typedef enum PaWDMKSType
    {
        Type_kNotUsed,
        Type_kWaveCyclic,
        Type_kWaveRT,
        Type_kCnt,
    } PaWDMKSType;

    typedef enum PaWDMKSSubType
    {
        SubType_kUnknown,
        SubType_kNotification,
        SubType_kPolled,
        SubType_kCnt,
    } PaWDMKSSubType;

    typedef struct PaWinWDMKSDeviceInfo {
        wchar_t filterPath[MAX_PATH];     /**< KS filter path in Unicode! */
        wchar_t topologyPath[MAX_PATH];   /**< Topology filter path in Unicode! */
        PaWDMKSType streamingType;
        GUID deviceProductGuid;           /**< The product GUID of the device (if supported) */
    } PaWinWDMKSDeviceInfo;

    typedef struct PaWDMKSDirectionSpecificStreamInfo
    {
        PaDeviceIndex device;
        unsigned channels;                  /**< No of channels the device is opened with */
        unsigned framesPerHostBuffer;       /**< No of frames of the device buffer */
        i32 endpointPinId;                  /**< Endpoint pin ID (on topology filter if topologyName is not empty) */
        i32 muxNodeId;                      /**< Only valid for input */
        PaWDMKSSubType streamingSubType;       /**< Not known until device is opened for streaming */
    } PaWDMKSDirectionSpecificStreamInfo;

    typedef struct PaWDMKSSpecificStreamInfo {
        PaWDMKSDirectionSpecificStreamInfo input;
        PaWDMKSDirectionSpecificStreamInfo output;
    } PaWDMKSSpecificStreamInfo;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PA_WIN_DS_H */
