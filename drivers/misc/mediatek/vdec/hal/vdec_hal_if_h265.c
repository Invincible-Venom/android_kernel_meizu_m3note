#include "vdec_hw_common.h"
#include "vdec_hal_if_h265.h"
#include "vdec_hw_h265.h"
#include "vdec_hal_errcode.h"
#include "../include/vdec_info_common.h"

//#include "x_hal_ic.h"
//#include "x_hal_1176.h"
//#include "x_debug.h"

//extern VDEC_INFO_VERIFY_FILE_INFO_T _tRecFileInfo;
//extern VDEC_INFO_H265_FBUF_INFO_T _ptH265FBufInfo[17];
//extern char _bFileStr1[9][300];
#include "../include/drv_common.h"

#if CONFIG_DRV_VERIFY_SUPPORT
#include "../verify/vdec_verify_general.h"
#include "../verify/vdec_verify_mpv_prov.h"
#include "../verify/vdec_info_verify.h"

#include <linux/string.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/delay.h>

#if (!CONFIG_DRV_LINUX)
#include <string.h>
#include <stdio.h>
#include "x_printf.h"
#endif
#define MAX_INT                     0x7FFFFFFF  ///< max. value of signed 32-bit integer

//@
#ifdef HEVC_CRC_CMP
#define bit_true_compare  FALSE
#else
#define bit_true_compare  TRUE
#endif


extern BOOL fgWrMsg2PC(void *pvAddr, UINT32 u4Size, UINT32 u4Mode, VDEC_INFO_VERIFY_FILE_INFO_T *pFILE_INFO);
#endif

const CHAR DIAG_SCAN[16]  =
{
    0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

const CHAR DIAG_SCAN8[64] =
{
    0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

#ifdef MPV_DUMP_H265_DEC_REG
void VDec_DumpH265Reg(UCHAR ucMpvId);
#endif


// **************************************************************************
// Function : INT32 i4VDEC_HAL_H265_InitVDecHW(UINT32 u4VDecID)
// Description :Initialize video decoder hardware only for H265
// Parameter : u4VDecID : video decoder hardware ID
//                  prH265VDecInitPrm : pointer to VFIFO info struct
// Return      : =0: success.
//                  <0: fail.
// **************************************************************************
INT32 i4VDEC_HAL_H265_InitVDecHW(UINT32 u4VDecID)
{
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] i4VDEC_HAL_H265_InitVDecHW() start!!\n");
#endif

#if (CONFIG_CHIP_VER_CURR < CONFIG_CHIP_VER_MT8555)
    vVDecResetHW(u4VDecID);
#elif (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8560)
    vVDecResetHW(u4VDecID, VDEC_H265);
#else
    vVDecResetHW(u4VDecID, VDEC_UNKNOWN);
#endif

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] i4VDEC_HAL_H265_InitVDecHW() Done!!\n");
#endif

    return  HAL_HANDLE_OK;
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H265_ShiftGetBitStream(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits);
// Description :Read barrel shifter after shifting
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 u4ShiftBits : shift bits number
// Return      : Value of barrel shifter input window after shifting
// **************************************************************************
UINT32 u4VDEC_HAL_H265_ShiftGetBitStream(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits)
{
    UINT32 u4RegVal;

    u4RegVal = u4VDecHEVCVLDGetBitS(u4BSID, u4VDecID, u4ShiftBits);

    return (u4RegVal);
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H265_GetBitStreamShift(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits);
// Description :Read Barrel Shifter before shifting
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 u4ShiftBits : shift bits number
// Return      : Value of barrel shifter input window before shifting
// **************************************************************************
UINT32 u4VDEC_HAL_H265_GetBitStreamShift(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits)
{
    UINT32 u4RegVal0;

    u4RegVal0 = u4VDecHEVCVLDGetBitS(u4BSID, u4VDecID, 0);
    u4VDecHEVCVLDGetBitS(u4BSID, u4VDecID, u4ShiftBits);

    return (u4RegVal0);
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H265_GetRealBitStream(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits);
// Description :Read Barrel Shifter before shifting
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 u4ShiftBits : shift bits number
// Return      : Most significant (32 - u4ShiftBits) bits of barrel shifter input window before shifting
// **************************************************************************
UINT32 u4VDEC_HAL_H265_GetRealBitStream(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4ShiftBits)
{
    UINT32 u4RegVal0;

    u4RegVal0 = u4VDecHEVCVLDGetBitS(u4BSID, u4VDecID, 0);
    u4VDecHEVCVLDGetBitS(u4BSID, u4VDecID, u4ShiftBits);

    return (u4RegVal0 >> (32 - u4ShiftBits));
}


// **************************************************************************
// Function : UINT32 bVDEC_HAL_H265_GetBitStreamFlg(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Read Barrel Shifter before shifting
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : MSB of barrel shifter input window before shifting
// **************************************************************************
BOOL bVDEC_HAL_H265_GetBitStreamFlg(UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4RegVal;

    u4RegVal = u4VDEC_HAL_H265_GetBitStreamShift(u4BSID, u4VDecID, 1);
    return ((u4RegVal >> 31));
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H265_UeCodeNum(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Do UE variable length decoding
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : Input window after UE variable length decoding
// **************************************************************************
UINT32 u4VDEC_HAL_H265_UeCodeNum(UINT32 u4BSID, UINT32 u4VDecID)
{
    return (u4VDecReadHEVCVLD(u4VDecID, RO_HEVLD_UE));
}


// **************************************************************************
// Function : INT32 i4VDEC_HAL_H265_SeCodeNum(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Do SE variable length decoding
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : Input window after SE variable length decoding
// **************************************************************************
INT32 i4VDEC_HAL_H265_SeCodeNum(UINT32 u4BSID, UINT32 u4VDecID)
{

    return ((INT32)u4VDecReadHEVCVLD(u4VDecID, RO_HEVLD_SE));
}

// *********************************************************************
// Function    : UINT32 u4VDEC_HAL_H265_GetStartCode_PicStart(UINT32 u4BSID, UINT32 u4VDecID)
// Description : Get next start code
// Parameter   : u4BSID : Barrel shifter ID
//                   u4VDecID : VLD ID
// Return      : None
// *********************************************************************
UINT32 u4VDEC_HAL_H265_GetStartCode_PicStart(UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4Temp = 0;
    UINT32 u4Temp2 = 0;
    UINT32 u4NalType = 0;
    UINT32 u4ShiftBits = 0;
    UINT32 u4RetryNum = 0x100000;

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] u4VDEC_HAL_H265_GetStartCode_PicStart() start!!\n");
#endif

    UINT32 i;

    do
    {
        u4Temp = u4VDEC_HAL_H265_ShiftGetBitStream(u4BSID, u4VDecID, 0);

        if ((u4Temp >> 8 & 0x00ffffff) != START_CODE)  //HW search, stop at 3 bytes startcode
        {
            vVDecWriteHEVCVLD(u4VDecID, RW_HEVLD_CTRL, u4VDecReadHEVCVLD(u4BSID, RW_HEVLD_CTRL) | (HEVC_FIND_SC_CFG));
            u4VDecReadHEVCVLD(u4VDecID, RW_HEVLD_CTRL);
            vVDecWriteHEVCVLD(u4VDecID,  HEVC_FC_TRG_REG,  HEVC_SC_START);
#ifdef VDEC_SIM_DUMP
			pr_debug("wait(`HEVC_SC_START == 0);\n");
#endif
            for (i = 0; i < u4RetryNum; i++)
            {
                if ((u4VDecReadHEVCVLD(u4VDecID,  HEVC_FC_TRG_REG) & HEVC_SC_START) == 0)
                {
                    break;
                }
#ifdef VDEC_BW_FAKE_ENGINE_ON
                msleep(1);
                if (i % 10 == 0)
                {
					pr_debug("[INFO] VLD parsing bit count = 0x%08x, u4RetryNum: %d/%d\n", u4VDecReadHEVCVLD(u4VDecID, HEVC_BITCNT_REG), i, u4RetryNum);
                }
#endif
            }
        }
        u4Temp = u4VDEC_HAL_H265_ShiftGetBitStream(u4BSID, u4VDecID, 8);
        u4Temp = u4VDEC_HAL_H265_GetBitStreamShift(u4BSID, u4VDecID, 32);
        u4NalType = (((u4Temp & 0xffff) >> 9) & 0x3f);
        u4Temp2 =  u4VDEC_HAL_H265_ShiftGetBitStream(u4BSID, u4VDecID, 0);
    }
    while ((((u4Temp2 >> 31) & 0x01) == 0) && (u4NalType <= 21));    // until get "first_slice_segment_in_pic_flag" == 1

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] Intput Window GetStartCode  = 0x%08x\n", u4Temp);
	pr_debug("[INFO] u4VDEC_HAL_H265_GetStartCode_PicStart() Done!!\n");
#endif


    return u4Temp;
}


// *********************************************************************
// Function    : UINT32 u4VDEC_HAL_H265_GetStartCode(UINT32 u4BSID, UINT32 u4VDecID)
// Description : Get next start code
// Parameter   : u4BSID : Barrel shifter ID
//                   u4VDecID : VLD ID
// Return      : None
// *********************************************************************
UINT32 u4VDEC_HAL_H265_GetStartCode_8530(UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4Temp = 0;
    UINT32 u4ShiftBits = 0;
    UINT32 u4RetryNum = 0x100000;

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] u4VDEC_HAL_H265_GetStartCode_8530() start!!\n");
#endif

#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8530)
    UINT32 i;

    u4Temp = u4VDEC_HAL_H265_ShiftGetBitStream(u4BSID, u4VDecID, 0);

    if ((u4Temp >> 8 & 0x00ffffff) != START_CODE)
    {
        vVDecWriteHEVCVLD(u4VDecID,  RW_HEVLD_CTRL, u4VDecReadHEVCVLD(u4BSID, RW_HEVLD_CTRL) | (HEVC_FIND_SC_CFG));
        u4VDecReadHEVCVLD(u4VDecID, RW_HEVLD_CTRL);
        vVDecWriteHEVCVLD(u4VDecID,  HEVC_FC_TRG_REG,  HEVC_SC_START);
#ifdef VDEC_SIM_DUMP
		pr_debug("wait(`HEVC_SC_START == 0);\n");
#endif
        for (i = 0; i < u4RetryNum; i++)
        {
            if ((u4VDecReadHEVCVLD(u4VDecID,  HEVC_FC_TRG_REG) & HEVC_SC_START) == 0)
            {
                break;
            }
        }
    }

#endif
    u4Temp = u4VDEC_HAL_H265_ShiftGetBitStream(u4BSID, u4VDecID, 8);
    u4Temp = u4VDEC_HAL_H265_GetBitStreamShift(u4BSID, u4VDecID, 32);


#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] Intput Window GetStartCode  = 0x%08x\n", u4Temp);
	pr_debug("[INFO] u4VDEC_HAL_H265_GetStartCode_8530() Done!!\n");
#endif


    return u4Temp;
}


// **************************************************************************
// Function : INT32 i4VDEC_HAL_H265_InitBarrelShifter(UINT32 u4BSID, UINT32 u4VDecID, VDEC_INFO_H265_BS_INIT_PRM_T *prH265BSInitPrm);
// Description :Initialize barrel shifter with byte alignment
// Parameter :u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 prH265BSInitPrm : pointer to h265 initialize barrel shifter information struct
// Return      : =0: success.
//                  <0: fail.
// **************************************************************************
INT32 i4VDEC_HAL_H265_InitBarrelShifter(UINT32 u4BSID, UINT32 u4VDecID, VDEC_INFO_H265_BS_INIT_PRM_T *prH265BSInitPrm)
{
    BOOL fgInitBSResult;

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] i4VDEC_HAL_H265_InitBarrelShifter() start!!\n");
#endif

    fgInitBSResult = fgInitH265BarrelShift(u4VDecID, prH265BSInitPrm);

    if (fgInitBSResult)
    {
#ifdef VDEC_SIM_DUMP
		pr_debug("[INFO] i4VDEC_HAL_H265_InitBarrelShifter() Done!!\n");
#endif
        return HAL_HANDLE_OK;
    }
    else
    {
		pr_debug("\n[ERROR] i4VDEC_HAL_H265_InitBarrelShifter() Fail!!!!!!!!!!!!!!\n\n");
        return INIT_BARRELSHIFTER_FAIL;
    }
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H265_ReadRdPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 *pu4Bits);
// Description :Read current read pointer
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 pu4Bits : read pointer value with remained bits
// Return      : Read pointer value with byte alignment
// **************************************************************************
UINT32 u4VDEC_HAL_H265_ReadRdPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4VFIFOSa, UINT32 *pu4Bits)
{
    UINT32 retval;
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] u4VDEC_HAL_H265_ReadRdPtr() start!!\n");
#endif
    retval = u4VDecReadH265VldRPtr(u4BSID, u4VDecID, pu4Bits, PHYSICAL(u4VFIFOSa));
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] u4VDEC_HAL_H265_ReadRdPtr() done!!\n");
#endif

    return retval;
}


// **************************************************************************
// Function : void v4VDEC_HAL_H265_AlignRdPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4AlignType);
// Description :Align read pointer to byte,word or double word
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
//                 u4AlignType : read pointer align type
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_AlignRdPtr(UINT32 u4BSID, UINT32 u4VDecID, UINT32 u4AlignType)
{
    return;
}


// **************************************************************************
// Function : UINT32 u4VDEC_HAL_H265_GetBitcount(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Read barrel shifter bitcount after initializing
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : Current bit count
// **************************************************************************
UINT32 u4VDEC_HAL_H265_GetBitcount(UINT32 u4BSID, UINT32 u4VDecID)
{
    return HAL_HANDLE_OK;
}


// **************************************************************************
// Function : void vVDEC_HAL_H265_Modification(UINT32 u4VDecID);
// Description :Reference list reordering
// Parameter : u4VDecID : video decoder hardware ID
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_RPL_Modification(UINT32 u4VDecID)
{
    UINT32 u4Cnt;
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_RPL_Modification()\n");
	pr_debug("wait(`HEVC_RPL_MOD == 0);\n");
#endif

    vVDecWriteHEVCVLD(u4VDecID, HEVC_FC_TRG_REG, HEVC_RPL_MOD);
    u4Cnt = 0;
    while (1)
    {
        if (u4Cnt == 100)
        {
            if (0 == (u4VDecReadHEVCVLD(u4VDecID, HEVC_FC_TRG_REG) & HEVC_RPL_MOD))
            {
                break;
            }
            else
            {
                u4Cnt = 0;
            }
        }
        else
        {
            u4Cnt ++;
        }
    }
}

