/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"
#include "filters/SRCFilters.h"

static int numtaps_2ch = 13;

/*****************************************************************************
 *
 * SRCInit initializes the persistent state of the sample rate converter.  It
 * must be called before SRCFilter or SRCFilter_S. It is the responsibility
 * of the caller to allocate the memory needed by the sample rate converter,
 * for both the persistent state structure and the delay line(s).  The number
 * of elements in the delay line array must be (Max input samples per
 * SRCFilter call) + NUMTAPS (less will result in errors and more will go
 * unused).
 *
 *****************************************************************************/
short lastSampleLeft = 0, lastSampleRight = 0;
int SRCInit(SRCState *pSRC, unsigned long ulInputRate, unsigned long ulOutputRate)
{
    long lNumPolyPhases, lSampleIncrement, lNumTaps;
    short sTap;

    lastSampleLeft = 0;
    lastSampleRight = 0;
    pSRC->last_sample_num = 0;
    pSRC->process_num = 160;

    switch ((ulOutputRate << 16) / ulInputRate)
    {
    //32k->16k
    case _32K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_0005_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_0_0005_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_0_0005_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_0005_16bit_2ch.sCoefs;
        break;
    }
    //44.1k->16k
    case _44K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_3628_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_0_3628_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_0_3628_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_3628_16bit_2ch.sCoefs;
        break;
    }
    //48k->16k
    case _48K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_3333_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_0_3333_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_0_3333_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_3333_16bit_2ch.sCoefs;
        break;
    }
    // 8k->16k. 22k->44k
    case _8K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_2_0_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_2_0_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_2_0_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_2_0_16bit_2ch.sCoefs;
        break;
    }

    //22k->16k
    case _22K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_7256_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_0_7256_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_0_7256_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_7256_16bit_2ch.sCoefs;
        break;
    }
    //24k->16k
    case _24K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_6666_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_0_6666_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_0_6666_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_6666_16bit_2ch.sCoefs;
        break;
    }
    //11k->16k
    case _11K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_1_4512_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_1_4512_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_1_4512_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_1_4512_16bit_2ch.sCoefs;
        break;
    }
    //8k->44k
    case _8K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_5_5125_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_5_5125_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_5_5125_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_5_5125_16bit_2ch.sCoefs;
        break;
    }
    //11k->44k
    case _11K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_4_0000_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_4_0000_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_4_0000_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_4_0000_16bit_2ch.sCoefs;
        break;
    }
    //16k->44k
    case _16K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_2_7562_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_2_7562_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_2_7562_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_2_7562_16bit_2ch.sCoefs;
        break;
    }
    //32k->44k
    case _32K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_1_3781_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_1_3781_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_1_3781_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_1_3781_16bit_2ch.sCoefs;
        break;
    }
    //48k->44k
    case _48K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_9187_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_0_9187_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_0_9187_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_9187_16bit_2ch.sCoefs;
        break;
    }
    //8k->48k
    case _8K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_6_0000_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_6_0000_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_6_0000_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_6_0000_16bit_2ch.sCoefs;
        break;
    }
    //11k->48k
    case _11K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_4_3537_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_4_3537_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_4_3537_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_4_3537_16bit_2ch.sCoefs;
        break;
    }
    //16k->48k
    case _16K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_3_0000_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_3_0000_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_3_0000_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_3_0000_16bit_2ch.sCoefs;
        break;
    }
    //22k->48k
    case _22K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_2_1768_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_2_1768_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_2_1768_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_2_1768_16bit_2ch.sCoefs;
        break;
    }
    //32k->48k
    case _32K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_1_5000_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_1_5000_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_1_5000_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_1_5000_16bit_2ch.sCoefs;
        break;
    }
    //44k->48k
    case _44K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_1_0884_16bit_2ch.sNumPolyPhases;
        lNumTaps = SRCFilter_1_0884_16bit_2ch.sNumTaps;
        lSampleIncrement = SRCFilter_1_0884_16bit_2ch.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_1_0884_16bit_2ch.sCoefs;
        break;
    }
    default:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        RK_AUDIO_LOG_D("Not support this rate :%d %lx\n", ulInputRate, (ulOutputRate << 16) / ulInputRate);
        return (0);
    }
    }
    RK_AUDIO_LOG_D("SRCInit:%d, NUMTAPS:%d ", __LINE__, numtaps_2ch);
    memset((void *)pSRC->Left_right, 0, numtaps_2ch * 4);
    RK_AUDIO_LOG_D("0x%x\n", (int)((ulOutputRate << 16) / ulInputRate));
    RK_AUDIO_LOG_D("ulOutputRate:%ld,ulInputRate:%ld\n", ulOutputRate, ulInputRate);

    //
    // Make sure that the number of taps in the filter matches the number of
    // taps supported by our filtering code.
    //
    numtaps_2ch = lNumTaps;
    if (lNumTaps != numtaps_2ch)
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        return (0);
    }

    //
    // Initialize the persistent SRC state.
    //
    RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
    pSRC->lFilterOffset = 0;
    pSRC->lFilterIncr = lSampleIncrement * numtaps_2ch;//eg:48k->44k,25*13
    pSRC->lFilterSize = lNumPolyPhases * numtaps_2ch;//23*13

    //
    // Set the initial state of the delay lines to silence.
    //
    for (sTap = 0; sTap < numtaps_2ch; sTap++)
    {
        pSRC->Left_right[2 * sTap] = 0;
        pSRC->Left_right[2 * sTap + 1] = 0;
    }
    RK_AUDIO_LOG_D("SRCInit end\n");

    //
    // We've successfully initialized the requested sample rate converter.
    //
    return (1);
}

