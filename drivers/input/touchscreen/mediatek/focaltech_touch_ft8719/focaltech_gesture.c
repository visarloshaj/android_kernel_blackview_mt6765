/*
 *
 * FocalTech TouchScreen driver.
 *
 * Copyright (c) 2012-2018, Focaltech Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*****************************************************************************
*
* File Name: focaltech_gestrue.c
*
* Author: Focaltech Driver Team
*
* Created: 2016-08-08
*
* Abstract:
*
* Reference:
*
*****************************************************************************/

/*****************************************************************************
* 1.Included header files
*****************************************************************************/
#include "focaltech_core.h"
#if FTS_GESTURE_EN
/******************************************************************************
* Private constant and macro definitions using #define
*****************************************************************************/
#define KEY_GESTURE_U                           KEY_U
#define KEY_GESTURE_UP                          KEY_UP
#define KEY_GESTURE_DOWN                        KEY_DOWN
#define KEY_GESTURE_LEFT                        KEY_LEFT
#define KEY_GESTURE_RIGHT                       KEY_RIGHT
#define KEY_GESTURE_O                           KEY_O
#define KEY_GESTURE_E                           KEY_E
#define KEY_GESTURE_M                           KEY_M
#define KEY_GESTURE_L                           KEY_L
#define KEY_GESTURE_W                           KEY_W
#define KEY_GESTURE_S                           KEY_S
#define KEY_GESTURE_V                           KEY_V
#define KEY_GESTURE_C                           KEY_C
#define KEY_GESTURE_Z                           KEY_Z

#define GESTURE_LEFT                            0x20
#define GESTURE_RIGHT                           0x21
#define GESTURE_UP                              0x22
#define GESTURE_DOWN                            0x23
#define GESTURE_DOUBLECLICK                     0x24
#define GESTURE_O                               0x30
#define GESTURE_W                               0x31
#define GESTURE_M                               0x32
#define GESTURE_E                               0x33
#define GESTURE_L                               0x44
#define GESTURE_S                               0x46
#define GESTURE_V                               0x54
#define GESTURE_Z                               0x65
#define GESTURE_C                               0x34
#define FTS_GESTRUE_POINTS                      255
#define FTS_GESTRUE_POINTS_HEADER               8
extern struct fts_ts_data *fts_data;

/*****************************************************************************
* Private enumerations, structures and unions using typedef
*****************************************************************************/
/*
* header        -   byte0:gesture id
*                   byte1:pointnum
*                   byte2~7:reserved
* coordinate_x  -   All gesture point x coordinate
* coordinate_y  -   All gesture point y coordinate
* mode          -   1:enable gesture function(default)
*               -   0:disable
* active        -   1:enter into gesture(suspend)
*                   0:gesture disable or resume
*/
struct fts_gesture_st {
    u8 header[FTS_GESTRUE_POINTS_HEADER];
    u16 coordinate_x[FTS_GESTRUE_POINTS];
    u16 coordinate_y[FTS_GESTRUE_POINTS];
    u8 mode;   /*host driver enable gesture flag*/
    u8 active;  /*gesture actutally work*/
};

/*****************************************************************************
* Static variables
*****************************************************************************/
static struct fts_gesture_st fts_gesture_data;

/*****************************************************************************
* Global variable or extern global variabls/functions
*****************************************************************************/
//extern bool proximity_check_status(void);
int g_pmode_tp_en(struct fts_ts_data *ts_data, int mode);
bool pmode_tp_flag;
EXPORT_SYMBOL(pmode_tp_flag);

/*****************************************************************************
* Static function prototypes
*****************************************************************************/
//static ssize_t fts_gesture_show(struct device *dev, struct device_attribute *attr, char *buf);
//static ssize_t fts_gesture_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t fts_gesture_buf_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t fts_gesture_buf_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t double_tap_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t double_tap_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static ssize_t swipe_right_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t swipe_right_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t swipe_left_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t swipe_left_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t swipe_up_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t swipe_up_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t swipe_down_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t swipe_down_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static ssize_t letter_o_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t letter_o_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t letter_e_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t letter_e_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t letter_m_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t letter_m_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t letter_w_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t letter_w_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t letter_l_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t letter_l_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t letter_s_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t letter_s_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t letter_v_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t letter_v_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t letter_c_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t letter_c_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t letter_z_switch_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t letter_z_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static DEVICE_ATTR (fts_gesture_buf, S_IRUGO | S_IWUSR, fts_gesture_buf_show, fts_gesture_buf_store);
static DEVICE_ATTR (double_tap_switch, S_IRUGO|S_IWUSR, double_tap_switch_show, double_tap_switch_store);