// **************************************************************************
// Function : void vVDEC_HAL_H265_PredWeightTable(UINT32 u4VDecID);
// Description :Decode prediction weighting table
// Parameter : u4VDecID : video decoder hardware ID
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_PredWeightTable(UINT32 u4VDecID)
{

    UINT32 u4Cnt;
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_PredWeightTable()\n");
	pr_debug("wait(`HEVC_WPT == 0);\n");
#endif

    vVDecWriteHEVCVLD(u4VDecID, HEVC_FC_TRG_REG, HEVC_WEIGHT_PRED_TBL);
    u4Cnt = 0;
    while (1)
    {
        if (u4Cnt == 100)
        {
            if (0 == (u4VDecReadHEVCVLD(u4VDecID, HEVC_FC_TRG_REG) & HEVC_WEIGHT_PRED_TBL))
            {
                break;
            }
            else
            {
                u4Cnt = 0;
            }
        }
        else
        {
            u4Cnt ++;
        }
    }

}


// **************************************************************************
// Function : void vVDEC_HAL_H265_TrailingBits(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Remove traling bits to byte align
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_TrailingBits(UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4Temp;

    u4Temp = 8 - (u4VDecHEVCVLDShiftBits(u4BSID, u4VDecID) % 8);
    // at list trailing bit

    if (u4Temp == 8)
    {
        u4Temp = u4VDecHEVCVLDGetBitS(u4BSID, u4VDecID, 8);
    }
    else
    {
        u4Temp = u4VDecHEVCVLDGetBitS(u4BSID, u4VDecID, u4Temp);
    }

}


// **************************************************************************
// Function : BOOL bVDEC_HAL_H265_IsMoreRbspData(UINT32 u4BSID, UINT32 u4VDecID);
// Description :Check whether there is more rbsp data
// Parameter : u4BSID  : barrelshifter ID
//                 u4VDecID : video decoder hardware ID
// Return      : Is morw Rbsp data or not
// **************************************************************************
BOOL bVDEC_HAL_H265_IsMoreRbspData(UINT32 u4BSID, UINT32 u4VDecID)
{
    UINT32 u4RemainedBits;
    UINT32 u4Temp;
    INT32 i;

    u4RemainedBits = (u4VDecHEVCVLDShiftBits(u4BSID, u4VDecID) % 8); //0~7
    //u4RemainedBits = (8 - (((u4VDecReadHEVCVLD(RW_HEVLD_CTRL) >> 16) & 0x3F) % 8));
    u4Temp = 0xffffffff;
    for (i = 0; i <= u4RemainedBits; i++)
    {
        u4Temp &= (~(1 << i));
    }

    if ((u4VDecHEVCVLDGetBitS(u4BSID, u4VDecID, 0) & u4Temp) == (0x80000000))
    {
        // no more
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


// **************************************************************************
// Function : void vVDEC_HAL_H265_SetPicInfoReg(UINT32 u4VDecID);
// Description :Set HW registers to initialize picture info
// Parameter : u4VDecID : video decoder hardware ID
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_SetPicInfoReg(UINT32 u4VDecID)
{
    UINT32 dram_pitch_width, log2_max_cu_size, max_cu_size;
    UINT32 pic_width, pic_height;
    UINT32 u4RetVal;
    H265_SPS_Data *prSPS;
    H265_PPS_Data *prPPS;

    prSPS = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.prSPS;
    prPPS = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.prPPS;
    pic_width = prSPS->u4PicWidthInLumaSamples;
    pic_height = prSPS->u4PicHeightInLumaSamples;

    log2_max_cu_size =  prSPS->u4Log2DiffMaxMinCodingBlockSize + prSPS->u4Log2MinCodingBlockSizeMinus3 + 3;
    max_cu_size = 1 << log2_max_cu_size;

    dram_pitch_width = (((pic_width + (max_cu_size - 1)) >> log2_max_cu_size) << log2_max_cu_size) / 16;
    //dram_pitch_width = (((pic_width+(64-1))>>6 )<<6)/16;

    // MC part
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] MC settings\n");
#endif

    u4RetVal = u4VDecReadMC(u4VDecID, 702 * 4);      // performance setting
    vVDecWriteMC(u4VDecID, 702 * 4, u4RetVal | (0x1 << 8));
    vVDecWriteMC(u4VDecID, HEVC_PIC_WIDTH, pic_width);
    vVDecWriteMC(u4VDecID, HEVC_PIC_HEIGHT, pic_height);
    vVDecWriteMC(u4VDecID, HEVC_DRAM_PITCH, dram_pitch_width);
    vVDecWriteMC(u4VDecID, HEVC_CBCR_DPB_OFFSET, _ptH265CurrFBufInfo[u4VDecID]->u4CAddrOffset);

    int pic_mb_x = ((pic_width  + max_cu_size - 1) / max_cu_size) * max_cu_size / 16;
    int pic_mb_y = ((pic_height + max_cu_size - 1) / max_cu_size) * max_cu_size / 16;

    //PP part
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] Current Buffer index: %d \n", _tVerMpvDecPrm[u4VDecID].ucDecFBufIdx);
	pr_debug("[INFO] PP settings\n");
#endif

    vVDecWriteMC(u4VDecID, HEVC_DBK_ON, 0x1);

    //@ for UFO
    if (_tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
    {
        vVDecWriteMC(u4VDecID, HEVC_Y_OUTPUT, (PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr) >> 9) & 0x7fffff);
        vVDecWriteMC(u4VDecID, HEVC_C_OUTPUT, (PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr + _ptH265CurrFBufInfo[u4VDecID]->u4CAddrOffset) >> 8) & 0xffffff);
    }
    else
    {
        vVDecWriteMC(u4VDecID, HEVC_Y_OUTPUT, (PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr) >> 9) & 0x7fffff);
        vVDecWriteMC(u4VDecID, HEVC_C_OUTPUT, (PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr + _ptH265CurrFBufInfo[u4VDecID]->u4DramPicSize) >> 8) & 0xffffff);
    }
    vVDecWriteMC(u4VDecID, HEVC_OUTPUT_WIDTH, pic_mb_x);

    vVDecWriteMC(u4VDecID, HEVC_DBK_ON2, ((prSPS->u4ChromaFormatIdc == 0) ? 2 : 3));
    vVDecWriteMC(u4VDecID, HEVC_ENABLE_WR_REC, 0x1);
    vVDecWriteMC(u4VDecID, HEVC_PIC_WITDTH_MB, pic_mb_x - 1);
    vVDecWriteMC(u4VDecID, HEVC_PIC_HEIGHT_MB, pic_mb_y - 1);


    if (prSPS->u4BitDepthLumaMinus8 > 0 || prSPS->u4BitDepthChromaMinus8 > 0)
    {
#ifdef VDEC_SIM_DUMP
		pr_debug("[INFO] 10bit settings \n");
#endif
        _ptH265CurrFBufInfo[u4VDecID]->bIsMain10 = 1;
        vVDecWriteMC(u4VDecID, 827 * 4, 0x1);       // compact mode MC settings
        vVDecWriteHEVCPP(u4VDecID, 739 * 4, 0x1); // compact mode MC settings
        vVDecWriteHEVCPP(u4VDecID, 740 * 4, 0x1);  // 10bits ext_bit_col_mode
    }

#ifdef HEVC_DDR4
    //PP output 32bits (DDR4 setting) align
    //disable: make CRC comparable
	pr_debug("[INFO] HEVC DDR4 settings ON, CRC not comparable \n");
    if (_tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode)
    {
        u4RetVal = u4VDecReadHEVCPP(u4VDecID, 814 * 4);
        vVDecWriteHEVCPP(u4VDecID, 814 * 4, u4RetVal | 0x1);
    }
    else
    {
        u4RetVal = u4VDecReadHEVCPP(u4VDecID, 744 * 4);
        vVDecWriteHEVCPP(u4VDecID, 744 * 4, u4RetVal | 0x1);
    }
#endif

}


// **************************************************************************
// Function : void vVDEC_HAL_H265_SetRefPicListReg(UINT32 u4VDecID);
// Description :Set HW registers related with P reference list
// Parameter : u4VDecID : video decoder hardware ID
//                 prPRefPicListInfo : pointer to information of p reference list
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_SetRefPicListReg(UINT32 u4VDecID)
{
    UINT32 addr;
    UINT32 i, pic_width, pic_height;
    unsigned int value;
    BOOL bIsUFO;
    H265_SPS_Data *prSPS;

    bIsUFO = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode;
    prSPS = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.prSPS;

    pic_width = prSPS->u4PicWidthInLumaSamples;
    pic_height = prSPS->u4PicHeightInLumaSamples;

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] UFO settings\n");
#endif
    // UFO mode settings
    if (bIsUFO)
    {

        vVDecWriteMC(u4VDecID, 698 * 4, PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YLenStartAddr));
        vVDecWriteMC(u4VDecID, 699 * 4, PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4CLenStartAddr));

        // Eve_rest UFO update
        vVDecWriteMC(u4VDecID, 825 * 4, _ptH265CurrFBufInfo[u4VDecID]->u4PicSizeYBS);
        vVDecWriteMC(u4VDecID, 826 * 4, _ptH265CurrFBufInfo[u4VDecID]->u4PicSizeCBS);

        //vVDecWriteHEVCPP(u4VDecID, 706*4, 0x1 );   // UFO garbage remove
        vVDecWriteHEVCPP(u4VDecID, 803 * 4, 0x1);   // UFO error handling no hang
        vVDecWriteMC(u4VDecID, 664 * 4, 0x11);

    }
    else
    {
        vVDecWriteMC(u4VDecID, 664 * 4, 0x0);
    }

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] EC settings\n");
#endif
    // Error Concealment settings
    UCHAR ucRefFBIndex;
    UINT32 minDPOC;

    //search min delta POC pic in DPB
    minDPOC = MAX_INT;
    for (i = 0; i < _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.ucMaxFBufNum; i++)
    {
        if (i == _tVerMpvDecPrm[u4VDecID].ucDecFBufIdx)
        {
            continue;
        }
        if (abs(_ptH265CurrFBufInfo[u4VDecID]->i4POC - _ptH265FBufInfo[u4VDecID][i].i4POC) < minDPOC)
        {
            minDPOC = abs(_ptH265CurrFBufInfo[u4VDecID]->i4POC - _ptH265FBufInfo[u4VDecID][i].i4POC);
            ucRefFBIndex = i;
        }
    }
    if (minDPOC == MAX_INT)
    {
        ucRefFBIndex = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.ucPreFBIndex;
    }

    for (i = 0; i < 16; i++)
    {
        vVDecWriteMC(u4VDecID, HEVC_MC_REF_B + i * 4,  PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4YStartAddr));
        vVDecWriteMC(u4VDecID, RW_MC_B_LIST1 + i * 4,  PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4YStartAddr));
        vVDecWriteMC(u4VDecID, (760 + i) * 4,  PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4YStartAddr + _ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4CAddrOffset));
        vVDecWriteMC(u4VDecID, (792 + i) * 4,  PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4YStartAddr + _ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4CAddrOffset));
        vVDecWriteHEVCMV(u4VDecID, i * 4, PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4MvStartAddr) >> 4);
        vVDecWriteHEVCMV(u4VDecID, (16 + i) * 4, PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4MvStartAddr) >> 4);
    }

    vVDecWriteHEVCVLD(u4VDecID, HEVC_VLD_ERROR_TYPE_ENABLE, 0xfffefffb);
    //vVDecWriteHEVCVLD(u4VDecID, HEVC_VLD_PICTURE_BYTES, total_bytes_in_curr_pic);
    vVDecWriteHEVCVLD(u4VDecID, HEVC_VLD_ERROR_HANDLING, 0x04011d01);   // 06172013, turn on slice_reconceal_sel

    // PP Write all mode Change
    if (_tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.prPPS->bTilesEnabledFlag
        && !_tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode
        && bit_true_compare)   //No use CRC -> bit_true_compare = true
    {
		pr_debug("HEVC_PP_ERROR_HANDLE_MODE case 1 (PP 0x80004011) \n");
        vVDecWriteHEVCPP(u4VDecID, HEVC_PP_ERROR_HANDLE_MODE, 0x80004011);
    }
    else if (_tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.prPPS->bTilesEnabledFlag)
    {
		pr_debug("HEVC_PP_ERROR_HANDLE_MODE case 2 (PP 0x00004011) \n");
        vVDecWriteHEVCPP(u4VDecID, HEVC_PP_ERROR_HANDLE_MODE, 0x00004011);
    }
    else
    {
		pr_debug("HEVC_PP_ERROR_HANDLE_MODE case 3 (PP 0x00007011) \n");
        vVDecWriteHEVCPP(u4VDecID, HEVC_PP_ERROR_HANDLE_MODE, 0x00007011);
    }
    //~

    // 20131227 CM Hung if reference frame exist, MV setting for EC not alway use List_o[0]
    minDPOC = MAX_INT;
    for (i = 0; i < _rH265PicInfo[u4VDecID].i4RefListNum; i++)
    {
        if (abs(_rH265PicInfo[u4VDecID].i4PocDiffList0[i]) < minDPOC)
        {
            minDPOC = abs(_rH265PicInfo[u4VDecID].i4PocDiffList0[i]);
            value = i;
        }
    }
    vVDecWriteHEVCMV(u4VDecID, 132 * 4, value << 4);

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] MV settings\n");
#endif

    value   =  _rH265PicInfo[u4VDecID].bLowDelayFlag;
    vVDecWriteHEVCMV(u4VDecID, HEVC_MV_CTRL, value);

    value   = 0;
    if (_rH265PicInfo[u4VDecID].i4RefListNum > 0)
    {
        for (i = 0; i < 16; i++)
        {
            int idx;
            idx = (i % _rH265PicInfo[u4VDecID].i4RefListNum);
            value = value | (_rH265PicInfo[u4VDecID].i4LongTermList0[idx] << i) | (_rH265PicInfo[u4VDecID].i4LongTermList1[idx] << (i + 16));
        }
        vVDecWriteHEVCMV(u4VDecID, HEVC_LONG_TERM, value);
    }

    value = 0;
    for (i = 0; i < _rH265PicInfo[u4VDecID].i4DpbLTBuffCnt; i++)
    {
        if (i % 4 == 0)
        {
            value = 0;
        }
        value |= ((_rH265PicInfo[u4VDecID].i4DpbLTBuffId[i] + 1) & 0x1f) << (i * 8);
        if (i % 4 == 3)
        {
            vVDecWriteHEVCMV(u4VDecID, HEVC_DPB_LT_BUF_ID_0_3 + ((i >> 2) << 2), value);
        }
    }

    if (i % 4 != 0)
    {
        vVDecWriteHEVCMV(u4VDecID, HEVC_DPB_LT_BUF_ID_0_3 + ((i >> 2) << 2), value);
    }

    if (_rH265PicInfo[u4VDecID].i4RefListNum > 0)
    {

        for (i = 0; i < 16; i = i + 1)
        {
            int idx;
            idx = (i % _rH265PicInfo[u4VDecID].i4RefListNum);
            addr = HEVC_L0_INFO_0 + i * 4;
            value = ((_rH265PicInfo[u4VDecID].i4PocDiffList0[idx] & 0xff) << 0) |
                    (((_rH265PicInfo[u4VDecID].i4BuffIdList0[idx] + 1) & 0x1f) << 8) ;
            vVDecWriteHEVCMV(u4VDecID, addr , value);
        }

        for (i = 0; i < 16; i = i + 1)
        {
            int idx;
            idx = (i % _rH265PicInfo[u4VDecID].i4RefListNum);
            addr      = HEVC_L1_INFO_0 + i * 4;
            value = ((_rH265PicInfo[u4VDecID].i4PocDiffList1[idx] & 0xff) << 0) |
                    (((_rH265PicInfo[u4VDecID].i4BuffIdList1[idx] + 1) & 0x1f) << 8) ;
            vVDecWriteHEVCMV(u4VDecID, addr , value);
        }
    }

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] Current Buffer index: %d \n", _tVerMpvDecPrm[u4VDecID].ucDecFBufIdx);
#endif
    vVDecWriteHEVCMV(u4VDecID, HEVC_MV_WR_SA , (PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4MvStartAddr) >> 4) & 0xfffffff);
    vVDecWriteMC(u4VDecID, 727 * 4, 0x1);   // Turn on Y.C ref buffer separate setting

    if (_rH265PicInfo[u4VDecID].i4RefListNum > 0)
    {
        for (i = 0; i < 16; i++)
        {
            int idx;
            idx = (i % _rH265PicInfo[u4VDecID].i4RefListNum);
            vVDecWriteHEVCMV(u4VDecID, i * 4 , (PHYSICAL(_ptH265FBufInfo[u4VDecID][_rH265PicInfo[u4VDecID].i4BuffIdList0[idx]].u4MvStartAddr) >> 4) & 0xfffffff);
            vVDecWriteMC(u4VDecID, (i + 279) * 4 , PHYSICAL(_ptH265FBufInfo[u4VDecID][_rH265PicInfo[u4VDecID].i4BuffIdList0[idx]].u4YStartAddr));
            vVDecWriteMC(u4VDecID, (i + 760) * 4 , PHYSICAL(_ptH265FBufInfo[u4VDecID][_rH265PicInfo[u4VDecID].i4BuffIdList0[idx]].u4YStartAddr
                                                            + _ptH265FBufInfo[u4VDecID][_rH265PicInfo[u4VDecID].i4BuffIdList0[idx]].u4CAddrOffset));
            /* RO_ME TILE not support UFO settings
            if (bIsUFO){
                _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.u4RefUFOEncoded &= (~( 0x1<<i ));
                _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.u4RefUFOEncoded |= (_ptH265FBufInfo[u4VDecID][_rH265PicInfo[u4VDecID].i4BuffIdList0[idx]].bIsUFOEncoded << i );
            }
            */
        }
        for (i = 0; i < 16; i++)
        {
            int idx;
            idx = (i % _rH265PicInfo[u4VDecID].i4RefListNum);
            vVDecWriteHEVCMV(u4VDecID, (i + 16) * 4 , (PHYSICAL(_ptH265FBufInfo[u4VDecID][ _rH265PicInfo[u4VDecID].i4BuffIdList1[idx]].u4MvStartAddr) >> 4) & 0xfffffff);
            vVDecWriteMC(u4VDecID, (i + 311) * 4 , PHYSICAL(_ptH265FBufInfo[u4VDecID][ _rH265PicInfo[u4VDecID].i4BuffIdList1[idx]].u4YStartAddr));
            vVDecWriteMC(u4VDecID, (i + 792) * 4 , PHYSICAL(_ptH265FBufInfo[u4VDecID][_rH265PicInfo[u4VDecID].i4BuffIdList1[idx]].u4YStartAddr
                                                            + _ptH265FBufInfo[u4VDecID][_rH265PicInfo[u4VDecID].i4BuffIdList1[idx]].u4CAddrOffset));
            /* RO_ME TILE not support UFO settings
            if (bIsUFO){
                _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.u4RefUFOEncoded &= (~( 0x1<<(i+16) ));
                _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.u4RefUFOEncoded |= (_ptH265FBufInfo[u4VDecID][ _rH265PicInfo[u4VDecID].i4BuffIdList1[idx]].bIsUFOEncoded << (i+16) );
            }
            */
        }
    }


