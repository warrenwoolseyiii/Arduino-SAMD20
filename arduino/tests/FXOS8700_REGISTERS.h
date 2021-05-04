/* ========================================================================== *
 *                        FXOS8700 Driver - Registers                         *
 * -------------------------------------------------------------------------- *
 *               Copyright (C) Flume, Inc - All Rights Reserved               *
 *  Unauthorized copying of this file, via any medium is strictly prohibited  *
 *                       Proprietary and confidential.                        *
 *           Written by Jeff Hufford <jeff@flumetech.com>, 2015-2016          *
 * ========================================================================== */
#ifndef FXOS8700_REGISTERS_h
#define FXOS8700_REGISTERS_h

/* Define registers per FXOS8700CQ, Document Number: FXOS8700CQ */
/* General Settings  */
#define F_SETUP 0x09
#define TRIG_CFG 0x0A
#define SYSMOD 0x0B
#define INT_SOURCE 0x0C
#define WHO_AM_I 0x0D

/* Auto-Sleep Count - R/W
 *    [1] Register contents are preserved when transitioning from active-to-
 * standby mode.
 *    [5] Modification of this register’s contents can only occur when device is
 * in standby mode, except the FS[1:0] bit fields in XYZ_DATA_CFG register.
 **/
#define ASLP_COUNT 0x29

/* control reg1
 *    [1] Register contents are preserved when transitioning from active-to-
 * standby mode.
 *    [5] Modification of this register’s contents can only occur when device is
 * in standby mode, except the FS[1:0] bit fields in XYZ_DATA_CFG register.
 */
#define CTRL_REG1 0x2A
enum ctrl_reg1_t
{
    ctrl_reg1_active = 0,
    ctrl_reg1_f_read = 1,
    ctrl_reg1_innoise = 2,
    ctrl_reg1_dr0 = 3,
    ctrl_reg1_dr1 = 4,
    ctrl_reg1_dr2 = 5,
    ctrl_reg1_aslp_rate0 = 6,
    ctrl_reg1_aslp_rate1 = 7
};

#define ODR_800HZ ( 0 )
#define ODR_400HZ ( 1 << ctrl_reg1_dr0 )
#define ODR_200HZ ( 1 << ctrl_reg1_dr1 )
#define ODR_100HZ ( 1 << ctrl_reg1_dr1 ) | ( 1 << ctrl_reg1_dr0 )
#define ODR_50HZ ( 1 << ctrl_reg1_dr2 )
#define ODR_12HZ ( 1 << ctrl_reg1_dr2 ) | ( 1 << ctrl_reg1_dr0 )
#define ODR_6HZ ( 1 << ctrl_reg1_dr2 ) | ( 1 << ctrl_reg1_dr1 )
#define ODR_1HZ \
    ( 1 << ctrl_reg1_dr2 ) | ( 1 << ctrl_reg1_dr1 ) | ( 1 << ctrl_reg1_dr0 )

#define ASLP_RATE_50HZ 0x00
#define ASLP_RATE_12HZ 0x40
#define ASLP_RATE_6HZ 0x80
#define ASLP_RATE_1HZ 0xC0

/* Control Reg 2 (Self-test, reset, OSR, sleep/wake) - R/W
 *    [1] Register contents are preserved when transitioning from active-to-
 * standby mode.
 *    [5] Modification of this register’s contents can only occur when device is
 * in standby mode, except the FS[1:0] bit fields in XYZ_DATA_CFG register.
 **/
#define CTRL_REG2 0x2B
enum ctrl_reg2_t
{
    ctrl_reg2_mods0 = 0,
    ctrl_reg2_mods1 = 1,
    ctrl_reg2_slpe = 2,
    ctrl_reg2_smods0 = 3,
    ctrl_reg2_smods1 = 4,
    // ctrl_reg2_5      = 5,
    ctrl_reg2_rst = 6,
    ctrl_reg2_st = 7
};

/* Control Reg 3 (Sleep mode interrupt wake enable, interrupt polarity,
 * push-pull/open-drain) - R/W
 *    [1] Register contents are preserved when transitioning from active-to-
 * standby mode.
 *    [5] Modification of this register’s contents can only occur when device is
 * in standby mode, except the FS[1:0] bit fields in XYZ_DATA_CFG register.
 **/
/* Interrupt control reg3
 * Page 41
 */
#define CTRL_REG3 0x2C
enum ctrl_reg3_t
{
    ctrl_reg3_pp_od = 0,
    ctrl_reg3_ipol = 1,
    ctrl_reg3_wake_a_vecm = 2,
    ctrl_reg3_wake_ffmt = 3,
    ctrl_reg3_wake_pulse = 4,
    ctrl_reg3_wake_lndprt = 5,
    ctrl_reg3_wake_trans = 6,
    ctrl_reg3_fifo_gate = 7
};