static DEVICE_ATTR (swipe_right_switch, S_IRUGO|S_IWUSR, swipe_right_switch_show, swipe_right_switch_store);
static DEVICE_ATTR (swipe_left_switch, S_IRUGO|S_IWUSR, swipe_left_switch_show, swipe_left_switch_store);
static DEVICE_ATTR (swipe_up_switch, S_IRUGO|S_IWUSR, swipe_up_switch_show, swipe_up_switch_store);
static DEVICE_ATTR (swipe_down_switch, S_IRUGO|S_IWUSR, swipe_down_switch_show, swipe_down_switch_store);

static DEVICE_ATTR (letter_o_switch, S_IRUGO|S_IWUSR, letter_o_switch_show, letter_o_switch_store);
static DEVICE_ATTR (letter_e_switch, S_IRUGO|S_IWUSR, letter_e_switch_show, letter_e_switch_store);
static DEVICE_ATTR (letter_m_switch, S_IRUGO|S_IWUSR, letter_m_switch_show, letter_m_switch_store);
static DEVICE_ATTR (letter_w_switch, S_IRUGO|S_IWUSR, letter_w_switch_show, letter_w_switch_store);
static DEVICE_ATTR (letter_l_switch, S_IRUGO|S_IWUSR, letter_l_switch_show, letter_l_switch_store);
static DEVICE_ATTR (letter_s_switch, S_IRUGO|S_IWUSR, letter_s_switch_show, letter_s_switch_store);
static DEVICE_ATTR (letter_v_switch, S_IRUGO|S_IWUSR, letter_v_switch_show, letter_v_switch_store);
static DEVICE_ATTR (letter_c_switch, S_IRUGO|S_IWUSR, letter_c_switch_show, letter_c_switch_store);
static DEVICE_ATTR (letter_z_switch, S_IRUGO|S_IWUSR, letter_z_switch_show, letter_z_switch_store);

static struct attribute *fts_gesture_mode_attrs[] = {
    &dev_attr_fts_gesture_buf.attr,
    &dev_attr_double_tap_switch.attr,
    &dev_attr_swipe_right_switch.attr,
    &dev_attr_swipe_left_switch.attr,
    &dev_attr_swipe_up_switch.attr,
    &dev_attr_swipe_down_switch.attr,
    &dev_attr_letter_o_switch.attr,
    &dev_attr_letter_e_switch.attr,
    &dev_attr_letter_m_switch.attr,
    &dev_attr_letter_w_switch.attr,
    &dev_attr_letter_l_switch.attr,
    &dev_attr_letter_s_switch.attr,
    &dev_attr_letter_v_switch.attr,
    &dev_attr_letter_c_switch.attr,
    &dev_attr_letter_z_switch.attr,
    NULL,
};

static struct attribute_group fts_gesture_group = {
    .attrs = fts_gesture_mode_attrs,
};

/************************************************************************
* Name: fts_gesture_show
*  Brief:
*  Input: device, device attribute, char buf
* Output:
* Return:
***********************************************************************/
/*static ssize_t fts_gesture_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int count;
    u8 val;
    struct input_dev *input_dev = fts_data->input_dev;

    mutex_lock(&input_dev->mutex);
    fts_read_reg(FTS_REG_GESTURE_EN, &val);
    count = snprintf(buf, PAGE_SIZE, "Gesture Mode: %s\n", fts_gesture_data.mode ? "On" : "Off");
    count += snprintf(buf + count, PAGE_SIZE, "Reg(0xD0) = %d\n", val);
    mutex_unlock(&input_dev->mutex);

    return count;
}*/