#ifdef VDEC_BW_FAKE_ENGINE_ON
    // Fake engine on HEVC LCU internal timeout disable
    UINT32 u4RetVal;
    u4RetVal = u4VDecReadHEVCVLD(u4VDecID, HEVC_VLD_ERROR_TYPE_ENABLE);
    vVDecWriteHEVCVLD(u4VDecID, HEVC_VLD_ERROR_TYPE_ENABLE, (u4RetVal & (~(0x3 << 28))));
#endif

}


// **************************************************************************
// Function : void vVDEC_HAL_H265_SetSPSHEVLD(UINT32 u4VDecID, H265_SPS_Data *prSPS);
// Description :Set Slice data to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 prPPS
//                 prSPS : pointer to sequence parameter set struct
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_SetSPSHEVLD(UINT32 u4VDecID, H265_SPS_Data *prSPS, H265_PPS_Data *prPPS)
{
    int max_cu_width, max_cu_heigtht;
    int pic_width, pic_height;
    int lcu_pic_width, lcu_pic_height;
    int log2_lt_ref_pic_sps = 1;
    int log2_short_ref_pic_idx = 1;
    int log2_max_cu_size;
    int log2_max_tu_size;

    UINT32 u4SPSInfo;
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetSPSHEVLD() start!!\n");
#endif

    log2_max_cu_size =  prSPS->u4Log2DiffMaxMinCodingBlockSize + prSPS->u4Log2MinCodingBlockSizeMinus3 + 3;
    log2_max_tu_size = prSPS->u4Log2DiffMaxMinTtransformBlockSize + prSPS->u4Log2MinTransformBlockSizeMinus2 + 2;

    max_cu_width = 1 << log2_max_cu_size;
    max_cu_heigtht = 1 << log2_max_cu_size;

    pic_width = prSPS->u4PicWidthInLumaSamples;
    pic_height = prSPS->u4PicHeightInLumaSamples;
    pic_width = ((pic_width + max_cu_width - 1) >> log2_max_cu_size) << log2_max_cu_size;
    pic_height = ((pic_height + max_cu_width - 1) >> log2_max_cu_size) << log2_max_cu_size;

    lcu_pic_width    = (pic_width % max_cu_width) ? pic_width / max_cu_width  + 1 : pic_width / max_cu_width;
    lcu_pic_height   = (pic_height % max_cu_heigtht) ? pic_height / max_cu_heigtht + 1 : pic_height / max_cu_heigtht;

    _rH265PicInfo[u4VDecID].u4PicWidthInCU = lcu_pic_width;
    _rH265PicInfo[u4VDecID].u4PicHeightInCU = lcu_pic_height;

    vVDecWriteVLDTOP(u4VDecID, RW_VLD_PIC_MB_SIZE_M1, (((lcu_pic_width  - 1) & 0xfff) << 0) | (((lcu_pic_height - 1) & 0xfff) << 16));
    vVDecWriteVLDTOP(u4VDecID, HEVC_VLD_TOP_PIC_PIX_SIZE, (((prSPS->u4PicWidthInLumaSamples) & 0xffff) << 0) | (((prSPS->u4PicHeightInLumaSamples) & 0xffff) << 16));
    vVDecWriteVLDTOP(u4VDecID, HEVC_VLD_TOP_PIC_BLK_SIZE, (((lcu_pic_width  - 0) & 0xfff) << 0) | (((lcu_pic_height - 0) & 0xfff) << 16));
    vVDecWriteVLDTOP(u4VDecID, HEVC_VLD_TOP_BIT_DEPTH, ((prSPS->u4BitDepthLumaMinus8 & 0xf) << 0) | ((prSPS->u4BitDepthChromaMinus8 & 0xf) << 4));

    u4SPSInfo = 0;
    u4SPSInfo = (prSPS->u4ChromaFormatIdc & 0x3);
    u4SPSInfo |= ((prSPS->bUsePCM & 0x1) << 2);
    u4SPSInfo |= ((prPPS->bListsModificationPresentFlag & 0x1) << 3);
    u4SPSInfo |= ((prSPS->bUseAMP & 0x1) << 4);
    u4SPSInfo |= ((prSPS->bUseSAO & 0x1) << 5);
    u4SPSInfo |= ((prSPS->bPCMFilterDisableFlag & 0x1) << 6);
    u4SPSInfo |= ((prSPS->bTMVPFlagsPresent & 0x1) << 7);
    u4SPSInfo |= ((prSPS->u4PCMBitDepthLumaMinus1 & 0xf) << 8);
    u4SPSInfo |= ((prSPS->u4PCMBitDepthChromaMinus1 & 0xf) << 12);
    u4SPSInfo |= ((prSPS->u4NumLongTermRefPicSPS & 0x3f) << 16);
    u4SPSInfo |= ((prSPS->bLongTermRefsPresent & 0x1) << 22);
    u4SPSInfo |= ((prSPS->u4NumShortTermRefPicSets & 0x7f) << 24);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVLD_SPS_0, u4SPSInfo);

	/*pr_debug("1-------prSPS->u4NumLongTermRefPicSPS %d\n",prSPS->u4NumLongTermRefPicSPS);*/
    while ((prSPS->u4NumLongTermRefPicSPS & 0x3f) > (1 << log2_lt_ref_pic_sps))
    {
		/*pr_debug("looping-------log2_lt_ref_pic_sps %d\n",log2_lt_ref_pic_sps);*/
        log2_lt_ref_pic_sps++;
    }
	/*pr_debug("2-------prSPS->u4NumShortTermRefPicSets %d\n",prSPS->u4NumShortTermRefPicSets);*/
    while ((prSPS->u4NumShortTermRefPicSets & 0x7f) > (1 << log2_short_ref_pic_idx))
    {
		/*pr_debug("looping-------log2_short_ref_pic_idx %d\n",log2_short_ref_pic_idx);*/
        log2_short_ref_pic_idx++;
    }
	/*pr_debug("3-------log2_lt_ref_pic_sps %d; log2_short_ref_pic_idx %d\n", log2_lt_ref_pic_sps, log2_short_ref_pic_idx);*/

    u4SPSInfo = 0;
    u4SPSInfo = ((prSPS->u4Log2MaxPicOrderCntLsbMinus4 + 4) & 0x1f);
    u4SPSInfo |= ((log2_short_ref_pic_idx & 0x7) << 8);
    u4SPSInfo |= ((log2_lt_ref_pic_sps & 0x7) << 12);
    u4SPSInfo |= ((prSPS->bUseStrongIntraSmoothing & 0x1) << 16);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVLD_SPS_1, u4SPSInfo);


    u4SPSInfo = 0;
    u4SPSInfo = ((prSPS->u4Log2MinCodingBlockSizeMinus3 + 3) & 0x7);
    u4SPSInfo |= ((log2_max_cu_size & 0x7) << 4);
    u4SPSInfo |= (((prSPS->u4Log2MinTransformBlockSizeMinus2 + 2) & 0x7) << 8);
    u4SPSInfo |= ((log2_max_tu_size & 0x7) << 12);
    u4SPSInfo |= (((prSPS->u4PCMLog2LumaMinSizeMinus3 + 3) & 0x7) << 16);
    u4SPSInfo |= (((prSPS->u4PCMLog2LumaMaxSize) & 0x7) << 20);
    u4SPSInfo |= (((prSPS->u4QuadtreeTUMaxDepthInter - 1) & 0x7) << 24);
    u4SPSInfo |= (((prSPS->u4QuadtreeTUMaxDepthIntra - 1) & 0x7) << 28);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVLD_SPS_SIZE, u4SPSInfo);
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetSPSHEVLD() done!!\n");
#endif

}