#define CTRL_REG4 0x2D
enum ctrl_reg4_t
{
    ctrl_reg4_en_drdy = 0,
    ctrl_reg4_en_a_vecm = 1,
    ctrl_reg4_en_ffmt = 2,
    ctrl_reg4_en_pulse = 3,
    ctrl_reg4_en_lndprt = 4,
    ctrl_reg4_en_trans = 5,
    ctrl_reg4_en_fifo = 6,
    ctrl_reg4_en_aslp = 7
};

#define CTRL_REG5 0x2E
#define TEMP 0x51

/* Magnetic Data */
#define M_DR_STATUS 0x32
enum mdr_status_t
{
    mdr_status_xdr = 0,
    mdr_status_ydr = 1,
    mdr_status_zdr = 2,
    mdr_status_xyzdr = 3,
    mdr_status_xow = 4,
    mdr_status_yow = 5,
    mdr_status_zow = 6,
    mdr_status_xyzow = 7
};

/* ************************* *
 * General Magnetic Settings *
 * ************************* */

/* Mag CTRL 1 - R/W
 *    [4] Register contents can be modified anytime in standby or active mode.
 * A write to this register will cause a reset of the corresponding internal
 * system debounce counter.
 **/
#define M_CTRL_REG1 0x5B
enum mctrl_reg1_t
{
    mctrl1_hms0 = 0,
    mctrl1_hms1 = 1,
    mctrl1_os0 = 2,
    mctrl1_os1 = 3,
    mctrl1_os2 = 4,
    mctrl1_ost = 5,
    mctrl1_rst = 6,
    mctrl1_acal = 7
};

#define MOSR_0 ( 0 << mctrl1_os0 )
#define MOSR_1 ( 1 << mctrl1_os0 )
#define MOSR_2 ( 2 << mctrl1_os0 )
#define MOSR_3 ( 3 << mctrl1_os0 )
#define MOSR_4 ( 4 << mctrl1_os0 )
#define MOSR_5 ( 5 << mctrl1_os0 )
#define MOSR_6 ( 6 << mctrl1_os0 )
#define MOSR_7 ( 7 << mctrl1_os0 )

/* Mag CTRL 2 - R/W:
 *    [5] Modification of this register’s contents can only occur when device is
 * in standby mode, except the FS[1:0] bit fields in XYZ_DATA_CFG register.
 **/
#define M_CTRL_REG2 0x5C
enum mctrl_reg2_t
{
    mctrl2_rst_cnt0 = 0,
    mctrl2_rst_cnt1 = 1,
    mctrl2_maxmin_rst = 2,
    mctrl2_maxmin_dis_ths = 3,
    mctrl2_maxmin_dis = 4,
    mctrl2_hyb_autoinc_mode = 5
};

/* Mag CTRL 3 - R/W:
 * [4] Register contents can be modified anytime in standby or active mode.
 * A write to this register will cause a reset of the corresponding internal
 * system debounce counter.
 **/
#define M_CTRL_REG3 0x5D
enum mctrl_reg3_t
{
    mctrl3_st_xy0 = 0,
    mctrl3_st_xy1 = 1,
    mctrl3_st_z = 2,
    mctrl3_ths_xyz_update = 3,
    mctrl3_aslp_os0 = 4,
    mctrl3_aslp_os1 = 5,
    mctrl3_aslp_os2 = 6,
    mctrl3_raw = 7
};

#define MSOSR_0 ( 0 << mctrl3_aslp_os0 )
#define MSOSR_1 ( 1 << mctrl3_aslp_os0 )
#define MSOSR_2 ( 2 << mctrl3_aslp_os0 )
#define MSOSR_3 ( 3 << mctrl3_aslp_os0 )
#define MSOSR_4 ( 4 << mctrl3_aslp_os0 )
#define MSOSR_5 ( 5 << mctrl3_aslp_os0 )
#define MSOSR_6 ( 6 << mctrl3_aslp_os0 )
#define MSOSR_7 ( 7 << mctrl3_aslp_os0 )

/* Interrupt Source Reg
 * Page 84
 */
#define M_INT_SRC 0x5E
enum src_m_t
{
    src_m_drdy = 0,
    src_m_vecm = 1,
    src_m_ths = 2
};

/* Magn Vector Config */
#define M_VECM_CFG 0x69
enum mvecm_cfg_t
{
    mvecm_cfg_init_cfg = 0, // 1->vecm int on INT1, 0->int on INT2
    mvecm_cfg_int_en = 1,   // 1->vecm interrupt en, 0->vecm interrupt dis
    mvecm_cfg_wake_en = 2,  // 1->inc vecm for auto-sleep, 0->don't include
    mvecm_cfg_en = 3, // 1->vecm function enabled, 0->vecm function disabled
    mvecm_cfg_updm =
        4, // 1->don't update on event, 0->update M_VECM_X/Y/Z_INIT on event
    mvecm_cfg_initm = 5, // 1->use M_VECM_X/Y/Z_INIT val, 0->use current val
    mvecm_cfg_ele = 6    // 1->event latch en, 0->event latch dis
};