/************************************************************************
* Name: fts_gesture_store
*  Brief:
*  Input: device, device attribute, char buf, char count
* Output:
* Return:
***********************************************************************/
/*static ssize_t fts_gesture_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct input_dev *input_dev = fts_data->input_dev;
    mutex_lock(&input_dev->mutex);
    if (FTS_SYSFS_ECHO_ON(buf)) {
        FTS_INFO("[GESTURE]enable gesture");
        fts_gesture_data.mode = ENABLE;
    } else if (FTS_SYSFS_ECHO_OFF(buf)) {
        FTS_INFO("[GESTURE]disable gesture");
        fts_gesture_data.mode = DISABLE;
    }
    mutex_unlock(&input_dev->mutex);

    return count;
}*/
/************************************************************************
* Name: fts_gesture_buf_show
*  Brief:
*  Input: device, device attribute, char buf
* Output:
* Return:
***********************************************************************/
static ssize_t fts_gesture_buf_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int count;
    int i = 0;
    struct input_dev *input_dev = fts_data->input_dev;

    mutex_lock(&input_dev->mutex);
    count = snprintf(buf, PAGE_SIZE, "Gesture ID: 0x%x\n", fts_gesture_data.header[0]);
    count += snprintf(buf + count, PAGE_SIZE, "Gesture PointNum: %d\n", fts_gesture_data.header[1]);
    count += snprintf(buf + count, PAGE_SIZE, "Gesture Point Buf:\n");
    for (i = 0; i < fts_gesture_data.header[1]; i++) {
        count += snprintf(buf + count, PAGE_SIZE, "%3d(%4d,%4d) ", i, fts_gesture_data.coordinate_x[i], fts_gesture_data.coordinate_y[i]);
        if ((i + 1) % 4 == 0)
            count += snprintf(buf + count, PAGE_SIZE, "\n");
    }
    count += snprintf(buf + count, PAGE_SIZE, "\n");
    mutex_unlock(&input_dev->mutex);

    return count;
}

/************************************************************************
* Name: fts_gesture_buf_store
*  Brief:
*  Input: device, device attribute, char buf, char count
* Output:
* Return:
***********************************************************************/
static ssize_t fts_gesture_buf_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    /* place holder for future use */
    return -EPERM;
}

/*****************************************************************************
*   Name: fts_create_gesture_sysfs
*  Brief:
*  Input:
* Output:
* Return: 0-success or others-error
*****************************************************************************/
int fts_create_gesture_sysfs(struct device *dev)
{
    int ret = 0;

    ret = sysfs_create_group(&dev->kobj, &fts_gesture_group);
    if ( ret != 0) {
        FTS_ERROR( "[GESTURE]fts_gesture_mode_group(sysfs) create failed!");
        sysfs_remove_group(&dev->kobj, &fts_gesture_group);
        return ret;
    }
    return 0;
}