// **************************************************************************
// Function : void vVDEC_HAL_H265_SetPPSHEVLD(UINT32 u4VDecID, H265_SPS_Data *prSPS, H265_PPS_Data *prPPS)
// Description :Set PPS data to HW
// Parameter : u4VDecID : video decoder hardware ID
//                   prSPS
//                   prPPS : pointer to picture parameter set struct
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_SetPPSHEVLD(UINT32 u4VDecID, H265_SPS_Data *prSPS, H265_PPS_Data *prPPS)
{
    UINT32 u4PPSInfo;
    INT32 i;
    int log2_min_cu_qp_delta_size;
    int log2_max_cu_size;
    BOOL bIsUFO;
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetPPSHEVLD() start!!\n");
#endif

    bIsUFO = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode;
    log2_max_cu_size =  prSPS->u4Log2DiffMaxMinCodingBlockSize + prSPS->u4Log2MinCodingBlockSizeMinus3 + 3;

    u4PPSInfo = 0;
    u4PPSInfo = (prPPS->bSignHideFlag & 0x1);
    u4PPSInfo |= ((prPPS->bCabacInitPresentFlag & 0x1) << 1);
    u4PPSInfo |= ((prPPS->bConstrainedIntraPredFlag & 0x1) << 2);
    u4PPSInfo |= ((prPPS->bTransformSkipEnabledFlag & 0x1) << 3);
    u4PPSInfo |= ((prPPS->bWPPredFlag & 0x1) << 4);
    u4PPSInfo |= ((prPPS->bWPBiPredFlag & 0x1) << 5);
    u4PPSInfo |= ((prPPS->bOutputFlagPresentFlag & 0x1) << 6);
    u4PPSInfo |= ((prPPS->bTransquantBypassEnableFlag & 0x1) << 7);
    u4PPSInfo |= ((prPPS->bDependentSliceSegmentsEnabledFlag & 0x1) << 8);
    u4PPSInfo |= ((prPPS->bEntropyCodingSyncEnabledFlag & 0x1) << 9);
    u4PPSInfo |= ((prPPS->bSliceHeaderExtensionPresentFlag & 0x1) << 11);
    u4PPSInfo |= ((prPPS->u4NumRefIdxL0DefaultActiveMinus1 & 0xf) << 16);
    u4PPSInfo |= ((prPPS->u4NumRefIdxL1DefaultActiveMinus1 & 0xf) << 20);
    u4PPSInfo |= (((prPPS->u4Log2ParallelMergeLevelMinus2 + 2) & 0x7) << 24);
    u4PPSInfo |= ((prPPS->u4NumExtraSliceHeaderBits & 0x7) << 28);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVLD_PPS, u4PPSInfo);

    log2_min_cu_qp_delta_size = log2_max_cu_size - (prPPS->u4DiffCuQPDeltaDepth);

    u4PPSInfo = 0;
    u4PPSInfo = (prPPS->bCuQPDeltaEnabledFlag & 0x1);
    u4PPSInfo |= ((prPPS->bPPSSliceChromaQpFlag & 0x1) << 1);
    u4PPSInfo |= ((log2_min_cu_qp_delta_size & 0x7) << 4);
    u4PPSInfo |= (((prPPS->i4PicInitQPMinus26 + 26) & 0x7f) << 8);
    u4PPSInfo |= ((prPPS->i4PPSCbQPOffset & 0x1f) << 16);
    u4PPSInfo |= ((prPPS->i4PPSCrQPOffset & 0x1f) << 24);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVLD_PPS_QP, u4PPSInfo);

    u4PPSInfo = 0;
    u4PPSInfo = (prPPS->bTilesEnabledFlag & 0x1);
    u4PPSInfo |= ((prPPS->u4NumColumnsMinus1 & 0x1f) << 8);
    u4PPSInfo |= ((prPPS->u4NumRowsMinus1 & 0x1f) << 16);
    u4PPSInfo |= ((prPPS->bLoopFilterAcrossTilesEnabledFlag & 0x1) << 24);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVLD_PPS_TILE , u4PPSInfo);

    u4PPSInfo = 0;
    u4PPSInfo = (prPPS->bLoopFilterAcrossSlicesEnabledFlag & 0x1);
    u4PPSInfo |= ((prPPS->bDeblockingFilterControlPresentFlag & 0x1) << 1);
    u4PPSInfo |= ((prPPS->bDeblockingFilterOverrideEnabledFlag & 0x1) << 2);
    u4PPSInfo |= ((prPPS->bPicDisableDeblockingFilterFlag & 0x1) << 3);
    u4PPSInfo |= ((prPPS->i4DeblockingFilterBetaOffsetDiv2 & 0x1f) << 4);    //[notice] initial value need to be check
    u4PPSInfo |= ((prPPS->i4DeblockingFilterTcOffsetDiv2 & 0x1f) << 9);     //

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVLD_PPS_DBK , u4PPSInfo);

    if (bIsUFO)
    {
        _ptH265CurrFBufInfo[u4VDecID]->bIsUFOEncoded = 1;
    }
    else
    {
        _ptH265CurrFBufInfo[u4VDecID]->bIsUFOEncoded = 0;
    }

    /* RO_ME TILE not support UFO settings
        if (bIsUFO){
            BOOL bUFOEnc;
            BOOL bUFODec;
            UINT32 u4RefUFOEncoded;
            int i;

            if ( prPPS->bTilesEnabledFlag )
            {
                _ptH265CurrFBufInfo[u4VDecID]->bIsUFOEncoded = 0;
                bUFOEnc = 0;
            }
            else
            {
                _ptH265CurrFBufInfo[u4VDecID]->bIsUFOEncoded = 1;
                bUFOEnc = 1;
            }

            u4RefUFOEncoded = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.u4RefUFOEncoded;
            if (u4RefUFOEncoded==0)
            {
                bUFODec = 0;
                vVDecWriteMC(u4VDecID, 664*4, (bUFODec<<4)|bUFOEnc);
            }
            else
            {
                bUFODec = 1;
                vVDecWriteMC(u4VDecID, 664*4, (bUFODec<<4)|bUFOEnc);
                vVDecWriteMC(u4VDecID, 722*4, 0x1);
                vVDecWriteMC(u4VDecID, 718*4, u4RefUFOEncoded);
                vVDecWriteMC(u4VDecID, 719*4, u4RefUFOEncoded);
                vVDecWriteMC(u4VDecID, 720*4, u4RefUFOEncoded);
            }

            #ifdef VDEC_SIM_DUMP
				pr_debug ("[INFO] UFO Mode !! Encoder: %d Decoder: %d\n", bUFOEnc, bUFODec);
            #endif
        }
    */

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetPPSHEVLD() done!!\n");
#endif

}


// **************************************************************************
// Function :void vVDEC_HAL_H265_SetSHDRHEVLD(UINT32 u4VDecID, H265_Slice_Hdr_Data *prSliceHdr , BOOL bUseSAO, H265_PPS_Data *prPPS)
// Description :Set part of slice header data to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 prSliceHdr : pointer to slice parameter set struct
//                 bUseSAO, prPPS
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_SetSHDRHEVLD(UINT32 u4VDecID, H265_Slice_Hdr_Data *prSliceHdr, BOOL bUseSAO, H265_PPS_Data *prPPS)
{
    UINT32 u4SHDRInfo;
    int i4Indexer;
    int i4NumRpsCurrTempList = 0;
    int numRpsCurrTempList0 = 0;
    int length = 0;
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetSHDRHEVLD() start!!\n");
#endif

    u4SHDRInfo = 0;
    u4SHDRInfo = ((bUseSAO & prSliceHdr->bSaoEnabledFlag & 0x1) << 5);
    u4SHDRInfo |= ((bUseSAO & prSliceHdr->bSaoEnabledFlagChroma & 0x1) << 6);
    u4SHDRInfo |= ((prSliceHdr->bCabacInitFlag & 0x1) << 7);
    u4SHDRInfo |= ((prSliceHdr->i4SliceQp & 0x7f) << 8);
    u4SHDRInfo |= (((prPPS->i4PPSCbQPOffset + prSliceHdr->i4SliceQpDeltaCb) & 0x1f) << 16);
    u4SHDRInfo |= (((prPPS->i4PPSCrQPOffset + prSliceHdr->i4SliceQpDeltaCr) & 0x1f) << 24);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_SLICE_1 , u4SHDRInfo);

    u4SHDRInfo = 0;
    u4SHDRInfo = (((prSliceHdr->i4NumRefIdx[0] - 1) & 0xf) << 0) ;
    u4SHDRInfo |= (((prSliceHdr->i4NumRefIdx[1] - 1) & 0xf) << 4);
    u4SHDRInfo |= ((prSliceHdr->u4ColRefIdx & 0xf) << 8) ;
    u4SHDRInfo |= (((5 - prSliceHdr->u4FiveMinusMaxNumMergeCand) & 0x7) << 12);
    u4SHDRInfo |= ((prSliceHdr->i4DeblockingFilterBetaOffsetDiv2 & 0x1f) << 16);        //[notice] initial value need to be check
    u4SHDRInfo |= ((prSliceHdr->i4DeblockingFilterTcOffsetDiv2 & 0x1f) << 21);          //
    u4SHDRInfo |= ((prSliceHdr->bTMVPFlagsPresent & 0x1) << 26);
    u4SHDRInfo |= ((prSliceHdr->bColFromL0Flag & 0x1) << 27);
    u4SHDRInfo |= ((prSliceHdr->bMvdL1ZeroFlag & 0x1) << 28);
    u4SHDRInfo |= ((prSliceHdr->bLoopFilterAcrossSlicesEnabledFlag & 0x1) << 29);
    u4SHDRInfo |= ((prSliceHdr->bDeblockingFilterDisableFlag & 0x1) << 30);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_SLICE_2, u4SHDRInfo);

    u4SHDRInfo = 0;
    u4SHDRInfo = ((_rH265PicInfo[u4VDecID].i4StrNumDeltaPocs & 0x1f) << 0);
    u4SHDRInfo |= (((_rH265PicInfo[u4VDecID].i4StrNumNegPosPics) & 0x3f) << 8);
    u4SHDRInfo |= ((_rH265PicInfo[u4VDecID].i4NumLongTerm & 0x1f) << 16);
    u4SHDRInfo |= ((_rH265PicInfo[u4VDecID].i4NumLongTermSps & 0x1f) << 24);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_SLICE_STR_LT, u4SHDRInfo);

    // calculate i4NumRpsCurrTempList
    for (i4Indexer = 0; i4Indexer < prSliceHdr->pShortTermRefPicSets->u4NumberOfNegativePictures
         + prSliceHdr->pShortTermRefPicSets->u4NumberOfPositivePictures
         + prSliceHdr->pShortTermRefPicSets->u4NumberOfLongtermPictures; i4Indexer++)
    {
        if (prSliceHdr->pShortTermRefPicSets->bUsed[i4Indexer])
        {
            i4NumRpsCurrTempList++;
        }
    }

    numRpsCurrTempList0 = i4NumRpsCurrTempList;
    if (numRpsCurrTempList0 > 1)
    {
        length = 1;
        numRpsCurrTempList0 --;
    }
    while (numRpsCurrTempList0 >>= 1)
    {
        length ++;
    }

    u4SHDRInfo = 0;
    u4SHDRInfo = ((i4NumRpsCurrTempList & 0xf) << 0) | ((length & 0x7) << 4);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_REF_PIC_LIST_MOD, u4SHDRInfo);

    u4SHDRInfo = 0;
    u4SHDRInfo  = (0xffff << 16) | (prSliceHdr->u4NalType << 8) | (21);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_ERR_DET_CTRL, u4SHDRInfo);
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetSHDRHEVLD() done!!\n");
#endif

}

// **************************************************************************
// Function :void vVDEC_HAL_H265_SetSLPP(UINT32 u4VDecID, pH265_SL_Data ScallingList)
// Description :Set scaling list data to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 ScallingList : scaling list data
/// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_SetSLPP(UINT32 u4VDecID, pH265_SL_Data ScallingList)
{
    UINT32  u4SLInfo;

    _rH265PicInfo[u4VDecID].u4IqSramAddrAccCnt = 0;

    if (_rH265PicInfo[u4VDecID].u4SliceCnt == 0)
    {
        u4SLInfo = 0;
        u4SLInfo = ((ScallingList->i4ScalingListDC[2][0] & 0xff) << 0);
        u4SLInfo |= ((ScallingList->i4ScalingListDC[2][1] & 0xff) << 8);
        u4SLInfo |= ((ScallingList->i4ScalingListDC[2][2] & 0xff) << 16);
        u4SLInfo |= ((ScallingList->i4ScalingListDC[2][3] & 0xff) << 24);

        vVDecWriteHEVCPP(u4VDecID, HEVC_IQ_SACLING_FACTOR_DC_0, u4SLInfo);

        u4SLInfo = 0;
        u4SLInfo = ((ScallingList->i4ScalingListDC[2][4] & 0xff) << 0);
        u4SLInfo |= ((ScallingList->i4ScalingListDC[2][5] & 0xff) << 8);
        u4SLInfo |= ((ScallingList->i4ScalingListDC[3][0] & 0xff) << 16);
        u4SLInfo |= ((ScallingList->i4ScalingListDC[3][1] & 0xff) << 24);

        vVDecWriteHEVCPP(u4VDecID, HEVC_IQ_SACLING_FACTOR_DC_1, u4SLInfo);
    }

}


// **************************************************************************
// Function :void vVDEC_HAL_H265_SetSLVLD(UINT32 u4VDecID, INT32 *coeff, INT32 width, INT32 height, INT32 invQuantScales)
// Description :Set scaling list data to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 coeff : scaling list coefficient
//                 width, height, invQuantScales
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_SetSLVLD(UINT32 u4VDecID, INT32 *coeff, INT32 width, INT32 height, INT32 invQuantScales)
{
    int index;
    UINT32  u4SLInfo;

    if ((invQuantScales == 40) && (width == height))
    {
        if (width == 4) //4x4
        {
            for (index = 0; index < 4; index++)
            {
                if (_rH265PicInfo[u4VDecID].u4SliceCnt == 0)
                {
                    vVDecWriteVLD(u4VDecID, HEVC_IQ_SRAM_ADDR, _rH265PicInfo[u4VDecID].u4IqSramAddrAccCnt);
                    u4SLInfo = 0;
                    u4SLInfo = (coeff[0 * 4 + index] & 0xff) << 0;
                    u4SLInfo |= (coeff[1 * 4 + index] & 0xff) << 8;
                    u4SLInfo |= (coeff[2 * 4 + index] & 0xff) << 16;
                    u4SLInfo |= (coeff[3 * 4 + index] & 0xff) << 24;

                    vVDecWriteVLD(u4VDecID, HEVC_IQ_SRAM_DATA, u4SLInfo);
                }
                _rH265PicInfo[u4VDecID].u4IqSramAddrAccCnt++;
            }

        }
        else //8x8, 16x16, 32x32
        {
            for (index = 0; index < 8; index++)
            {
                if (_rH265PicInfo[u4VDecID].u4SliceCnt == 0)
                {
                    vVDecWriteVLD(u4VDecID, HEVC_IQ_SRAM_ADDR, _rH265PicInfo[u4VDecID].u4IqSramAddrAccCnt);
                    u4SLInfo = 0;
                    u4SLInfo = (coeff[0 * 8 + index] & 0xff) << 0;
                    u4SLInfo |= (coeff[1 * 8 + index] & 0xff) << 8;
                    u4SLInfo |= (coeff[2 * 8 + index] & 0xff) << 16;
                    u4SLInfo |= (coeff[3 * 8 + index] & 0xff) << 24;

                    vVDecWriteVLD(u4VDecID, HEVC_IQ_SRAM_DATA, u4SLInfo);
                }
                _rH265PicInfo[u4VDecID].u4IqSramAddrAccCnt++;

                if (_rH265PicInfo[u4VDecID].u4SliceCnt == 0)
                {
                    vVDecWriteVLD(u4VDecID, HEVC_IQ_SRAM_ADDR, _rH265PicInfo[u4VDecID].u4IqSramAddrAccCnt);
                    u4SLInfo = 0;
                    u4SLInfo = (coeff[4 * 8 + index] & 0xff) << 0;
                    u4SLInfo |= (coeff[5 * 8 + index] & 0xff) << 8;
                    u4SLInfo |= (coeff[6 * 8 + index] & 0xff) << 16;
                    u4SLInfo |= (coeff[7 * 8 + index] & 0xff) << 24;

                    vVDecWriteVLD(u4VDecID, HEVC_IQ_SRAM_DATA, u4SLInfo);
                }
                _rH265PicInfo[u4VDecID].u4IqSramAddrAccCnt++;
            }
        }
    }


}