/* Magnetic Data Out */
#define M_OUT_X_MSB 0x33
#define M_OUT_X_LSB 0x34
#define M_OUT_Y_MSB 0x35
#define M_OUT_Y_LSB 0x36
#define M_OUT_Z_MSB 0x37
#define M_OUT_Z_LSB 0x38

/* Magnetic Data Compare */
#define CMP_OUT_X_MSB 0x39
#define CMP_OUT_X_LSB 0x3A
#define CMP_OUT_Y_MSB 0x3B
#define CMP_OUT_Y_LSB 0x3C
#define CMP_OUT_Z_MSB 0x3D
#define CMP_OUT_Z_LSB 0x3E

/* Magnetic Data Offsets */
#define M_OFF_X_MSB 0x3F
#define M_OFF_X_LSB 0x40
#define M_OFF_Y_MSB 0x41
#define M_OFF_Y_LSB 0x42
#define M_OFF_Z_MSB 0x43
#define M_OFF_Z_LSB 0x44

/* Magnetic Data Min/Max */
#define MAX_X_MSB 0x45
#define MAX_X_LSB 0x46
#define MAX_Y_MSB 0x47
#define MAX_Y_LSB 0x48
#define MAX_Z_MSB 0x49
#define MAX_Z_LSB 0x4A
#define MIN_X_MSB 0x4B
#define MIN_X_LSB 0x4C
#define MIN_Y_MSB 0x4D
#define MIN_Y_LSB 0x4E
#define MIN_Z_MSB 0x4F
#define MIN_Z_LSB 0x50

/* Magnetic Threshold Cfg - R/W
 *    [1] Register contents are preserved when transitioning from active-to-
 * standby mode.
 *    [5] Modification of this register’s contents can only occur when device is
 * in standby mode.
 **/
#define M_THS_CFG 0x52
enum m_ths_cfg_t
{
    m_ths_int_cfg = 0,
    m_ths_int_en = 1,
    m_ths_wake_en = 2,
    m_ths_xefe = 3,
    m_ths_yefe = 4,
    m_ths_zefe = 5,
    m_ths_oae = 6,
    m_ths_ele = 7
};

/* Magnetic Threshold Source - R
 *    [1] Register contents are preserved when transitioning from active-to-
 * standby mode.
 *    [2] Register contents are reset when transitioning from standby-to-active
 * mode.
 **/
#define M_THS_SRC 0x53
enum m_ths_src_t
{
    m_ths_xhp = 0,
    m_ths_xhe = 1,
    m_ths_yhp = 2,
    m_ths_yhe = 3,
    m_ths_zhp = 4,
    m_ths_zhe = 5,
    //  m_ths_6    = 6,
    m_ths_ea = 7
};

/* Magnetic Thresholds -
 * The following applies to 0x54-0x59 :
 *    [1] Register contents are preserved when transitioning from active-to-
 * standby mode.
 *    [5] Modification of this register’s contents can only occur when device is
 * in standby mode.
 **/
#define M_THS_X_MSB 0x54
#define M_THS_X_LSB 0x55
#define M_THS_Y_MSB 0x56
#define M_THS_Y_LSB 0x57
#define M_THS_Z_MSB 0x58
#define M_THS_Z_LSB 0x59

/* Mag THS_COUNT - R/W:
 *    [1] Register contents are preserved when transitioning from active-to-
 * standby mode.
 *    [3] Hybrid auto-increment mode may be used to read out acceleration and
 * magnetic data from registers x1-x6 using a burst-read transaction. When
 * M_CTRL_REG2[hyb_autoinc_mode] = 1, the user may do a burst read of 12 bytes
 * starting from OUT_X_MSB (address 0x1) to read out both the current
 * accelerometer and magnetometer data in one contiguous operation.
 *    [5] Modification of this register’s contents can only occur when device is
 * in standby mode, except the FS[1:0] bit fields in XYZ_DATA_CFG register.
 **/
#define M_THS_COUNT 0x5A

/* Vect Magn Registers */
#define M_VECM_THS_MSB 0x6A
#define M_VECM_THS_LSB 0x6B
#define M_VECM_CNT 0x6C
#define M_VECM_INITX_MSB 0x6D
#define M_VECM_INITX_LSB 0x6E
#define M_VECM_INITY_MSB 0x6F
#define M_VECM_INITY_LSB 0x70
#define M_VECM_INITZ_MSB 0x71
#define M_VECM_INITZ_LSB 0x72

#endif

/* End FXOS8700_REGISTERS_h */