/*****************************************************************************
*   Name: fts_gesture_report
*  Brief:
*  Input:
* Output:
* Return:
*****************************************************************************/
static void fts_gesture_report(struct input_dev *input_dev, int gesture_id)
{
    int gesture = 0;
    bool is_enabled = 0;

    FTS_FUNC_ENTER();
    printk("[Focal][Touch] %s :  gesture_id = 0x%x\n ", __func__, gesture_id);

    switch (gesture_id) {
        case GESTURE_LEFT:
            gesture = KEY_GESTURE_LEFT;
            is_enabled = fts_data->swipe_left_enable;
            break;
        case GESTURE_RIGHT:
            gesture = KEY_GESTURE_RIGHT;
            is_enabled = fts_data->swipe_right_enable;
            break;
        case GESTURE_UP:
            gesture = KEY_GESTURE_UP;
            is_enabled = fts_data->swipe_up_enable;
            break;
        case GESTURE_DOWN:
            gesture = KEY_GESTURE_DOWN;
            is_enabled = fts_data->swipe_down_enable;
            break;
        case GESTURE_DOUBLECLICK:
            gesture = KEY_WAKEUP;
            is_enabled = fts_data->double_tap_enable;
            break;
        case GESTURE_O:
            gesture = KEY_GESTURE_O;
            is_enabled = fts_data->letter_o_enable;
            break;
        case GESTURE_W:
            gesture = KEY_GESTURE_W;
            is_enabled = fts_data->letter_w_enable;
            break;
        case GESTURE_M:
            gesture = KEY_GESTURE_M;
            is_enabled = fts_data->letter_m_enable;
            break;
        case GESTURE_E:
            gesture = KEY_GESTURE_E;
            is_enabled = fts_data->letter_e_enable;
            break;
        case GESTURE_L:
            gesture = KEY_GESTURE_L;
            is_enabled = fts_data->letter_l_enable;
            break;
        case GESTURE_S:
            gesture = KEY_GESTURE_S;
            is_enabled = fts_data->letter_s_enable;
            break;
        case GESTURE_V:
            gesture = KEY_GESTURE_V;
            is_enabled = fts_data->letter_v_enable;
            break;
        case GESTURE_Z:
            gesture = KEY_GESTURE_Z;
            is_enabled = fts_data->letter_z_enable;
            break;
        case GESTURE_C:
            gesture = KEY_GESTURE_C;
            is_enabled = fts_data->letter_c_enable;
            break;
        default:
            gesture = -1;
            break;
    }

    /* report event key */
    if (gesture != -1 && is_enabled) {
        printk("Gesture Code=%d", gesture);
        input_report_key(input_dev, gesture, 1);
        input_sync(input_dev);
        input_report_key(input_dev, gesture, 0);
        input_sync(input_dev);
    }
    
    FTS_FUNC_EXIT();
}

/************************************************************************
*   Name: fts_gesture_read_buffer
*  Brief: read data from TP register
*  Input:
* Output:
* Return: fail <0
***********************************************************************/
static int fts_gesture_read_buffer(u8 *buf, int read_bytes)
{
    int remain_bytes;
    int ret;
    int i;

    if (read_bytes <= 256) {
        ret = fts_read(buf, 1, buf, read_bytes);
    } else {
        ret = fts_read(buf, 1, buf, 256);
        remain_bytes = read_bytes - 256;
        for (i = 1; remain_bytes > 0; i++) {
            if (remain_bytes <= 256)
                ret = fts_read(buf, 0, buf + 256 * i, remain_bytes);
            else
                ret = fts_read(buf, 0, buf + 256 * i, 256);
            remain_bytes -= 256;
        }
    }

    return ret;
}

/************************************************************************
*   Name: fts_gesture_readdata
*  Brief: read data from TP register
*  Input:
* Output:
* Return: return 0 if succuss, otherwise reture error code
***********************************************************************/
int fts_gesture_readdata(struct fts_ts_data *ts_data, u8 *data)
{
    u8 buf[FTS_GESTRUE_POINTS * 4] = { 0 };
    int ret = 0;
    int i = 0;
    int gestrue_id = 0;
    int read_bytes = 0;
    int pointnumint = 0;
    u8 pointnum = 0;
    u8 state = 0;
    struct input_dev *input_dev = ts_data->input_dev;

    if (!ts_data->suspended) {
        return -EINVAL;
    }

    ret = fts_read_reg(FTS_REG_GESTURE_EN, &state);
    if ((ret < 0) || (state != ENABLE)) {
        FTS_DEBUG("gesture not enable, don't process gesture");
        return -EIO;
    }

    /* init variable before read gesture point */
    memset(fts_gesture_data.header, 0, FTS_GESTRUE_POINTS_HEADER);
    memset(fts_gesture_data.coordinate_x, 0, FTS_GESTRUE_POINTS * sizeof(u16));
    memset(fts_gesture_data.coordinate_y, 0, FTS_GESTRUE_POINTS * sizeof(u16));
    memset(buf, 0, (FTS_GESTRUE_POINTS * 4) * sizeof(u8));

    buf[0] = FTS_REG_GESTURE_OUTPUT_ADDRESS;
    ret = fts_read(buf, 1, buf, FTS_GESTRUE_POINTS_HEADER);
    if (ret < 0) {
        FTS_ERROR("[GESTURE]Read gesture header data failed!!");
        FTS_FUNC_EXIT();
        return ret;
    }

    memcpy(fts_gesture_data.header, buf, FTS_GESTRUE_POINTS_HEADER);
    gestrue_id = buf[0];
    pointnum = buf[1];
    pointnumint = ((int)pointnum);
    if (pointnumint > 7) {
        pointnumint = 7;
    }
    read_bytes = (pointnumint) * 4 + 2;
    memset(buf, 0, (FTS_GESTRUE_POINTS * 4) * sizeof(u8));
    buf[0] = FTS_REG_GESTURE_OUTPUT_ADDRESS;
    printk("[GESTURE]PointNum=%d", pointnumint);
    ret = fts_gesture_read_buffer(buf, read_bytes);
    if (ret < 0) {
        FTS_ERROR("[GESTURE]Read gesture touch data failed!!");
        FTS_FUNC_EXIT();
        return ret;
    }

    fts_gesture_report(input_dev, gestrue_id);
    for (i = 0; i < pointnumint; i++) {
        fts_gesture_data.coordinate_x[i] = (((s16) buf[0 + (4 * i + 2)]) & 0x0F) << 8
                                           | (((s16) buf[1 + (4 * i + 2)]) & 0xFF);
        fts_gesture_data.coordinate_y[i] = (((s16) buf[2 + (4 * i + 2)]) & 0x0F) << 8
                                           | (((s16) buf[3 + (4 * i + 2)]) & 0xFF);
    }

    return 0;
}