// **************************************************************************
// Function : void vVDEC_HAL_H265_SetTilesInfo(UINT32 u4VDecID, H265_SPS_Data *prSPS, H265_PPS_Data *prPPS)
//// Description :Set Tiles Info to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 prSPS
//                 prPPS : pointer to picture parameter set struct
//
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_SetTilesInfo(UINT32 u4VDecID, H265_SPS_Data *prSPS, H265_PPS_Data *prPPS)
{
    UINT32  uiColumnIdx = 0;
    UINT32  uiRowIdx = 0;
    UINT32  uiRightEdgePosInCU;
    UINT32  uiBottomEdgePosInCU;

    UINT32  last_lcu_x_in_tile_0_reg = 0;
    UINT32  last_lcu_x_in_tile_1_reg = 0;
    UINT32  last_lcu_x_in_tile_2_reg = 0;
    UINT32  last_lcu_x_in_tile_3_reg = 0;
    UINT32  last_lcu_x_in_tile_4_reg = 0;
    UINT32  last_lcu_x_in_tile_5_reg = 0;
    UINT32  last_lcu_x_in_tile_6_reg = 0;
    UINT32  last_lcu_x_in_tile_7_reg = 0;
    UINT32  last_lcu_x_in_tile_8_reg = 0;
    UINT32  last_lcu_x_in_tile_9_reg = 0;
    UINT32  last_lcu_x_in_tile_10_reg = 0;
    UINT32  last_lcu_x_in_tile_11_reg = 0;
    UINT32  last_lcu_x_in_tile_12_reg = 0;
    UINT32  last_lcu_x_in_tile_13_reg = 0;
    UINT32  last_lcu_x_in_tile_14_reg = 0;
    UINT32  last_lcu_x_in_tile_15_reg = 0;
    UINT32  last_lcu_x_in_tile_16_reg = 0;

    UINT32  last_lcu_y_in_tile_0_reg = 0;
    UINT32  last_lcu_y_in_tile_1_reg = 0;
    UINT32  last_lcu_y_in_tile_2_reg = 0;
    UINT32  last_lcu_y_in_tile_3_reg = 0;
    UINT32  last_lcu_y_in_tile_4_reg = 0;
    UINT32  last_lcu_y_in_tile_5_reg = 0;
    UINT32  last_lcu_y_in_tile_6_reg = 0;
    UINT32  last_lcu_y_in_tile_7_reg = 0;
    UINT32  last_lcu_y_in_tile_8_reg = 0;
    UINT32  last_lcu_y_in_tile_9_reg = 0;
    UINT32  last_lcu_y_in_tile_10_reg = 0;
    UINT32  last_lcu_y_in_tile_11_reg = 0;
    UINT32  last_lcu_y_in_tile_12_reg = 0;
    UINT32  last_lcu_y_in_tile_13_reg = 0;
    UINT32  last_lcu_y_in_tile_14_reg = 0;
    UINT32  last_lcu_y_in_tile_15_reg = 0;
    UINT32  last_lcu_y_in_tile_16_reg = 0;

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetTilesInfo() start!!\n");
#endif

    for (uiColumnIdx = 0; uiColumnIdx < prPPS->u4NumColumnsMinus1 + 1; uiColumnIdx++)
    {
        uiRightEdgePosInCU = _rH265PicInfo[u4VDecID].rTileInfo[uiColumnIdx].u4RightEdgePosInCU;
        if (uiRightEdgePosInCU >= 512)
        {
            last_lcu_x_in_tile_16_reg = last_lcu_x_in_tile_16_reg | (1 << (uiRightEdgePosInCU - 512));
        }
        else if (uiRightEdgePosInCU >= 480)
        {
            last_lcu_x_in_tile_15_reg = last_lcu_x_in_tile_15_reg | (1 << (uiRightEdgePosInCU - 480));
        }
        else if (uiRightEdgePosInCU >= 448)
        {
            last_lcu_x_in_tile_14_reg = last_lcu_x_in_tile_14_reg | (1 << (uiRightEdgePosInCU - 448));
        }
        else if (uiRightEdgePosInCU >= 416)
        {
            last_lcu_x_in_tile_13_reg = last_lcu_x_in_tile_13_reg | (1 << (uiRightEdgePosInCU - 416));
        }
        else if (uiRightEdgePosInCU >= 384)
        {
            last_lcu_x_in_tile_12_reg = last_lcu_x_in_tile_12_reg | (1 << (uiRightEdgePosInCU - 384));
        }
        else if (uiRightEdgePosInCU >= 352)
        {
            last_lcu_x_in_tile_11_reg = last_lcu_x_in_tile_11_reg | (1 << (uiRightEdgePosInCU - 352));
        }
        else if (uiRightEdgePosInCU >= 320)
        {
            last_lcu_x_in_tile_10_reg = last_lcu_x_in_tile_10_reg | (1 << (uiRightEdgePosInCU - 320));
        }
        else if (uiRightEdgePosInCU >= 288)
        {
            last_lcu_x_in_tile_9_reg = last_lcu_x_in_tile_9_reg | (1 << (uiRightEdgePosInCU - 288));
        }
        else if (uiRightEdgePosInCU >= 256)
        {
            last_lcu_x_in_tile_8_reg = last_lcu_x_in_tile_8_reg | (1 << (uiRightEdgePosInCU - 256));
        }
        else if (uiRightEdgePosInCU >= 224)
        {
            last_lcu_x_in_tile_7_reg = last_lcu_x_in_tile_7_reg | (1 << (uiRightEdgePosInCU - 224));
        }
        else if (uiRightEdgePosInCU >= 192)
        {
            last_lcu_x_in_tile_6_reg = last_lcu_x_in_tile_6_reg | (1 << (uiRightEdgePosInCU - 192));
        }
        else if (uiRightEdgePosInCU >= 160)
        {
            last_lcu_x_in_tile_5_reg = last_lcu_x_in_tile_5_reg | (1 << (uiRightEdgePosInCU - 160));
        }
        else if (uiRightEdgePosInCU >= 128)
        {
            last_lcu_x_in_tile_4_reg = last_lcu_x_in_tile_4_reg | (1 << (uiRightEdgePosInCU - 128));
        }
        else if (uiRightEdgePosInCU >= 96)
        {
            last_lcu_x_in_tile_3_reg = last_lcu_x_in_tile_3_reg | (1 << (uiRightEdgePosInCU - 96));
        }
        else if (uiRightEdgePosInCU >= 64)
        {
            last_lcu_x_in_tile_2_reg = last_lcu_x_in_tile_2_reg | (1 << (uiRightEdgePosInCU - 64));
        }
        else if (uiRightEdgePosInCU >= 32)
        {
            last_lcu_x_in_tile_1_reg = last_lcu_x_in_tile_1_reg | (1 << (uiRightEdgePosInCU - 32));
        }
        else
        {
            last_lcu_x_in_tile_0_reg = last_lcu_x_in_tile_0_reg | (1 << (uiRightEdgePosInCU - 0));
        }
    }

    for (uiRowIdx = 0; uiRowIdx < prPPS->u4NumRowsMinus1 + 1; uiRowIdx++)
    {
        uiBottomEdgePosInCU = _rH265PicInfo[u4VDecID].rTileInfo[uiRowIdx * (prPPS->u4NumColumnsMinus1 + 1)].u4BottomEdgePosInCU;
        if (uiBottomEdgePosInCU >= 512)
        {
            last_lcu_y_in_tile_16_reg = last_lcu_y_in_tile_16_reg | (1 << (uiBottomEdgePosInCU - 512));
        }
        else if (uiBottomEdgePosInCU >= 480)
        {
            last_lcu_y_in_tile_15_reg = last_lcu_y_in_tile_15_reg | (1 << (uiBottomEdgePosInCU - 480));
        }
        else if (uiBottomEdgePosInCU >= 448)
        {
            last_lcu_y_in_tile_14_reg = last_lcu_y_in_tile_14_reg | (1 << (uiBottomEdgePosInCU - 448));
        }
        else if (uiBottomEdgePosInCU >= 416)
        {
            last_lcu_y_in_tile_13_reg = last_lcu_y_in_tile_13_reg | (1 << (uiBottomEdgePosInCU - 416));
        }
        else if (uiBottomEdgePosInCU >= 384)
        {
            last_lcu_y_in_tile_12_reg = last_lcu_y_in_tile_12_reg | (1 << (uiBottomEdgePosInCU - 384));
        }
        else if (uiBottomEdgePosInCU >= 352)
        {
            last_lcu_y_in_tile_11_reg = last_lcu_y_in_tile_11_reg | (1 << (uiBottomEdgePosInCU - 352));
        }
        else if (uiBottomEdgePosInCU >= 320)
        {
            last_lcu_y_in_tile_10_reg = last_lcu_y_in_tile_10_reg | (1 << (uiBottomEdgePosInCU - 320));
        }
        else if (uiBottomEdgePosInCU >= 288)
        {
            last_lcu_y_in_tile_9_reg = last_lcu_y_in_tile_9_reg | (1 << (uiBottomEdgePosInCU - 288));
        }
        else if (uiBottomEdgePosInCU >= 256)
        {
            last_lcu_y_in_tile_8_reg = last_lcu_y_in_tile_8_reg | (1 << (uiBottomEdgePosInCU - 256));
        }
        else if (uiBottomEdgePosInCU >= 224)
        {
            last_lcu_y_in_tile_7_reg = last_lcu_y_in_tile_7_reg | (1 << (uiBottomEdgePosInCU - 224));
        }
        else if (uiBottomEdgePosInCU >= 192)
        {
            last_lcu_y_in_tile_6_reg = last_lcu_y_in_tile_6_reg | (1 << (uiBottomEdgePosInCU - 192));
        }
        else if (uiBottomEdgePosInCU >= 160)
        {
            last_lcu_y_in_tile_5_reg = last_lcu_y_in_tile_5_reg | (1 << (uiBottomEdgePosInCU - 160));
        }
        else if (uiBottomEdgePosInCU >= 128)
        {
            last_lcu_y_in_tile_4_reg = last_lcu_y_in_tile_4_reg | (1 << (uiBottomEdgePosInCU - 128));
        }
        else if (uiBottomEdgePosInCU >= 96)
        {
            last_lcu_y_in_tile_3_reg = last_lcu_y_in_tile_3_reg | (1 << (uiBottomEdgePosInCU - 96));
        }
        else if (uiBottomEdgePosInCU >= 64)
        {
            last_lcu_y_in_tile_2_reg = last_lcu_y_in_tile_2_reg | (1 << (uiBottomEdgePosInCU - 64));
        }
        else if (uiBottomEdgePosInCU >= 32)
        {
            last_lcu_y_in_tile_1_reg = last_lcu_y_in_tile_1_reg | (1 << (uiBottomEdgePosInCU - 32));
        }
        else
        {
            last_lcu_y_in_tile_0_reg = last_lcu_y_in_tile_0_reg | (1 << (uiBottomEdgePosInCU - 0));
        }

    }

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_0, last_lcu_x_in_tile_0_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_1, last_lcu_x_in_tile_1_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_2, last_lcu_x_in_tile_2_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_3, last_lcu_x_in_tile_3_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_4, last_lcu_x_in_tile_4_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_5, last_lcu_x_in_tile_5_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_6, last_lcu_x_in_tile_6_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_7, last_lcu_x_in_tile_7_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_8, last_lcu_x_in_tile_8_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_9, last_lcu_x_in_tile_9_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_10, last_lcu_x_in_tile_10_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_11, last_lcu_x_in_tile_11_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_12, last_lcu_x_in_tile_12_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_13, last_lcu_x_in_tile_13_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_14, last_lcu_x_in_tile_14_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_15, last_lcu_x_in_tile_15_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_X_16, last_lcu_x_in_tile_16_reg);

    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_0, last_lcu_y_in_tile_0_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_1, last_lcu_y_in_tile_1_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_2, last_lcu_y_in_tile_2_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_3, last_lcu_y_in_tile_3_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_4, last_lcu_y_in_tile_4_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_5, last_lcu_y_in_tile_5_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_6, last_lcu_y_in_tile_6_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_7, last_lcu_y_in_tile_7_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_8, last_lcu_y_in_tile_8_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_9, last_lcu_y_in_tile_9_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_10, last_lcu_y_in_tile_10_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_11, last_lcu_y_in_tile_11_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_12, last_lcu_y_in_tile_12_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_13, last_lcu_y_in_tile_13_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_14, last_lcu_y_in_tile_14_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_15, last_lcu_y_in_tile_15_reg);
    vVDecWriteHEVCVLD(u4VDecID, RW_HEVC_TILE_Y_16, last_lcu_y_in_tile_16_reg);

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetTilesInfo() done!!\n");
#endif

}

#ifdef  HEVC_STILL_IMAGE

