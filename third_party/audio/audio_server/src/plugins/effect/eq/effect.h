/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef __EFFECT_H__
#define __EFFECT_H__


#include "rkos_typedef.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/* EQ mode */
typedef enum
{
    EQ_NOR,
    EQ_BASS,
    EQ_HEAVY,
    EQ_POP,
    EQ_JAZZ,
    EQ_UNIQUE,
    EQ_USER

} eEQMode;

#ifdef CODEC_24BIT
typedef long    EQ_TYPE;
typedef long long   EQ_TYPE_LONG;
#else
typedef short    EQ_TYPE;
typedef long    EQ_TYPE_LONG;

#endif

/*parameter structure of RK audio effect.*/
/*
typedef __PACKED_STRUCT
{
    short    dbGain[5]; //5 band EQµÄÔöÒæ
} tRKEQCoef;
*/
typedef __PACKED_STRUCT
{
    short     dbGain[5];
} tRKEQCoef;

/*
typedef __PACKED_STRUCT
{
    eEQMode Mode;      // EQ mode
    unsigned short max_DbGain;
   //#ifdef _RK_EQ_
    tRKEQCoef  RKCoef;
   // #endif

    //tPFEQCoef  PFCoef;
} RKEffect;
*/
typedef __PACKED_STRUCT
{
    eEQMode Mode;
    unsigned short max_DbGain;
    tRKEQCoef RKCoef;
} RKEffect;

#define CUSTOMEQ_LEVELNUM        7

/* API interface function. */

rk_err_t Effect_Delete(void);
long EffectInit(void);       // initialization.
long EffectEnd(RKEffect *userEQ);      //handle over.
long EffectProcess(EQ_TYPE *pBuffer, uint32 PcmLen, RKEffect *userEQ);   //audio effect process function,call it every frame.

//this function is for adjust audio effect.
long EffectAdjust(RKEffect *userEQ, void *eqARG, uint32 sampleRate);
long RKEQAdjust(RKEffect *pEft, uint32 sampleRate);
void EQ_ClearBuff(void);
void ReadEqData(uint8 *p, uint32 off, uint32 size);
void RockEQReduce9dB(EQ_TYPE *pwBuffer, long cwBuffer, long mode, RKEffect *userEQ);
void RockEQAdjust(short SmpRate, short *g, short db, RKEffect *userEQ);
void RockEQProcess(EQ_TYPE *pData, uint32 PcmLen, RKEffect *userEQ);
rk_err_t rk_eq_init(void);
rk_err_t rk_eq_deinit(void);
#endif