static ssize_t double_tap_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int tmp = 0;
	tmp = buf[0] - 48;

	if (tmp == 0)
		fts_data->double_tap_enable = 0;
	else if (tmp == 1)
		fts_data->double_tap_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->double_tap_enable);
	return count;
}

static ssize_t double_tap_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d \n", fts_data->double_tap_enable);
}

static ssize_t swipe_right_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->swipe_right_enable = 0;
    else if (tmp == 1)
        fts_data->swipe_right_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->swipe_right_enable);
    return count;
}

static ssize_t swipe_right_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->swipe_right_enable);
}

static ssize_t swipe_left_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->swipe_left_enable = 0;
    else if (tmp == 1)
        fts_data->swipe_left_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->swipe_left_enable);
    return count;
}

static ssize_t swipe_left_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->swipe_left_enable);
}

static ssize_t swipe_up_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->swipe_up_enable = 0;
    else if (tmp == 1)
        fts_data->swipe_up_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->swipe_up_enable);
    return count;
}

static ssize_t swipe_up_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->swipe_up_enable);
}

static ssize_t swipe_down_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->swipe_down_enable = 0;
    else if (tmp == 1)
        fts_data->swipe_down_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->swipe_down_enable);
    return count;
}

static ssize_t swipe_down_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->swipe_down_enable);
}

static ssize_t letter_o_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->letter_o_enable = 0;
    else if (tmp == 1)
        fts_data->letter_o_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->letter_o_enable);
    return count;
}

static ssize_t letter_o_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->letter_o_enable);
}

static ssize_t letter_e_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->letter_e_enable = 0;
    else if (tmp == 1)
        fts_data->letter_e_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->letter_e_enable);
    return count;
}

static ssize_t letter_e_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->letter_e_enable);
}

static ssize_t letter_m_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->letter_m_enable = 0;
    else if (tmp == 1)
        fts_data->letter_m_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->letter_m_enable);
    return count;
}

static ssize_t letter_m_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->letter_m_enable);
}

static ssize_t letter_w_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->letter_w_enable = 0;
    else if (tmp == 1)
        fts_data->letter_w_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->letter_w_enable);
    return count;
}

static ssize_t letter_w_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->letter_w_enable);
}

static ssize_t letter_l_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->letter_l_enable = 0;
    else if (tmp == 1)
        fts_data->letter_l_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->letter_l_enable);
    return count;
}

static ssize_t letter_l_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->letter_l_enable);
}

static ssize_t letter_s_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->letter_s_enable = 0;
    else if (tmp == 1)
        fts_data->letter_s_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->letter_s_enable);
    return count;
}

static ssize_t letter_s_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->letter_s_enable);
}