// **************************************************************************
// Function : void vVDEC_HAL_H265_SetStillImageInfo(UINT32 u4VDecID, H265_SPS_Data *prSPS, H265_PPS_Data *prPPS)
//// Description :Set Still image Info to HW
// Parameter : u4VDecID : video decoder hardware ID
//                 prSPS
//                 prPPS : pointer to picture parameter set struct
//
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_SetStillImageInfo(UINT32 u4VDecID, H265_SPS_Data *prSPS, H265_PPS_Data *prPPS)
{
    UINT32 PIC_WIDTH_ALIGN, PIC_HEIGHT_ALIGN;
    UINT32 Horizontal_Y, Horizontal_C, Tile_Vertical_Y, Tile_Vertical_C, Tile_Horizontal_Y, Tile_Horizontal_C, Tile_Y_Flag;
    UINT32 Horizontal_Sign, Vertical_Sign_Y, Vertical_Sign_C;
    UINT32 u4RetVal;

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetStillImageInfo() start!!\n");
#endif

    //PIC_WIDTH_ALIGN = ((prSPS->u4PicWidthInLumaSamples+63) >> 6) << 6;
    //PIC_HEIGHT_ALIGN = ((prSPS->u4PicHeightInLumaSamples+31) >> 5) << 5;
    PIC_WIDTH_ALIGN = ((HEVC_STILL_IMG_MAX_WH + 63) >> 6) << 6;
    PIC_HEIGHT_ALIGN = ((HEVC_STILL_IMG_MAX_WH + 31) >> 5) << 5;

    Horizontal_Y = PIC_WIDTH_ALIGN >> 2;
    Horizontal_C = PIC_WIDTH_ALIGN >> 2;

    Tile_Vertical_Y = PIC_HEIGHT_ALIGN >> 2;
    Tile_Vertical_C = PIC_HEIGHT_ALIGN >> 3;

    Tile_Y_Flag = PIC_HEIGHT_ALIGN >> 4;

    Tile_Horizontal_Y = PIC_WIDTH_ALIGN >> 4;
    Tile_Horizontal_C = PIC_WIDTH_ALIGN >> 4;

    //SAO Paramater
    Horizontal_Sign = PIC_WIDTH_ALIGN >> 5;

    Vertical_Sign_Y  = PIC_HEIGHT_ALIGN >> 4;
    Vertical_Sign_C  = PIC_HEIGHT_ALIGN >> 5;

	pr_debug("[INFO] Still Image profile!!\n");

    // VLD Wrapper Setting:
    vVDecWriteVLDTOP(u4VDecID, 11 * 4, 0xe0000000); // Switch VLD Wrapper to Dram mode
    vVDecWriteVLDTOP(u4VDecID, 16 * 4, 0x3); //No dram burst mode for HEVC
    vVDecWriteVLDTOP(u4VDecID, 10 * 4, PHYSICAL(_pucVLDWrapperWrok[u4VDecID])); // Dram Size: PIC_WIDTHx3 bytes
    vVDecWriteVLDTOP(u4VDecID, 38 * 4, PHYSICAL(_pucVLDWrapperWrok[u4VDecID]) + HEVC_STILL_IMG_MAX_WH * 3); // Dram Size: PIC_WIDTHx3 bytes
    vVDecWriteVLDTOP(u4VDecID, 46 * 4, PHYSICAL(_pucVLDWrapperWrok[u4VDecID]) + HEVC_STILL_IMG_MAX_WH * 6); // Dram Size: PIC_HEIGHTx2 bytes

    // PP Wrapper Setting:
    vVDecWriteHEVCPP(u4VDecID, 771 * 4, 0x1); // Switch PP Wrapper to Dram mode
    vVDecWriteHEVCPP(u4VDecID, 769 * 4, PHYSICAL(_pucPPWrapperWork[u4VDecID])); // Dram Size: 4352x16 bytes
    vVDecWriteHEVCPP(u4VDecID, 770 * 4, PHYSICAL(_pucPPWrapperWork[u4VDecID]) + 4352 * 16); // Dram Size: 18432x16 bytes
    u4RetVal = u4VDecReadHEVCPP(u4VDecID, 512 * 4);
    vVDecWriteHEVCPP(u4VDecID, 512 * 4, (u4RetVal | (0x1 << 8)));

    // DBK (Deblocking) Setting
    vVDecWriteHEVCPP(u4VDecID, 45 * 4,   0x0); // DBK_Wrapper_Start_Offset
    vVDecWriteHEVCPP(u4VDecID, 46 * 4,   Horizontal_Y); // Horizontal_Y_Offset
    vVDecWriteHEVCPP(u4VDecID, 47 * 4,   Horizontal_Y + Horizontal_C); // Horizontal_C_Offset
    vVDecWriteHEVCPP(u4VDecID, 48 * 4,   Horizontal_Y + Horizontal_C + Tile_Vertical_Y); // Tile_Vertical_Y_Offset
    vVDecWriteHEVCPP(u4VDecID, 49 * 4,   Horizontal_Y + Horizontal_C + Tile_Vertical_Y + Tile_Vertical_C); // Tile_Vertical_C_Offset
    vVDecWriteHEVCPP(u4VDecID, 50 * 4,   Horizontal_Y + Horizontal_C + Tile_Vertical_Y + Tile_Vertical_C + Tile_Y_Flag); // Tile_Y_Flag_Offset
    vVDecWriteHEVCPP(u4VDecID, 51 * 4,   Horizontal_Y + Horizontal_C + Tile_Vertical_Y + Tile_Vertical_C + Tile_Y_Flag + Tile_Horizontal_Y); // Tile_Horizontal_Y_Offset
    vVDecWriteHEVCPP(u4VDecID, 52 * 4,   PIC_WIDTH_ALIGN / 2); // PIC_WIDTH_ALIGN/2

    // SAO Setting
    vVDecWriteHEVCPP(u4VDecID, 705 * 4, Horizontal_Sign); // SAO_Horizontal_Sign_Offset
    u4RetVal = u4VDecReadHEVCPP(u4VDecID, 705 * 4);
    vVDecWriteHEVCPP(u4VDecID, 706 * 4, u4RetVal + Vertical_Sign_Y); // SAO_Vertical_Sign_Y_Offset
    u4RetVal = u4VDecReadHEVCPP(u4VDecID, 706 * 4);
    vVDecWriteHEVCPP(u4VDecID, 707 * 4, u4RetVal + Vertical_Sign_C); // SAO_Vertical_Sign_C_Offset

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] vVDEC_HAL_H265_SetStillImageInfo() done!!\n");
#endif

}
#endif


// **************************************************************************
// Function : INT32 i4VDEC_HAL_H265_DecStart(UINT32 u4VDecID, VDEC_INFO_DEC_PRM_T *prDecPrm);
// Description :Set video decoder hardware registers to decode for H265
// Parameter : ptHalDecH265Info : pointer to H265 decode info struct
// Return      : =0: success.
//                  <0: fail.
// **************************************************************************
INT32 i4VDEC_HAL_H265_DecStart(UINT32 u4VDecID, VDEC_INFO_DEC_PRM_T *prDecPrm)
{

    vVDecWriteHEVCVLD(u4VDecID, HEVLD_PIC_TRG_REG, 0x1);

    return HAL_HANDLE_OK;
}


// **************************************************************************
// Function : void v4VDEC_HAL_H265_GetMbxMby(UINT32 u4VDecID, UINT32 *pu4Mbx, UINT32 *pu4Mby);
// Description :Read current decoded mbx and mby
// Parameter : u4VDecID : video decoder hardware ID
//                 u4Mbx : macroblock x value
//                 u4Mby : macroblock y value
// Return      : None
// **************************************************************************
void vVDEC_HAL_H265_GetMbxMby(UINT32 u4VDecID, UINT32 *pu4Mbx, UINT32 *pu4Mby)
{
    UINT32 u4RegReturnValue;
    //DBG_H265_PRINTF(pfLogFile,"\n[Info] %s() \n", __FUNCTION__);
    u4RegReturnValue = u4VDecReadHEVCVLD(u4VDecID, HEVC_LCU_POS_REG);
    *pu4Mbx = (u4RegReturnValue & 0x0000FFFF);
    *pu4Mby = ((u4RegReturnValue >> 16) & 0x0000FFFF);
}


// **************************************************************************
// Function : BOOL fgVDEC_HAL_H265_DecPicComplete(UINT32 u4VDecID);
// Description :Check if all video decoder modules are complete
// Parameter : u4VDecID : video decoder hardware ID
// Return      : TRUE: Decode complete, FALSE: Not yet
// **************************************************************************
BOOL fgVDEC_HAL_H265_DecPicComplete(UINT32 u4VDecID)
{
    if (u4VDecReadHEVCVLD(u4VDecID, RO_HEVLD_STATE_INFO) & HEVLD_PIC_FINISH)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// **************************************************************************
// Function : void u4VDEC_HAL_H265_GetErrMsg(UINT32 u4VDecID);
// Description :Read h265 error message after decoding end
// Parameter : u4VDecID : video decoder hardware ID
// Return      : H265 decode error message
// **************************************************************************
UINT32 u4VDEC_HAL_H265_GetErrMsg(UINT32 u4VDecID)
{
    return (u4VDecReadHEVCVLD(u4VDecID, RO_HEVLD_ERR_TYPE) | u4VDecReadHEVCVLD(u4VDecID, RO_HEVLD_ERR_ACCUMULATOR))
           & u4VDecReadHEVCVLD(u4VDecID, HEVC_VLD_ERROR_TYPE_ENABLE);
}


// **************************************************************************
// Function : void u4VDEC_HAL_H265_GetErrMsg(UINT32 u4VDecID);
// Description :Read h265 error message after decoding end
// Parameter : u4VDecID : video decoder hardware ID
// Return      : H265 decode error message
// **************************************************************************
BOOL fgVDEC_HAL_H265_ChkErrInfo(UINT32 ucBsId, UINT32 u4VDecID, UINT32 u4DecErrInfo, UINT32 u4ECLevel)
{
    UINT32 u4Data;
    BOOL fgIsVDecErr;

    fgIsVDecErr = TRUE;

    switch (u4ECLevel)
    {
        case 2:
            // Ignore the real non-NextStartCode condition
            if ((u4DecErrInfo == (CABAC_ZERO_WORD_ERR | NO_NEXT_START_CODE))
                // Add For CQ: 31166, 31113 Customer_B_B_K: HEVCHD Disc
                || (u4DecErrInfo == (CABAC_ZERO_WORD_ERR))
               )
            {
                fgIsVDecErr = FALSE;
            }
        case 0:
        case 1:
        default:
            if (u4DecErrInfo == CABAC_ZERO_WORD_ERR)
            {
                vVDEC_HAL_H265_TrailingBits(ucBsId, u4VDecID);
                u4Data = u4VDEC_HAL_H265_ShiftGetBitStream(ucBsId, u4VDecID, 0);
                if (((u4Data >> 8) == START_CODE) || (u4Data == 0x00000000) || (u4Data == START_CODE))
                {
                    fgIsVDecErr = FALSE;
                }
            }
            else if (u4DecErrInfo == NO_NEXT_START_CODE) // don't care "No next start code"
            {
                fgIsVDecErr = FALSE;
            }
            else if ((u4DecErrInfo == CABAC_ALIGN_BIT_ERR) && (!(u4VDecReadHEVCVLD(u4VDecID, RW_HEVLD_ERR_MASK) & CABAC_ALIGN_BIT_ERR))) // don't care "No next start code"
            {
                fgIsVDecErr = FALSE;
            }
            break;
    }

    return fgIsVDecErr;
}



void vVDEC_HAL_H265_VDec_PowerDown(UCHAR u4VDecID)
{
#if (CONFIG_CHIP_VER_CURR >= CONFIG_CHIP_VER_MT8555)
    vVDecPowerDownHW(u4VDecID);
#endif
}

#if CONFIG_DRV_VERIFY_SUPPORT
UINT32 u4VDEC_HAL_H265_VDec_ReadFinishFlag(UINT32 u4VDecID)
{
    return ((u4VDecReadHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE) >> 16) & 0x1);
}

UINT32 u4VDEC_HAL_H265_VDec_ClearInt(UINT32 u4VDecID)
{
#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] u4VDEC_HAL_H265_VDec_ClearInt() start!!\n");
#endif
    UINT32 u4temp;
    vVDecWriteHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE,  u4VDecReadHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE) | 0x1);
    u4temp = u4VDecReadHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE);
    vVDecWriteHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE,  u4temp | (0x1 << 4));
    vVDecWriteHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE,  u4temp & 0xffffffef);
    u4temp = u4VDecReadHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE);
    vVDecWriteHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE,  u4temp | (0x1 << 12)); // clear for VP mode

#ifdef VDEC_SIM_DUMP
	pr_debug("[INFO] u4VDEC_HAL_H265_VDec_ClearInt() done!!\n");
#endif

}

extern int  Wait_decode_finished(unsigned long  start_time);