//****************************************************************************
//
// SRCFilterStereo runs the sample rate conversion filter over the given
// streams of samples, thereby performing the sample rate conversion requested
// by the initial call to SRCInit.    This function works on a pair of mono
// streams of samples.
// lNumInputSamples为单声道输入采样点数
// plNumOutputSamples为单声道输出采样点数
//****************************************************************************
void SRCFilterStereo(SRCState *pSRC, short *psInDataLeft, short *psInDataRight,
                     short *psOutDataLeft, short *psOutDataRight,
                     long lNumInputSamples, long *plNumOutputSamples)
{
    long lOutDataLeft, lOutDataRight;
    short *psPtr1, *psPtr2, *psPtr3;
    short *psSampleLeft, *psSampleRight, sCoeff;
    int iLoop;
    int i;
    int count = 0;
    //RK_AUDIO_LOG_D("%d SRCFilterStereo, NUMTAPS:%d\n",__LINE__,numtaps_2ch);
    memcpy(psInDataLeft - (numtaps_2ch << 1), pSRC->Left_right, (numtaps_2ch << 2));
#if 1
#if 1
    i = 1;
    while (1)
    {
        psSampleLeft = psInDataLeft - 2 * i;
        psSampleRight = psInDataRight - 2 * i;
        i++;
        if ((*psSampleLeft == lastSampleLeft) && (*psSampleRight == lastSampleRight))
        {
            break;
        }

    }
    //RK_AUDIO_LOG_D("%d SRCFilterStereo, NUMTAPS:%d\n",__LINE__,numtaps_2ch);
#endif

    //
    // Compute the number of output samples which we will produce.
    //

    //RK_AUDIO_LOG_D("SRCFilterStereo:%d\n",__LINE__);
    iLoop = ((((lNumInputSamples + i - 1) * pSRC->lFilterSize) -
              pSRC->lFilterOffset - 1) / pSRC->lFilterIncr);

    *plNumOutputSamples = iLoop;

    // Loop through each output sample.
    //
    //RK_AUDIO_LOG_D("%d SRCFilterStereo, NUMTAPS:%d\n",__LINE__,numtaps_2ch);
    while (iLoop--)
    {
        //
        // Increment the offset into the filter.
        //
        pSRC->lFilterOffset += pSRC->lFilterIncr;

        // See if we've passed the entire filter, indicating that we've
        // consumed one of the input samples.
        //
        while (pSRC->lFilterOffset >= pSRC->lFilterSize)
        {
            //
            // We have, so wrap the filter offset (i.e. treat the filter as if
            // it were a circular buffer in memory).
            //
            pSRC->lFilterOffset -= pSRC->lFilterSize;

            //
            // Consume the input sample.
            //
            //if (flag == 1)
            {
                psSampleLeft += 2;
                psSampleRight += 2;
                //flag = 0;
            }

        }
        //flag = 1;
        //RK_AUDIO_LOG_D("%d\n",pSRC->lFilterOffset);
        //
        // Get pointers to the filter and the two sample data streams.
        //
        psPtr1 = pSRC->psFilter + pSRC->lFilterOffset;
        //RK_AUDIO_LOG_D("pSRC->lFilterOffset = %d\n", pSRC->lFilterOffset);
        count++;

        psPtr2 = psSampleLeft;
        psPtr3 = psSampleRight;

        //
        // Run the filter over the two sample streams.    The number of MACs here
        // must agree with NUMTAPS.
        //
        lOutDataLeft = 0;
        lOutDataRight = 0;
        sCoeff = *psPtr1++;
        i = 0;
        while (i < numtaps_2ch)
        {

            lOutDataLeft += sCoeff * *psPtr2;
            lOutDataRight += sCoeff * *psPtr3;
            if (i != (numtaps_2ch - 1))
            {
                sCoeff = *psPtr1++;
                psPtr2 -= 2;
                psPtr3 -= 2;

            }
            i++;

        }

        // Clip the filtered samples if necessary.
        //
        if ((lOutDataLeft + 0x40000000) < 0)
        {
            lOutDataLeft = (lOutDataLeft & 0x80000000) ? 0xc0000000 :
                           0x3fffffff;
        }
        if ((lOutDataRight + 0x40000000) < 0)
        {
            lOutDataRight = (lOutDataRight & 0x80000000) ? 0xc0000000 :
                            0x3fffffff;
        }

        //
        // Write out the samples.
        //
        *psOutDataLeft = (short)(lOutDataLeft >> 15);
        psOutDataLeft += 2;
        *psOutDataRight = (short)(lOutDataRight >> 15);
        psOutDataRight += 2;
    }
    //RK_AUDIO_LOG_D("%d SRCFilterStereo, NUMTAPS:%d\n",__LINE__,numtaps_2ch);
    //RK_AUDIO_LOG_D("SRCFilterStereo:%d\n",__LINE__);
    //pSRC->lFilterOffset = 0;//cdd 20161012

    // Copy the tail of the filter memory buffer to the beginning, therefore
    // "remembering" the last few samples to use when processing the next batch
    // of samples.
    //
    //if (pSRC->lFilterOffset   == 286)
    lastSampleLeft = *psSampleLeft;
    lastSampleRight = *psSampleRight;
    memcpy(pSRC->Left_right, psInDataLeft + (lNumInputSamples << 1) - (numtaps_2ch << 1), (numtaps_2ch << 2));
    //RK_AUDIO_LOG_D("SRCFilterStereo:%d\n",__LINE__);
#endif
}

//****************************************************************************
//
// SRCFilter runs the sample rate conversion filter over the given streams of
// samples, thereby performing the sample rate conversion requested
// by the initial call to SRCInit.
// long lNumSamples 双声道采样点数
// psInLeft指向buffer的26的位置
//****************************************************************************
long SRCFilter(SRCState *pSRC, short *psInLeft, short *psLeft, long lNumSamples)
{
    long lLength;
    long lNumOutputSamples;

    lLength = lNumSamples >> 1;
    SRCFilterStereo(pSRC, psInLeft, &psInLeft[1], &psLeft[0], &psLeft[1],
                    lLength, &lNumOutputSamples);
    return (lNumOutputSamples << 1); //返回双声道长度
}