static ssize_t letter_v_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->letter_v_enable = 0;
    else if (tmp == 1)
        fts_data->letter_v_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->letter_v_enable);
    return count;
}

static ssize_t letter_v_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->letter_v_enable);
}

static ssize_t letter_c_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->letter_c_enable = 0;
    else if (tmp == 1)
        fts_data->letter_c_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->letter_c_enable);
    return count;
}

static ssize_t letter_c_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->letter_c_enable);
}

static ssize_t letter_z_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int tmp = 0;
    tmp = buf[0] - 48;
    
    if (tmp == 0)
        fts_data->letter_z_enable = 0;
    else if (tmp == 1)
        fts_data->letter_z_enable = 1;
    printk("[Focal][Touch] %s: %d ! \n", __func__, fts_data->letter_z_enable);
    return count;
}

static ssize_t letter_z_switch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d \n", fts_data->letter_z_enable);
}

/*****************************************************************************
*   Name: fts_gesture_recovery
*  Brief: recovery gesture state when reset or power on
*  Input:
* Output:
* Return:
*****************************************************************************/
void fts_gesture_recovery(struct fts_ts_data *ts_data)
{
    if ((ENABLE == fts_gesture_data.mode) && (ENABLE == fts_gesture_data.active)) {
        FTS_INFO("enter fts_gesture_recovery");
        fts_write_reg(0xD1, 0xff);
        fts_write_reg(0xD2, 0xff);
        fts_write_reg(0xD5, 0xff);
        fts_write_reg(0xD6, 0xff);
        fts_write_reg(0xD7, 0xff);
        fts_write_reg(0xD8, 0xff);
        fts_write_reg(FTS_REG_GESTURE_EN, ENABLE);
    }
}

/*****************************************************************************
*   Name: fts_gesture_suspend
*  Brief:
*  Input:
* Output:
* Return: return 0 if succuss, otherwise return error code
*****************************************************************************/
int fts_gesture_suspend(struct fts_ts_data *ts_data)
{
    int ret;
    int i;
    u8 state;

    printk("[Focal][Touch] %s : gesture suspend\n", __func__);
    //FTS_INFO("gesture suspend...");
    /* gesture not enable, return immediately */
    if (fts_gesture_data.mode == DISABLE) {
        FTS_INFO("gesture is disabled");
        return -EINVAL;
    }
    fts_gesture_data.mode = ENABLE;
    fts_write_reg(FTS_REG_GESTURE_EN, ENABLE);

	for (i = 0; i < 5; i++) {
		state = fts_write_reg(0xd1, 0xFF);
		if (state < 0) {
			msleep(1);
			pr_err("[FTS][tocuh] write d1 fail %d times \n",i);
		} else
			break;
	}

	for (i = 0; i < 5; i++) {
		state = fts_write_reg(0xd2, 0xFF);
		if (state < 0) {
			msleep(1);
			pr_err("[FTS][tocuh] write d2 fail %d times \n",i);
		} else
			break;
	}

	for (i = 0; i < 5; i++) {
   		state = fts_write_reg(0xd5, 0xFF);
		if (state < 0) {
			msleep(1);
			pr_err("[FTS][tocuh] write d5 fail %d times \n",i);
		} else
			break;
	}

	for (i = 0; i < 5; i++) {
   		state = fts_write_reg(0xd6, 0xFF);
		if (state < 0) {
			msleep(1);
			pr_err("[FTS][tocuh] write d6 fail %d times \n",i);
		} else
			break;
	}

	for (i = 0; i < 5; i++) {
   		state = fts_write_reg(0xd7, 0xFF);
		if (state < 0) {
			msleep(1);
			pr_err("[FTS][tocuh] write d7 fail %d times \n",i);
		} else
			break;
	}

    for (i = 0; i < 5; i++) {
        state = fts_write_reg(0xd8, 0xFF);
        if (state < 0) {
            msleep(1);
            pr_err("[FTS][tocuh] write d8 fail %d times \n",i);
        } else
            break;
    }

    ret = enable_irq_wake(fts_data->irq);
    if (ret) {
        FTS_INFO("enable_irq_wake(irq:%d) failed", fts_data->irq);
    }

    fts_gesture_data.active = ENABLE;
    FTS_INFO("[GESTURE]Enter into gesture(suspend) successfully!");
    FTS_FUNC_EXIT();
    return 0;
}