UINT32 vVDEC_HAL_H265_VDec_VPmode(UINT32 u4VDecID)
{
    UINT32 risc_val1, pic_width, pic_height;
    BOOL bIsUFO;
    H265_SPS_Data *prSPS;

    bIsUFO = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.bIsUFOMode;
    prSPS = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.prSPS;

    //VP mode for end of slice error
    risc_val1 = u4VDecReadHEVCVLD(u4VDecID, 57 * 4);
    //if (_u4PicCnt[u4VDecID] >= 1){  risc_val1 = 1; }      // force frame num turn on VP mode for debug

    if (risc_val1 & 0x1)
    {

#if 1   //Eve_rest: 10bits UFO supported
        UINT32 SliceStartLCURow, u4LCUsize, u4RealWidth, u4W_Dram;
        UINT32 pic_real_wifth, pic_real_height, i, minDPOC;
        UINT32 VLD_26_W, VLD_26_H, MC_608, VLD_TOP_26, VLD_TOP_28;
        VDEC_INFO_H265_INIT_PRM_T rH265VDecInitPrm;
        UINT8 ucRefFBIndex;

        //search min delta POC pic in DPB
        minDPOC = MAX_INT;
        for (i = 0; i < _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.ucMaxFBufNum; i++)
        {
            if (i == _tVerMpvDecPrm[u4VDecID].ucDecFBufIdx)
            {
                continue;
            }
            if (abs(_ptH265CurrFBufInfo[u4VDecID]->i4POC - _ptH265FBufInfo[u4VDecID][i].i4POC) < minDPOC)
            {
                minDPOC = abs(_ptH265CurrFBufInfo[u4VDecID]->i4POC - _ptH265FBufInfo[u4VDecID][i].i4POC);
                ucRefFBIndex = i;
            }
        }
        if (minDPOC == MAX_INT)
        {
            ucRefFBIndex = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.ucPreFBIndex;
        }

        rH265VDecInitPrm.u4FGDatabase = (UINT32)_pucFGDatabase[u4VDecID];
        rH265VDecInitPrm.u4FGSeedbase = (UINT32)_pucFGSeedbase[u4VDecID];

        risc_val1 = u4VDecReadHEVCVLD(u4VDecID, RW_HEVLD_SPS_SIZE);
        u4LCUsize = 1 << ((risc_val1 >> 4) & 0x7);
        risc_val1 = u4VDecReadHEVCVLD(u4VDecID, RO_VLD_VWPTR);
        SliceStartLCURow = (risc_val1 >> 16)  & 0x3ff;

        risc_val1 = u4VDecReadVLDTOP(u4VDecID, HEVC_VLD_TOP_PIC_PIX_SIZE);
        u4RealWidth = risc_val1 & 0xFFFF;
        u4W_Dram = ((u4RealWidth + 63) / 64) * 64;

        if ((SliceStartLCURow % 2) == 1 && u4LCUsize == 16)
        {
            SliceStartLCURow--;
        }

        if (bIsUFO)     //UFO HW constrain
        {
            while (SliceStartLCURow * u4LCUsize * u4W_Dram % (8 * 4096) != 0 || ((SliceStartLCURow % 2) == 1 && u4LCUsize == 16))
            {
                SliceStartLCURow--;
            }
        }

        //SliceStartLCURow=0; //full frame copy test

		pr_debug("[INFO] VP mode!!  SliceStartLCURow %d; u4LCUsisze %d; refBufferIndex %d(pic #%d)\n", SliceStartLCURow, u4LCUsize, ucRefFBIndex, _ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4PicCnt);

        pic_real_wifth = u4VDecReadMC(u4VDecID, HEVC_PIC_WIDTH);
        VLD_26_W = ((pic_real_wifth + 15) >> 4) << 4;
        pic_real_height = u4VDecReadMC(u4VDecID, HEVC_PIC_HEIGHT);
        VLD_26_H = ((pic_real_height + 31) >> 5) << 5;
        VLD_TOP_26 = ((VLD_26_H / 16 - 1) << 16)  | (VLD_26_W / 16 - 1);

        i4VDEC_HAL_H265_InitVDecHW(u4VDecID);
        vVDecWriteHEVCMISC(u4VDecID, 41 * 4, u4VDecReadHEVCMISC(u4VDecID, 41 * 4) & (0x0 << 12));


        if (_ptH265CurrFBufInfo[u4VDecID]->bIsMain10 == 0)
        {
            vVDecWriteVLDTOP(u4VDecID, 64 * 4, 0x0);    //8bits
        }
        else
        {
            vVDecWriteVLDTOP(u4VDecID, 64 * 4, 0x2);    //10bits
        }

        vVDecWriteVLDTOP(u4VDecID, RW_VLD_PIC_MB_SIZE_M1, VLD_TOP_26);

        vVDecWriteVLDTOP(u4VDecID, 56 * 4, PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4YStartAddr));
        vVDecWriteVLDTOP(u4VDecID, 57 * 4, PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4YStartAddr + _ptH265CurrFBufInfo[u4VDecID]->u4CAddrOffset));
        vVDecWriteVLDTOP(u4VDecID, 58 * 4, PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr));
        vVDecWriteVLDTOP(u4VDecID, 59 * 4, PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr + _ptH265CurrFBufInfo[u4VDecID]->u4CAddrOffset));

        //Size
        // vVDecWriteVLDTOP(u4VDecID, 67*4, ((_tVerMpvDecPrm[u4VDecID].u4PicH*_tVerMpvDecPrm[u4VDecID].u4PicW*3/2+15)/16));
        UINT32 MoveSizeInMB = (VLD_26_W / 16 * (((pic_real_height + 15) >> 4) << 4) / 16) - SliceStartLCURow * u4LCUsize / 16 * VLD_26_W / 16;
        UINT32 Start_X = 0;
		pr_debug("[INFO] Start X:0x%x Size:0x%x\n", Start_X, MoveSizeInMB);
        vVDecWriteVLDTOP(u4VDecID, 67 * 4, u4VDecReadVLDTOP(u4VDecID, 67 * 4) | MoveSizeInMB);

        // UFO mode settings
        if (bIsUFO)
        {
#ifdef VDEC_SIM_DUMP
			pr_debug("[INFO] VP UFO settings\n");
#endif
            vVDecWriteVLDTOP(u4VDecID, 60 * 4, PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4YLenStartAddr));
            vVDecWriteVLDTOP(u4VDecID, 61 * 4, PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4CLenStartAddr));
            vVDecWriteVLDTOP(u4VDecID, 62 * 4, PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YLenStartAddr));
            vVDecWriteVLDTOP(u4VDecID, 63 * 4, PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4CLenStartAddr));

            //start MBx / MBy
            //vVDecWriteVLDTOP(u4VDecID, 54*4, (0x0 << 16) | 0x0);
            //UFO
            vVDecWriteVLDTOP(u4VDecID, 67 * 4, u4VDecReadVLDTOP(u4VDecID, 67 * 4) | (0x1 << 28));
        }
        else
        {
            /*
                start MBx / MBy
                if(MoveSizeInMB % 2 == 1)
                {
                    MoveSizeInMB--;
                    Start_X++;
                }
            */
        }
        vVDecWriteVLDTOP(u4VDecID, 54 * 4, (Start_X << 16) | (SliceStartLCURow * u4LCUsize / 16));

#ifdef VDEC_SIM_DUMP
		pr_debug("[INFO] VP settings\n");
#endif

        vVDecWriteVLDTOP(u4VDecID, 65 * 4, (0x1 << 8) | 0x1);

        risc_val1 = u4VDecReadHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE);
        vVDecWriteHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE, risc_val1 | (0x1 << 20)) ;   //int_wait_bits_nop  (after  Oryx E3)

#else //RO_ME K_2 older VP mode
        UINT32 SliceStartLCURow, u4LCUsize, u4RealWidth, u4W_Dram;
        UINT32 pic_real_wifth, pic_real_height, i, minDPOC;
        UINT32 MC_130, MC_131, MC_608, VLD_TOP_26, VLD_TOP_28;
        VDEC_INFO_H265_INIT_PRM_T rH265VDecInitPrm;
        UINT8 ucRefFBIndex;

        //search min delta POC pic in DPB
        minDPOC = MAX_INT;
        for (i = 0; i < _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.ucMaxFBufNum; i++)
        {
            if (i == _tVerMpvDecPrm[u4VDecID].ucDecFBufIdx)
            {
                continue;
            }
            if (abs(_ptH265CurrFBufInfo[u4VDecID]->i4POC - _ptH265FBufInfo[u4VDecID][i].i4POC) < minDPOC)
            {
                minDPOC = abs(_ptH265CurrFBufInfo[u4VDecID]->i4POC - _ptH265FBufInfo[u4VDecID][i].i4POC);
                ucRefFBIndex = i;
            }
        }
        if (minDPOC == MAX_INT)
        {
            ucRefFBIndex = _tVerMpvDecPrm[u4VDecID].SpecDecPrm.rVDecH265DecPrm.ucPreFBIndex;
        }

        rH265VDecInitPrm.u4FGDatabase = (UINT32)_pucFGDatabase[u4VDecID];
        rH265VDecInitPrm.u4FGSeedbase = (UINT32)_pucFGSeedbase[u4VDecID];


        risc_val1 = u4VDecReadHEVCVLD(u4VDecID, RW_HEVLD_SPS_SIZE);
        u4LCUsize = 1 << ((risc_val1 >> 4) & 0x7);
        risc_val1 = u4VDecReadHEVCVLD(u4VDecID, RO_VLD_VWPTR);
        SliceStartLCURow = (risc_val1 >> 16)  & 0x3ff;

        risc_val1 = u4VDecReadVLDTOP(u4VDecID, HEVC_VLD_TOP_PIC_PIX_SIZE);
        u4RealWidth = risc_val1 & 0xFFFF;
        u4W_Dram = ((u4RealWidth + 63) / 64) * 64;

        if ((SliceStartLCURow % 2) == 1 && u4LCUsize == 16)
        {
            SliceStartLCURow--;
        }

        if (bIsUFO)    //UFO HW constrain
        {
            while (SliceStartLCURow * u4LCUsize * u4W_Dram % (8 * 4096) != 0 || ((SliceStartLCURow % 2) == 1 && u4LCUsize == 16))
            {
                SliceStartLCURow--;
            }
        }

        //SliceStartLCURow=0; //full frame copy test

		pr_debug("[INFO] VP mode!!  SliceStartLCURow %d; u4LCUsisze %d; refBufferIndex %d(pic #%d)\n", SliceStartLCURow, u4LCUsize, ucRefFBIndex, _ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4PicCnt);

        pic_real_wifth = u4VDecReadMC(u4VDecID, HEVC_PIC_WIDTH);
        MC_130 = ((pic_real_wifth + 15) >> 4) << 4;
        pic_real_height = u4VDecReadMC(u4VDecID, HEVC_PIC_HEIGHT);
        pic_real_height -= SliceStartLCURow * u4LCUsize;
        MC_131 = ((pic_real_height + 15) >> 4) << 4;
        MC_608 = u4VDecReadMC(u4VDecID, HEVC_DRAM_PITCH);
        VLD_TOP_26 = ((((pic_real_height + 15) / 16 - 1) & 0x7ff) << 16) | (((pic_real_wifth + 15) / 16 - 1) & 0x7ff);
        VLD_TOP_28 = (((pic_real_height + 15) >> 4) << 20) | (((pic_real_wifth + 15) >> 4) << 4);

        i4VDEC_HAL_H265_InitVDecHW(u4VDecID);

#ifdef VDEC_SIM_DUMP
		pr_debug("[INFO] VP UFO settings\n");
#endif
        // UFO mode settings
        if (bIsUFO)
        {
            pic_width = ((pic_real_wifth + 63) >> 6) << 6;
            pic_height = ((pic_real_height + 31) >> 5) << 5;
            vVDecWriteMC(u4VDecID, 700 * 4, ((pic_width / 16 - 1) << 16) | (pic_height / 16 - 1));
            vVDecWriteMC(u4VDecID, 664 * 4, 0x11);
            vVDecWriteMC(u4VDecID, 698 * 4, PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YLenStartAddr + (SliceStartLCURow * u4LCUsize * u4W_Dram / 256)));
            vVDecWriteMC(u4VDecID, 699 * 4, PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4CLenStartAddr + (SliceStartLCURow * u4LCUsize * u4W_Dram / 512)));
            //VP UFO need fixed!!
            vVDecWriteMC(u4VDecID, 663 * 4, _ptH265CurrFBufInfo[u4VDecID]->u4PicSizeCBS + (SliceStartLCURow * u4LCUsize * u4W_Dram / 256) - SliceStartLCURow * u4LCUsize * u4W_Dram);
            vVDecWriteMC(u4VDecID, 701 * 4, _ptH265CurrFBufInfo[u4VDecID]->u4UFOLenYsize - (SliceStartLCURow * u4LCUsize * u4W_Dram / 256) + (SliceStartLCURow * u4LCUsize * u4W_Dram / 512));
            vVDecWriteMC(u4VDecID, 343 * 4, _ptH265CurrFBufInfo[u4VDecID]->u4PicSizeYBS - (SliceStartLCURow * u4LCUsize * u4W_Dram) / 2);
            vVDecWriteHEVCPP(u4VDecID, 706 * 4, 0x1);  // UFO garbage remove

            // bypass PP_out setting
            vVDecWriteMC(u4VDecID, 139 * 4, ((pic_real_wifth + 15) >> 4));
            vVDecWriteMC(u4VDecID, 152 * 4, ((pic_real_wifth + 15) >> 4) - 1);
            vVDecWriteMC(u4VDecID, 153 * 4, ((pic_real_height + 15) >> 4) - 1);

            vVDecWriteMC(u4VDecID, 136 * 4, 0x1);
            risc_val1 = u4VDecReadMC(u4VDecID, 142 * 4);
            vVDecWriteMC(u4VDecID, 142 * 4, risc_val1 & (~0x3));
            vVDecWriteMC(u4VDecID, 148 * 4, 0x1);
            risc_val1 = u4VDecReadMC(u4VDecID, 525 * 4);
            vVDecWriteMC(u4VDecID, 525 * 4, risc_val1 & (~0x1));
            vVDecWriteMC(u4VDecID, 137 * 4, (PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr) + u4W_Dram * SliceStartLCURow * u4LCUsize) >> 9);
            vVDecWriteMC(u4VDecID, 138 * 4, (PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr + _ptH265CurrFBufInfo[u4VDecID]->u4CAddrOffset) + u4W_Dram * SliceStartLCURow * u4LCUsize / 2) >> 8);

        }

#ifdef VDEC_SIM_DUMP
		pr_debug("[INFO] VP settings\n");
#endif
        risc_val1 = u4VDecReadVLDTOP(u4VDecID, 36 * 4);
        vVDecWriteVLDTOP(u4VDecID, 36 * 4, risc_val1 | (0x1 << 1)); //Turn on VP mode flag

        vVDecWriteMC(u4VDecID, HEVC_PIC_WIDTH, MC_130);
        vVDecWriteMC(u4VDecID, HEVC_PIC_HEIGHT, MC_131);
        vVDecWriteMC(u4VDecID, HEVC_DRAM_PITCH, MC_608);

        vVDecWriteVLDTOP(u4VDecID, RW_VLD_PIC_MB_SIZE_M1, VLD_TOP_26);
        vVDecWriteVLDTOP(u4VDecID, HEVC_VLD_TOP_PIC_PIX_SIZE, VLD_TOP_28);

        vVDecWriteMC(u4VDecID, 9 * 4, 0x1);

        vVDecWriteMC(u4VDecID, 0 * 4, (PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4YStartAddr) + u4W_Dram * SliceStartLCURow * u4LCUsize) >> 9);
        vVDecWriteMC(u4VDecID, 1 * 4, (PHYSICAL(_ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4YStartAddr + _ptH265FBufInfo[u4VDecID][ucRefFBIndex].u4CAddrOffset) + u4W_Dram * SliceStartLCURow * u4LCUsize / 2) >> 8);
        vVDecWriteMC(u4VDecID, 2 * 4, (PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr) + u4W_Dram * SliceStartLCURow * u4LCUsize) >> 9);
        vVDecWriteMC(u4VDecID, 3 * 4, (PHYSICAL(_ptH265CurrFBufInfo[u4VDecID]->u4YStartAddr + _ptH265CurrFBufInfo[u4VDecID]->u4CAddrOffset) + u4W_Dram * SliceStartLCURow * u4LCUsize / 2) >> 8);

        risc_val1 = u4VDecReadVLDTOP(u4VDecID, 36 * 4);
        vVDecWriteVLDTOP(u4VDecID, 36 * 4,  risc_val1 | 0x1); // Trigger VP mode


        risc_val1 = u4VDecReadHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE);
        vVDecWriteHEVCMISC(u4VDecID, RW_HEVC_DEC_COMPLETE, risc_val1 & (~(0x1 << 12))) ;
#endif


        if (Wait_decode_finished(jiffies))
        {
            return 2;
        }
        else
        {
            return 0;
        }
    }

    _u4CurrPicStartAddr[1]  = 0;
    return 0;
}

extern int Dump_reg(UINT32 base_r, UINT32 start_r, UINT32 end_r , char *pBitstream_name , UINT32 frame_number, BOOL bDecodeDone);

void vVDEC_HAL_H265_VDec_DumpReg(UINT32 u4VDecID, UINT32  i4DecodeDone)
{
    char pBitstream_name[200] = {0};

    memcpy(pBitstream_name , _bFileStr1[u4VDecID][1] + 12 , (strlen(_bFileStr1[u4VDecID][1]) - 38));
    pBitstream_name[(strlen(_bFileStr1[u4VDecID][1]) - 38)] = '\0';

	pr_debug("[INFO] Dump register for %s #%d\n", pBitstream_name, _u4PicCnt[u4VDecID]);

    if (i4DecodeDone == 0 || i4DecodeDone == 1) //normal decode
    {
        Dump_reg(0xF6028000, 0, 0, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);       //HEVC
        Dump_reg(0xF6028000, 33, 37, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);   //HEVC
        Dump_reg(0xF6028000, 40, 255, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);      //HEVC
        Dump_reg(0xF6028000, 40, 255, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);      //HEVC
        Dump_reg(0xF6021000, 33, 255, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);       //VLD
        Dump_reg(0xF6024000, 0, 255, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);         //MV
        Dump_reg(0xF6022000, 0, 702, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);         //MC
        Dump_reg(0xF6025000, 0, 1023, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);        //PP
        Dump_reg(0xF6021800, 0, 50, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);        //VLD_TOP
        Dump_reg(0xF6020000, 0, 134, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);         //MISC
    }
    if (i4DecodeDone == 2)    //VPmode dump
    {
        Dump_reg(0xF6021800, 0, 67, pBitstream_name, _u4PicCnt[u4VDecID], 1);        //VLD_TOP
        Dump_reg(0xF6021000, 33, 255, pBitstream_name, _u4PicCnt[u4VDecID], 1);       //VLD
        Dump_reg(0xF6022000, 0, 702, pBitstream_name, _u4PicCnt[u4VDecID], 1);         //MC
        Dump_reg(0xF6020000, 0, 131, pBitstream_name, _u4PicCnt[u4VDecID], 1);         //MISC
    }
    if (i4DecodeDone == 3)         //special case
    {
        Dump_reg(0xF6028000, 0, 0, pBitstream_name, _u4PicCnt[u4VDecID], 0);       //HEVC
        Dump_reg(0xF6028000, 33, 37, pBitstream_name, _u4PicCnt[u4VDecID], 0);   //HEVC
        Dump_reg(0xF6028000, 40, 255, pBitstream_name, _u4PicCnt[u4VDecID], 0);      //HEVC
        Dump_reg(0xF6020000, 68, 79, pBitstream_name, _u4PicCnt[u4VDecID], 0);         //MISC
    }
    if (i4DecodeDone == 4)         //special case
    {
        Dump_reg(0xF6028000, 0, 0, pBitstream_name, _u4PicCnt[u4VDecID], i4DecodeDone);       //HEVC
    }
}


void vVDEC_HAL_H265_VDec_GetYCbCrCRC(UINT32 u4VDecID, UINT32 *pu4CRC)
{
    UINT32 u4RetValue = 0;

    u4RetValue = u4VDecReadMISC(u4VDecID, 0x8);
    u4RetValue = u4VDecReadMISC(u4VDecID, 0xC);
    u4RetValue = u4VDecReadMISC(u4VDecID, 0x10);
    u4RetValue = u4VDecReadMISC(u4VDecID, 0x14);
    u4RetValue = u4VDecReadMISC(u4VDecID, 0x18);
    u4RetValue = u4VDecReadMISC(u4VDecID, 0x1C);
    u4RetValue = u4VDecReadMISC(u4VDecID, 0x20);
    u4RetValue = u4VDecReadMISC(u4VDecID, 0x24);

    u4RetValue = u4VDecReadMISC(u4VDecID, VDEC_CRC_Y_CHKSUM0);
    pu4CRC[0] = u4RetValue;
    u4RetValue = u4VDecReadMISC(u4VDecID, VDEC_CRC_Y_CHKSUM1);
    pu4CRC[1] = u4RetValue;
    u4RetValue = u4VDecReadMISC(u4VDecID, VDEC_CRC_Y_CHKSUM2);
    pu4CRC[2] = u4RetValue;
    u4RetValue = u4VDecReadMISC(u4VDecID, VDEC_CRC_Y_CHKSUM3);
    pu4CRC[3] = u4RetValue;

    u4RetValue = u4VDecReadMISC(u4VDecID, VDEC_CRC_C_CHKSUM0);
    pu4CRC[4] = u4RetValue;
    u4RetValue = u4VDecReadMISC(u4VDecID, VDEC_CRC_C_CHKSUM1);
    pu4CRC[5] = u4RetValue;
    u4RetValue = u4VDecReadMISC(u4VDecID, VDEC_CRC_C_CHKSUM2);
    pu4CRC[6] = u4RetValue;
    u4RetValue = u4VDecReadMISC(u4VDecID, VDEC_CRC_C_CHKSUM3);
    pu4CRC[7] = u4RetValue;

	pr_debug("[INFO] Frame %d  Y CRC >> 0x%08x 0x%08x 0x%08x 0x%08x --MISC 95-98\n", _u4PicCnt[u4VDecID], pu4CRC[0], pu4CRC[1], pu4CRC[2], pu4CRC[3]);
	pr_debug("[INFO] Frame %d  C CRC >> 0x%08x 0x%08x 0x%08x 0x%08x --MISC 99-102 \n", _u4PicCnt[u4VDecID], pu4CRC[4], pu4CRC[5], pu4CRC[6], pu4CRC[7]);


    //Connie UFO debug
	pr_debug("[INFO] Frame %d  Y test CRC >> 0x%08x 0x%08x 0x%08x 0x%08x -- MISC 2-5\n", _u4PicCnt[u4VDecID], u4VDecReadMISC(u4VDecID, 2 * 4), u4VDecReadMISC(u4VDecID, 3 * 4), u4VDecReadMISC(u4VDecID, 4 * 4), u4VDecReadMISC(u4VDecID, 5 * 4));
	pr_debug("[INFO] Frame %d  C test CRC >> 0x%08x 0x%08x 0x%08x 0x%08x -- MISC 6-9\n", _u4PicCnt[u4VDecID], u4VDecReadMISC(u4VDecID, 6 * 4), u4VDecReadMISC(u4VDecID, 7 * 4), u4VDecReadMISC(u4VDecID, 8 * 4), u4VDecReadMISC(u4VDecID, 9 * 4));

    return 1;
}



void vVDEC_HAL_H265_VDec_ReadCheckSum(UINT32 u4VDecID, UINT32 *pu4CheckSum)
{
    UINT32  u4Temp, u4Cnt;

    u4Temp = 0;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x5f4);
    pu4CheckSum ++;
    u4Temp ++;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x5f8);
    pu4CheckSum ++;
    u4Temp ++;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x608);
    pu4CheckSum ++;
    u4Temp ++;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x60c);
    pu4CheckSum ++;
    u4Temp ++;

    //MC  378~397
    for (u4Cnt = 378; u4Cnt <= 397; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadMC(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //HEVC VLD  165~179
    for (u4Cnt = 165; u4Cnt <= 179; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCVLD(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //MV  147~151
    for (u4Cnt = 147; u4Cnt <= 151; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //IP  212
    *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (212 << 2));
    pu4CheckSum ++;
    u4Temp ++;

    //IQ  235~239
    for (u4Cnt = 241; u4Cnt <= 245; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //IS  241~245
    for (u4Cnt = 241; u4Cnt <= 245; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    while (u4Temp < MAX_CHKSUM_NUM)
    {
        *pu4CheckSum = 0;
        pu4CheckSum ++;
        u4Temp ++;
    }
}

BOOL fgVDEC_HAL_H265_VDec_CompCheckSum(UINT32 *pu4DecCheckSum, UINT32 *pu4GoldenCheckSum)
{
    if ((*pu4GoldenCheckSum) != (*pu4DecCheckSum))
    {
        return (FALSE);
    }
    pu4GoldenCheckSum ++;
    pu4DecCheckSum ++;
    if ((*pu4GoldenCheckSum) != (*pu4DecCheckSum))
    {
        return (FALSE);
    }
    pu4GoldenCheckSum ++;
    pu4DecCheckSum ++;
    if ((*pu4GoldenCheckSum) != (*pu4DecCheckSum))
    {
        return (FALSE);
    }
    pu4GoldenCheckSum ++;
    pu4DecCheckSum ++;
    if ((*pu4GoldenCheckSum) != (*pu4DecCheckSum))
    {
        return (FALSE);
    }
    pu4GoldenCheckSum ++;
    pu4DecCheckSum ++;
    return (TRUE);
}

#endif

#ifdef MPV_DUMP_H265_CHKSUM
#define MAX_CHKSUM_NUM 80
UINT32 _u4DumpChksum[2][MAX_CHKSUM_NUM];
void vVDEC_HAL_H265_VDec_ReadCheckSum1(UINT32 u4VDecID)
{
    UINT32  u4Temp, u4Cnt;

    UINT32 *pu4CheckSum = _u4DumpChksum[0];

    u4Temp = 0;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x5f4);
    pu4CheckSum ++;
    u4Temp ++;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x5f8);
    pu4CheckSum ++;
    u4Temp ++;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x608);
    pu4CheckSum ++;
    u4Temp ++;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x60c);
    pu4CheckSum ++;
    u4Temp ++;

    //MC  378~397
    for (u4Cnt = 378; u4Cnt <= 397; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadMC(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    *pu4CheckSum = u4VDecReadVLD(u4VDecID, (44 << 2));
    pu4CheckSum ++;
    u4Temp ++;

    *pu4CheckSum = u4VDecReadVLD(u4VDecID, (45 << 2));
    pu4CheckSum ++;
    u4Temp ++;

    *pu4CheckSum = u4VDecReadVLD(u4VDecID, (46 << 2));
    pu4CheckSum ++;
    u4Temp ++;

    //VLD  58~63
    for (u4Cnt = 58; u4Cnt <= 63; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadVLD(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    *pu4CheckSum = u4VDecReadHEVCVLD(u4VDecID, 0x84);
    pu4CheckSum ++;
    u4Temp ++;

    //HEVC VLD  148~152
    for (u4Cnt = 148; u4Cnt <= 155; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCVLD(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //HEVC VLD  165~179
    for (u4Cnt = 165; u4Cnt <= 179; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCVLD(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //MV  147~151
    for (u4Cnt = 147; u4Cnt <= 151; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //IP  212
    *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (212 << 2));
    pu4CheckSum ++;
    u4Temp ++;

    //IQ  235~239
    for (u4Cnt = 241; u4Cnt <= 245; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //IS  241~245
    for (u4Cnt = 241; u4Cnt <= 245; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    while (u4Temp < MAX_CHKSUM_NUM)
    {
        *pu4CheckSum = 0;
        pu4CheckSum ++;
        u4Temp ++;
    }
}

void vVDEC_HAL_H265_VDec_ReadCheckSum2(UINT32 u4VDecID)
{
    UINT32  u4Temp, u4Cnt;

    UINT32 *pu4CheckSum = _u4DumpChksum[1];

    u4Temp = 0;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x5f4);
    pu4CheckSum ++;
    u4Temp ++;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x5f8);
    pu4CheckSum ++;
    u4Temp ++;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x608);
    pu4CheckSum ++;
    u4Temp ++;
    *pu4CheckSum = u4VDecReadMC(u4VDecID, 0x60c);
    pu4CheckSum ++;
    u4Temp ++;

    //MC  378~397
    for (u4Cnt = 378; u4Cnt <= 397; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadMC(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    *pu4CheckSum = u4VDecReadVLD(u4VDecID, (44 << 2));
    pu4CheckSum ++;
    u4Temp ++;

    *pu4CheckSum = u4VDecReadVLD(u4VDecID, (45 << 2));
    pu4CheckSum ++;
    u4Temp ++;

    *pu4CheckSum = u4VDecReadVLD(u4VDecID, (46 << 2));
    pu4CheckSum ++;
    u4Temp ++;

    //VLD  58~63
    for (u4Cnt = 58; u4Cnt <= 63; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadVLD(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    *pu4CheckSum = u4VDecReadHEVCVLD(u4VDecID, 0x84);
    pu4CheckSum ++;
    u4Temp ++;

    //HEVC VLD  148~152
    for (u4Cnt = 148; u4Cnt <= 155; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCVLD(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //HEVC VLD  165~179
    for (u4Cnt = 165; u4Cnt <= 179; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCVLD(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //MV  147~151
    for (u4Cnt = 147; u4Cnt <= 151; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //IP  212
    *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (212 << 2));
    pu4CheckSum ++;
    u4Temp ++;

    //IQ  235~239
    for (u4Cnt = 241; u4Cnt <= 245; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    //IS  241~245
    for (u4Cnt = 241; u4Cnt <= 245; u4Cnt++)
    {
        *pu4CheckSum = u4VDecReadHEVCMV(u4VDecID, (u4Cnt << 2));
        pu4CheckSum ++;
        u4Temp ++;
    }

    while (u4Temp < MAX_CHKSUM_NUM)
    {
        *pu4CheckSum = 0;
        pu4CheckSum ++;
        u4Temp ++;
    }
}
#endif