/*****************************************************************************
*   Name: fts_gesture_resume
*  Brief:
*  Input:
* Output:
* Return: return 0 if succuss, otherwise return error code
*****************************************************************************/
int fts_gesture_resume(struct fts_ts_data *ts_data)
{
    int ret;
    int i;
    u8 state = 0;

    FTS_INFO("check gesture resume");
    /* gesture not enable, return immediately */
    if (fts_gesture_data.mode == DISABLE) {
        FTS_DEBUG("gesture is disabled");
        return -EINVAL;
    }

    if (fts_gesture_data.active == DISABLE) {
	printk("[FTS][touch] %s: no gesture mode in suspend, no need running fts_gesture_resume\n", __func__);
        FTS_DEBUG("gesture in suspend is failed, no running fts_gesture_resume");
        return -EINVAL;
    }

    fts_gesture_data.active = DISABLE;
    for (i = 0; i < 5; i++) {
        fts_write_reg(FTS_REG_GESTURE_EN, DISABLE);
        msleep(1);
        fts_read_reg(FTS_REG_GESTURE_EN, &state);
        if (state == DISABLE)
            break;
    }

    if (i >= 5) {
        FTS_ERROR("[GESTURE]Clear gesture(resume) failed!\n");
        return -EIO;
    }

    ret = disable_irq_wake(fts_data->irq);
    if (ret) {
        FTS_INFO("disable_irq_wake(irq:%d) failed", fts_data->irq);
    }

    FTS_INFO("[GESTURE]resume from gesture successfully!");
    FTS_FUNC_EXIT();
    return 0;
}

/*****************************************************************************
*   Name: fts_gesture_init
*  Brief:
*  Input:
* Output:
* Return:
*****************************************************************************/
int fts_gesture_init(struct fts_ts_data *ts_data)
{
    struct input_dev *input_dev = ts_data->input_dev;

    FTS_FUNC_ENTER();
    input_set_capability(input_dev, EV_KEY, KEY_WAKEUP);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_UP);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_DOWN);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_LEFT);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_RIGHT);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_O);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_E);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_M);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_L);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_W);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_S);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_V);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_Z);
    input_set_capability(input_dev, EV_KEY, KEY_GESTURE_C);

    __set_bit(KEY_WAKEUP, input_dev->keybit);
    __set_bit(KEY_GESTURE_RIGHT, input_dev->keybit);
    __set_bit(KEY_GESTURE_LEFT, input_dev->keybit);
    __set_bit(KEY_GESTURE_UP, input_dev->keybit);
    __set_bit(KEY_GESTURE_DOWN, input_dev->keybit);
    __set_bit(KEY_GESTURE_O, input_dev->keybit);
    __set_bit(KEY_GESTURE_E, input_dev->keybit);
    __set_bit(KEY_GESTURE_M, input_dev->keybit);
    __set_bit(KEY_GESTURE_W, input_dev->keybit);
    __set_bit(KEY_GESTURE_L, input_dev->keybit);
    __set_bit(KEY_GESTURE_S, input_dev->keybit);
    __set_bit(KEY_GESTURE_V, input_dev->keybit);
    __set_bit(KEY_GESTURE_C, input_dev->keybit);
    __set_bit(KEY_GESTURE_Z, input_dev->keybit);

    fts_create_gesture_sysfs(ts_data->dev);
    fts_gesture_data.mode = ENABLE;
    fts_gesture_data.active = DISABLE;

    FTS_FUNC_EXIT();
    return 0;
}

/************************************************************************
*   Name: fts_gesture_exit
*  Brief: call when driver removed
*  Input:
* Output:
* Return:
***********************************************************************/
int fts_gesture_exit(struct fts_ts_data *ts_data)
{
    FTS_FUNC_ENTER();
    sysfs_remove_group(&ts_data->dev->kobj, &fts_gesture_group);
    FTS_FUNC_EXIT();
    return 0;
}
#endif
