#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/irqreturn.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/version.h>
#include <linux/wait.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/signal.h>
#include <linux/ctype.h>
#include <linux/wakelock.h>
#include <linux/kobject.h>
#include <linux/poll.h>
#include <net/sock.h>
#include <linux/delay.h>

#include <linux/kernel.h>

#ifndef MTK_ANDROID_L
#include <linux/interrupt.h>
#include <linux/gpio.h>
#else
#include <mach/eint.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#if defined(CONFIG_FB)
#include <linux/notifier.h>
#include <linux/fb.h>
#elif defined(CONFIG_HAS_EARLYSUSPEND)
#include <linux/earlysuspend.h>
#endif

#include "bf_fp_platform.h"

#define BF_IOCTL_MAGIC_NO                   0xFC
#define BF_IOCTL_INIT_ARGS                  _IOWR(BF_IOCTL_MAGIC_NO, 0,uint32_t)
#define BF_IOCTL_REGISTER_READ_WRITE        _IOWR(BF_IOCTL_MAGIC_NO,  1, uint32_t)
#define BF_IOCTL_RESET                      _IO(BF_IOCTL_MAGIC_NO,  2)
#define BF_IOCTL_DISABLE_INTERRUPT          _IO(BF_IOCTL_MAGIC_NO,  3)
#define BF_IOCTL_GAIN_ADJUST                _IOWR(BF_IOCTL_MAGIC_NO, 4,uint32_t)
#define BF_IOCTL_ENABLE_POWER               _IO(BF_IOCTL_MAGIC_NO, 5)
#define BF_IOCTL_DISABLE_POWER              _IO(BF_IOCTL_MAGIC_NO, 6)
#define BF_IOCTL_ENABLE_SPI_CLOCK           _IOW(BF_IOCTL_MAGIC_NO,  7,uint32_t)
#define BF_IOCTL_DISABLE_SPI_CLOCK          _IOW(BF_IOCTL_MAGIC_NO,  8,uint32_t)
#define BF_IOCTL_GET_ID                     _IOWR(BF_IOCTL_MAGIC_NO, 9, uint32_t)
#define BF_IOCTL_INIT_DEVICE                _IOW(BF_IOCTL_MAGIC_NO,  10,uint32_t)
#define BF_IOCTL_REMOVE_DEVICE              _IOW(BF_IOCTL_MAGIC_NO,  11,uint32_t)
#define BF_IOCTL_INPUT_KEY                  _IOW(BF_IOCTL_MAGIC_NO,  12,uint32_t)
#define BF_IOCTL_ENBACKLIGHT                _IOW(BF_IOCTL_MAGIC_NO,  13,uint32_t)
#define BF_IOCTL_ISBACKLIGHT                _IOWR(BF_IOCTL_MAGIC_NO, 14,uint32_t)
#define BF_IOCTL_DISPALY_STATUS             _IOW(BF_IOCTL_MAGIC_NO,  15,uint32_t)
#define BF_IOCTL_SET_PID                    _IOW(BF_IOCTL_MAGIC_NO,  16,uint32_t)
#define BF_IOCTL_INPUT_KEY_DOWN             _IOW(BF_IOCTL_MAGIC_NO,  17,uint32_t)
#define BF_IOCTL_INPUT_KEY_UP               _IOW(BF_IOCTL_MAGIC_NO,  18,uint32_t)
#define BF_IOCTL_LOW_RESET                  _IO(BF_IOCTL_MAGIC_NO,  19)
#define BF_IOCTL_HIGH_RESET                 _IO(BF_IOCTL_MAGIC_NO,  20)
#define BF_IOCTL_NETLINK_INIT               _IOW(BF_IOCTL_MAGIC_NO,  21,uint32_t)
#define BF_IOCTL_NETLINK_PORT               _IOWR(BF_IOCTL_MAGIC_NO, 22,uint32_t)
#define BF_IOCTL_ENABLE_INTERRUPT           _IO(BF_IOCTL_MAGIC_NO,  23)
#define BF_IOCTL_RESET_FLAG                 _IOW(BF_IOCTL_MAGIC_NO,  24,uint32_t)
#define BF_IOCTL_IS_OPT_POWER_ON2V8       	_IOWR(BF_IOCTL_MAGIC_NO,  25, uint32_t)
typedef enum bf_key {
    BF_KEY_NONE = 0,
    BF_KEY_POWER,
    BF_KEY_CAMERA,
    BF_KEY_UP,
    BF_KEY_DOWN,
    BF_KEY_RIGHT,
    BF_KEY_LEFT,
    BF_KEY_HOME,
    BF_KEY_F10,
    BF_KEY_F11
} bf_key_t;

/* for netlink use */
static int g_pid;
static int g_netlink_port = NETLINK_BF;
struct bf_device *g_bf_dev = NULL;
static struct input_dev *bf_inputdev = NULL;
static uint32_t bf_key_need_report = 0;
static struct wake_lock fp_suspend_lock;
static struct wake_lock hw_reset_lock;
static struct kobject *bf_kobj = NULL;
static DEFINE_MUTEX(irq_count_lock);
static irqreturn_t bf_eint_handler (int irq, void *data);
static int bf_init_dts_and_irq(struct bf_device *bf_dev);




#ifdef MTK_ANDROID_L
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
/*
#define GPIO_FP_INT_PIN             (GPIO2 | 0x80000000)//GPIO_FINGER_PRINT_EINT//(GPIO60 | 0x80000000)
#define CUST_EINT_FP_INT_NUM        2//CUST_EINT_FINGER_NUM
#define GPIO_FP_INT_PIN_M_EINT      GPIO_MODE_00
#define GPIO_FINGER_PRINT_RESET     (GPIO3 | 0x80000000)
*/
#define GPIO_FP_INT_PIN             (GPIO18 )//GPIO_FINGER_PRINT_EINT//(GPIO60 | 0x80000000)
#define CUST_EINT_FP_INT_NUM        2//CUST_EINT_FINGER_NUM
#define GPIO_FP_INT_PIN_M_EINT      GPIO_MODE_06
#define GPIO_FINGER_PRINT_RESET     (GPIO19)

extern void mt_eint_ack(unsigned int eint_num);
extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);




static void disable_irq_nosync(unsigned int irq)
{
    mt_eint_mask(irq);
}
static void enable_irq(unsigned int irq)
{
    mt_eint_unmask(irq);
}
static void enable_irq_wake(unsigned int irq)
{
}
static inline bool gpio_is_valid(int number)
{
    return number >= 0;
}

static inline int gpio_get_value(unsigned int gpio)
{
    return mt_get_gpio_in(gpio | 0x80000000);
}

static inline int gpio_direction_input(unsigned gpio)
{
    return     mt_set_gpio_dir((gpio | 0x80000000), GPIO_DIR_IN);
}

//mt_set_gpio_out(GPIO_FINGER_POWER_3V3_EN_PIN, GPIO_OUT_ONE);
static inline int gpio_direction_output(unsigned gpio, int value)
{
    mt_set_gpio_dir((gpio | 0x80000000), GPIO_DIR_OUT);
    return mt_set_gpio_out((gpio | 0x80000000), value);
}

static inline int gpio_request(unsigned gpio, const char *label)
{
    return mt_set_gpio_mode((gpio | 0x80000000), GPIO_MODE_00);
}

static inline int gpio_to_irq(unsigned int gpio)
{
    return gpio;
}

//??????????????????
static void bf_eint_handler_l(void)
{
    disable_irq_nosync(g_bf_dev->irq_num);
    BF_LOG("\n");
    queue_work(g_bf_dev->fingerprint_workqueue, &(g_bf_dev->fingerprint_work));
}

void work_func(struct work_struct *p_work)
{
    BF_LOG("  ++\n");
    bf_eint_handler(g_bf_dev->irq_num, g_bf_dev);
    enable_irq(g_bf_dev->irq_num);
    BF_LOG("  --\n");
}


#endif
DECLARE_WAIT_QUEUE_HEAD (waiting_spi_prepare);
atomic_t suspended;


extern int hct_finger_set_power(int cmd);
extern int hct_finger_set_18v_power(int cmd);
extern int hct_finger_set_reset(int cmd);
extern int hct_finger_set_eint(int cmd);
extern int hct_finger_set_spi_mode(int cmd);
extern void hct_waite_for_finger_dts_paser(void);//add
#if defined(CONFIG_FB)
/*----------------------------------------------------------------------------*/
static int fb_notifier_callback(struct notifier_block *self,
                                unsigned long event, void *data)
{
    struct fb_event *evdata = data;
    int *blank =  evdata->data;
    if (evdata && evdata->data) {
        if (event == FB_EVENT_BLANK ) {
            if (*blank == FB_BLANK_UNBLANK) {
                BF_LOG("fb_notifier_callback FB_BLANK_UNBLANK:%d", g_bf_dev->need_report);
                g_bf_dev->need_report = 0;
                bf_send_netlink_msg(g_bf_dev, BF_NETLINK_CMD_SCREEN_ON);
            } else if (*blank == FB_BLANK_POWERDOWN) {
                BF_LOG("fb_notifier_callback FB_BLANK_POWERDOWN:%d", g_bf_dev->need_report);
                g_bf_dev->need_report = 1;
                bf_send_netlink_msg(g_bf_dev, BF_NETLINK_CMD_SCREEN_OFF);
            }
        }
    }
    return 0;
}
#elif defined(CONFIG_HAS_EARLYSUSPEND)
static void early_suspend(struct bf_device *bf_dev)
{
    BF_LOG("++++++++++\n");
    g_bf_dev->need_report = 1;
    bf_send_netlink_msg(g_bf_dev, BF_NETLINK_CMD_SCREEN_OFF);
    BF_LOG("----------\n");
}
static void early_resume(struct bf_device *bf_dev)
{
    BF_LOG("+++++++++++\n");
    g_bf_dev->need_report = 0;
    bf_send_netlink_msg(g_bf_dev, BF_NETLINK_CMD_SCREEN_ON);
    BF_LOG("----------\n");
}
#endif

static int bf_hw_power(struct bf_device *bf_dev, bool enable)
{
int err = 0;

     if (enable) {
        err = regulator_enable(bf_dev->vdd_reg);

        if (err) {
            BF_LOG(KERN_ERR, "Regulator vdd enable failed err = %d\n", err);
            return err;
        }

        msleep(10);
    }
    else if (!enable) {
        err = regulator_disable(bf_dev->vdd_reg);

        if (err) {
            BF_LOG(KERN_ERR, "Regulator vdd disable failed err = %d\n", err);
            return err;
        }
    }





#if defined BF_PINCTL
    if (enable) {
#ifdef NEED_OPT_POWER_ON2V8
        pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_power_2v8_high);
#endif
#ifdef NEED_OPT_POWER_ON1V8
        pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_power_1v8_high);
#endif
    } else {
#ifdef NEED_OPT_POWER_ON2V8
        pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_power_2v8_low);
#endif
#ifdef NEED_OPT_POWER_ON1V8
        pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_power_1v8_low);
#endif
    }
#else    //BF_PINCTL
    if (enable) {
#ifdef NEED_OPT_POWER_ON2V8
        gpio_direction_output (bf_dev->power_2v8_gpio, 1);
#endif
#ifdef NEED_OPT_POWER_ON1V8
        gpio_direction_output (bf_dev->power_1v8_gpio, 1);
#endif
    } else {
#ifdef NEED_OPT_POWER_ON2V8
        gpio_direction_output (bf_dev->power_2v8_gpio, 0);
#endif
#ifdef NEED_OPT_POWER_ON1V8
        gpio_direction_output (bf_dev->power_1v8_gpio, 0);
#endif
    }
#endif    //BF_PINCTL
    return 0;
}

static int bf_hw_reset(struct bf_device *bf_dev)
{
#if defined BF_PINCTL
    pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_reset_low);
    mdelay(5);
    pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_reset_high);
#else
    gpio_direction_output (bf_dev->reset_gpio, 0);
    mdelay(5);
    gpio_direction_output (bf_dev->reset_gpio, 1);
#endif
    return 0;
}

static int bf_hw_reset_level (struct bf_device *bf_dev, bool enable)
{
    BF_LOG("bf_hw_reset_level %d", enable);
#if defined BF_PINCTL
    if (enable) {
        pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_reset_high);
    } else {
        pinctrl_select_state (bf_dev->pinctrl_gpios, bf_dev->pins_reset_low);
    }
#else
    if (enable) {
        gpio_direction_output(bf_dev->reset_gpio, 1);
    } else {
        gpio_direction_output(bf_dev->reset_gpio, 0);
    }
#endif
    return 0;
}

static ssize_t bf_show_hwreset(struct device *ddri, struct device_attribute *attr, char *buf)
{
    u32 pin_val = -1;
    pin_val = gpio_get_value(g_bf_dev->reset_gpio);
    BF_LOG("reset pin_val=%d\n", pin_val);

    return sprintf(buf, "reset pin_val=%d\n", pin_val);
}
static ssize_t bf_store_hwreset(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    char *next;
    u8 hwreset_flag  = simple_strtoul(buf, &next, 10);
    BF_LOG("hwreset_flag %d\n", hwreset_flag);
    if(hwreset_flag) {
		hct_finger_set_reset(1);
        //bf_hw_reset_level(g_bf_dev, 1);
    } else {
		hct_finger_set_reset(0);
        //bf_hw_reset_level(g_bf_dev, 0);
    }

    return size;
}
static DEVICE_ATTR(reset, 0664, bf_show_hwreset, bf_store_hwreset);

static ssize_t bf_show_powermode(struct device *ddri, struct device_attribute *attr, char *buf)
{
    u32 pin_val = -1;
    pin_val = gpio_get_value(g_bf_dev->power_gpio);
    BF_LOG("power pin_val=%d\n", pin_val);

    return sprintf(buf, "power pin_val=%d\n", pin_val);
}
static ssize_t bf_store_powermode(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
    char *next;
    u8 power_flag  = simple_strtoul(buf, &next, 10);
    BF_LOG("power_flag %d\n", power_flag);
    if(power_flag) {
//hct_finger_set_power(1);
        bf_hw_power(g_bf_dev, 1);
    } else {
//hct_finger_set_power(0);
        bf_hw_power(g_bf_dev, 0);
    }

    return size;
}
static DEVICE_ATTR(power, 0664, bf_show_powermode, bf_store_powermode);

/*----------------------------------------------------------------------------*/
static struct device_attribute *bf_attr_list[] = {
    &dev_attr_reset,
    &dev_attr_power,
};
/*----------------------------------------------------------------------------*/
static void bf_create_attributes(struct device *dev)
{
    int num = (int)(sizeof(bf_attr_list) / sizeof(bf_attr_list[0]));
    for (; num > 0;)
        device_create_file(dev, bf_attr_list[--num]);
}

static int bf_sysfs_init(void)
{
    int ret = 0;
    bf_kobj = kobject_create_and_add("bf_sysfs", NULL);
    if(bf_kobj == NULL) {
        BF_LOG("subsystem_register failed\n");
        ret = -ENOMEM;
        return ret;
    }

    ret = sysfs_create_file(bf_kobj, &dev_attr_reset.attr);
    ret = sysfs_create_file(bf_kobj, &dev_attr_power.attr);

    if(ret) {
        BF_LOG("sysfs_create_file failed\n");
    }

    kobject_uevent(bf_kobj, KOBJ_ADD);
    return ret;
}

static int bf_sysfs_uninit(void)
{
    int ret;

    if(bf_kobj == NULL) {
        BF_LOG("bf_kobj don't exist \n");
        ret = -EEXIST;
        return ret;
    }

    sysfs_remove_file(bf_kobj, &dev_attr_reset.attr);
    sysfs_remove_file(bf_kobj, &dev_attr_power.attr);
    kobject_del(bf_kobj);
    return ret;
}

static void bf_remove_attributes(struct device *dev)
{
    int num = (int)(sizeof(bf_attr_list) / sizeof(bf_attr_list[0]));
    for (; num > 0;)
        device_remove_file(dev, bf_attr_list[--num]);
}

/**
 * get gpio information from device tree
 */
static int bf_main_get_gpio_info (struct bf_device *bf_dev)
{
    struct device_node *node = NULL;
    int err = 0;


     node = bf_dev->pdev->dev.of_node;
    if (node) {

        bf_dev->irq_gpio =  of_get_named_gpio(node, "fpint-gpio", 0);
        if(bf_dev->irq_gpio < 0) {
            BF_LOG("get fpint-gpio fail:%d", bf_dev->reset_gpio);
            return bf_dev->irq_gpio;
        }

        bf_dev->irq_num = gpio_to_irq(bf_dev->irq_gpio);
        if(!bf_dev->irq_num) {
            BF_LOG("get irq number fail:%d", bf_dev->irq_num);
            return -ENXIO;
        }
     }

	hct_finger_set_spi_mode(1);
    //hct_waite_for_finger_dts_paser();
	hct_finger_set_eint(1);

BF_LOG("lqw 111111111");

	bf_dev->pdev->dev.of_node= of_find_compatible_node(NULL, NULL, "mediatek,hct_finger");
BF_LOG("lqw 222222222222");
	if (!bf_dev->pdev->dev.of_node) {
		BF_LOG( "of_find_compatible_node(..) failed.\n");
		return (-ENODEV);
	}

//	bf_dev->irq_num = irq_of_parse_and_map(bf_dev->pdev->dev.of_node, 0);
	//BF_LOG(KERN_INFO, "irq number is %d.\n", bf_dev->irq_num);

   	bf_dev->vdd_reg = regulator_get(&bf_dev->pdev->dev, "irtx_ldo");
	BF_LOG("zcy add %s is regulator_get !!!\n",__func__);

    if (IS_ERR(bf_dev->vdd_reg)) {
        err = PTR_ERR(bf_dev->vdd_reg);
        BF_LOG(KERN_ERR, "Regulator get failed vdd err = %d\n", err);
        return err;
    }
BF_LOG("lqw 3333333333333");
    if (regulator_count_voltages(bf_dev->vdd_reg) > 0) {
        err = regulator_set_voltage(bf_dev->vdd_reg, 2800000,
                                    2800000);

        if (err < 0 ) {
            BF_LOG(KERN_ERR, "Regulator set_vtg failed vdd err = %d\n", err);
            return err;
        }
    }
BF_LOG("lqw 55555555");

return 0;
}



#ifdef BF_PINCTL
static int32_t bf_main_pinctrl_init(struct bf_device *bf_dev)
{
    int32_t error = 0;


    error = pinctrl_select_state(bf_dev->pinctrl_gpios, bf_dev->pins_reset_high);
    if (error) {
        dev_err(&bf_dev->pdev->dev, "failed to activate pins_reset_high state\n");
    }

    error = pinctrl_select_state(bf_dev->pinctrl_gpios, bf_dev->pins_fp_interrupt);
    if (error) {
        dev_err(&bf_dev->pdev->dev, "failed to activate pins_fp_interrupt state\n");
    }
#ifdef NEED_OPT_POWER_ON2V8
    error = pinctrl_select_state(bf_dev->pinctrl_gpios, bf_dev->pins_power_2v8_high);
    if (error) {
        dev_err(&bf_dev->pdev->dev, "failed to activate pins_power_2v8_high state\n");
    }

#endif
#ifdef NEED_OPT_POWER_ON_1V8
    error = pinctrl_select_state(bf_dev->pinctrl_gpios, bf_dev->pins_power_1v8_high);
    if (error) {
        dev_err(&bf_dev->pdev->dev, "failed to activate pins_power_1v8_high state\n");
    }

#endif

    return error;
}
#endif    //BF_PINCTL

/*
 *gpio???????????????
 */
static int32_t bf_main_gpio_init(struct bf_device *bf_dev)
{
    int error = 0;
#if 0
    /*reset pin*/
    if (gpio_is_valid(bf_dev->reset_gpio)) {
        error = gpio_request(bf_dev->reset_gpio, "bf reset");
        if (error) {
            dev_err(&bf_dev->pdev->dev, "unable to request reset GPIO %d\n", bf_dev->reset_gpio);
            goto out;
        } else {
            gpio_direction_output (bf_dev->reset_gpio, 1);
        }
    } else {
        dev_err(&bf_dev->pdev->dev, "invalid reset GPIO %d\n", bf_dev->reset_gpio);
        error = -1;
        goto out;
    }
#endif
	printk("zcy add %s is start !!!\n",__func__);
    /*irq pin*/
    if (gpio_is_valid(bf_dev->irq_gpio)) {
BF_LOG("lqw 11111111111111");
        error = gpio_request(bf_dev->irq_gpio, "bf irq_gpio");
        if (error) {
            dev_err(&bf_dev->pdev->dev, "unable to request irq_gpio GPIO %d\n", bf_dev->irq_gpio);
            goto out1;
        } else {
BF_LOG("lqw 22222222222222");
            gpio_direction_input(bf_dev->irq_gpio);
        }
    } else {
        dev_err(&bf_dev->pdev->dev, "invalid irq_gpio GPIO %d\n", bf_dev->irq_gpio);
        error = -1;
        goto out1;
    }

#ifdef NEED_OPT_POWER_ON2V8
    if (gpio_is_valid(bf_dev->power_2v8_gpio)) {
        error = gpio_request(bf_dev->power_2v8_gpio, "bf power_2v8_gpio");
        if (error) {
            dev_err(&bf_dev->pdev->dev, "unable to request power_2v8_gpio GPIO %d\n", bf_dev->power_2v8_gpio);
            goto out2;
        } else {
            gpio_direction_output (bf_dev->power_2v8_gpio, 1);
        }
    } else {
        dev_err(&bf_dev->pdev->dev, "invalid power_2v8_gpio GPIO %d\n", bf_dev->power_2v8_gpio);
        error = -1;
        goto out2;
    }
#endif    //NEED_OPT_POWER_ON_2V8

#ifdef NEED_OPT_POWER_ON1V8
    if (gpio_is_valid(bf_dev->power_1v8_gpio)) {
        error = gpio_request(bf_dev->power_1v8_gpio, "bf power_1v8_gpio");
        if (error) {
            dev_err(&bf_dev->pdev->dev, "unable to request power_1v8_gpio GPIO %d\n", bf_dev->power_1v8_gpio);
            goto out3;
        } else {
            gpio_direction_output (bf_dev->power_1v8_gpio, 1);
        }

    } else {
        dev_err(&bf_dev->pdev->dev, "invalid power_1v8_gpio GPIO %d\n", bf_dev->power_1v8_gpio);
        error = -1;
        goto out3;
    }
#endif    //NEED_OPT_POWER_ON1V8
    return 0;


out3:
#ifdef NEED_OPT_POWER_ON2V8
#ifndef MTK_ANDROID_L
    gpio_free(bf_dev->power_2v8_gpio);
#endif
#endif //NEED_OPT_POWER_ON2V8

out2:
#ifndef MTK_ANDROID_L
    gpio_free(bf_dev->irq_gpio);
#endif
out1:
#ifndef MTK_ANDROID_L
   // gpio_free(bf_dev->reset_gpio);
#endif
out:
    return error;
}

static void bf_main_gpio_uninit(struct bf_device *bf_dev)
{
#ifndef MTK_ANDROID_L
    if (gpio_is_valid(bf_dev->irq_gpio))
        gpio_free(bf_dev->irq_gpio);
    if (gpio_is_valid(bf_dev->reset_gpio))
        gpio_free(bf_dev->reset_gpio);
#ifdef NEED_OPT_POWER_ON2V8
    if (gpio_is_valid(bf_dev->power_2v8_gpio))
        gpio_free(bf_dev->power_2v8_gpio);
#endif
#ifdef NEED_OPT_POWER_ON1V8
    if (gpio_is_valid(bf_dev->power_1v8_gpio))
        gpio_free(bf_dev->power_1v8_gpio);
#endif
#endif
}

static void bf_main_pin_uninit(struct bf_device *bf_dev)
{
#ifdef BF_PINCTL
    devm_pinctrl_put(bf_dev->pinctrl_gpios);
#endif
}

static int32_t bf_main_pin_init(struct bf_device *bf_dev)
{
    int32_t error = 0;
	printk("zcy add %s is start !!!\n",__func__);
/*
#ifdef BF_PINCTL

    error = bf_main_pinctrl_init(bf_dev);
    if(error) {
        BF_LOG("bf_main_pinctrl_init fail!");
    }

#else
*/

    error = bf_main_gpio_init(bf_dev);
    if(error) {
        BF_LOG("bf_main_gpio_init fail!");
    }


    return error;
}

/* -------------------------------------------------------------------- */
/* netlink functions                 */
/* -------------------------------------------------------------------- */
void bf_send_netlink_msg(struct bf_device *bf_dev, const int command)
{
    struct nlmsghdr *nlh = NULL;
    struct sk_buff *skb = NULL;
    int ret;
    char data_buffer[2];

    BF_LOG("enter, send command %d", command);
    memset(data_buffer, 0, 2);
    data_buffer[0] = (char)command;
    if (NULL == bf_dev->netlink_socket) {
        BF_LOG("invalid socket");
        return;
    }

    if (0 == g_pid) {
        BF_LOG("invalid native process pid");
        return;
    }

    /*alloc data buffer for sending to native*/
    skb = alloc_skb(MAX_NL_MSG_LEN, GFP_ATOMIC);
    if (skb == NULL) {
        return;
    }

    nlh = nlmsg_put(skb, 0, 0, 0, MAX_NL_MSG_LEN, 0);
    if (!nlh) {
        BF_LOG("nlmsg_put failed");
        kfree_skb(skb);
        return;
    }

    NETLINK_CB(skb).portid = 0;
    NETLINK_CB(skb).dst_group = 0;

    *(char *)NLMSG_DATA(nlh) = command;
    *((char *)NLMSG_DATA(nlh) + 1) = 0;
    ret = netlink_unicast(bf_dev->netlink_socket, skb, g_pid, MSG_DONTWAIT);
    if (ret < 0) {
        BF_LOG("send failed");
        return;
    }

    BF_LOG("send done, data length is %d", ret);
    return ;
}

static void bf_recv_netlink_msg(struct sk_buff *__skb)
{
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;
    char str[128];


    skb = skb_get(__skb);
    if (skb == NULL) {
        BF_LOG("skb_get return NULL");
        return;
    }

    if (skb->len >= NLMSG_SPACE(0)) {
        nlh = nlmsg_hdr(skb);
        //add by wangdongbo
        //memcpy(str, NLMSG_DATA(nlh), sizeof(str));
        g_pid = nlh->nlmsg_pid;
        BF_LOG("pid: %d, msg: %s", g_pid, str);
        mutex_lock(&irq_count_lock);
        g_bf_dev->irq_count = 1;
        mutex_unlock(&irq_count_lock);
    } else {
        BF_LOG("not enough data length");
    }

    kfree_skb(__skb);
}

static int bf_destroy_inputdev(void)
{
    if (bf_inputdev) {
        input_unregister_device(bf_inputdev);
        input_free_device(bf_inputdev);
        bf_inputdev = NULL;
    }
    return 0;
}

static int bf_close_netlink(struct bf_device *bf_dev)
{
    if (bf_dev->netlink_socket != NULL) {
        netlink_kernel_release(bf_dev->netlink_socket);
        bf_dev->netlink_socket = NULL;
        return 0;
    }

    BF_LOG("no netlink socket yet");
    return -1;
}

static int bf_init_netlink(struct bf_device *bf_dev)
{
    struct netlink_kernel_cfg cfg;

    BF_LOG("bf_init_netlink g_netlink_port: %d", g_netlink_port);

    memset(&cfg, 0, sizeof(struct netlink_kernel_cfg));
    cfg.input = bf_recv_netlink_msg;

    bf_dev->netlink_socket = netlink_kernel_create(&init_net, g_netlink_port, &cfg);
    if (bf_dev->netlink_socket == NULL) {
        BF_LOG("netlink create failed");
        return -1;
    }
    BF_LOG("netlink create success");
    return 0;
}

static irqreturn_t bf_eint_handler (int irq, void *data)
{
    struct bf_device *bf_dev = (struct bf_device *)data;

    wait_event_interruptible_timeout(waiting_spi_prepare, !atomic_read(&suspended), msecs_to_jiffies (100));
    wake_lock_timeout(&fp_suspend_lock, 2 * HZ);
    BF_LOG("++++irq_handler netlink send+++++,%d,%d", g_bf_dev->irq_count, bf_dev->doing_reset);
    if(g_bf_dev->irq_count) {
        if(!bf_dev->doing_reset) {
            mutex_lock(&irq_count_lock);
            g_bf_dev->irq_count = 0;
            mutex_unlock(&irq_count_lock);
        }
        bf_send_netlink_msg(bf_dev, BF_NETLINK_CMD_IRQ);
    }
    BF_LOG("-----irq_handler netlink -----");
    return IRQ_HANDLED;
}

#ifdef FAST_VERSION
int g_bl229x_enbacklight = 1;
#endif

/* -------------------------------------------------------------------- */
/* file operation function                                                                                */
/* -------------------------------------------------------------------- */
static long bf_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int error = 0;
#ifdef FAST_VERSION
    u32 bl229x_enbacklight = 0;
    u32 chipid = 0x5283;
#endif
#ifdef BF_REE
    bl_read_write_reg_command_t read_write_cmd;
    int dma_size = 0;
#endif
    struct bf_device *bf_dev = NULL;
    unsigned int key_event = 0;
    BF_LOG("bf_ioctl davie.");

    bf_dev = (struct bf_device *)filp->private_data;
    if (_IOC_TYPE(cmd) != BF_IOCTL_MAGIC_NO) {
        BF_LOG("Not blestech fingerprint cmd.");
        return -EINVAL;
    }

    if (_IOC_DIR(cmd) & _IOC_READ)
        error = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));

    if (error == 0 && _IOC_DIR(cmd) & _IOC_WRITE)
        error = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

    if (error) {
        BF_LOG("Not blestech fingerprint cmd direction.");
        return -EINVAL;
    }

    switch (cmd) {
    case BF_IOCTL_RESET:
        BF_LOG("BF_IOCTL_RESET: chip reset command\n");
       // bf_hw_reset(bf_dev);
	 hct_finger_set_reset(0);
	 mdelay(5);
	 hct_finger_set_reset(1);
        break;
    case BF_IOCTL_ENABLE_INTERRUPT:
        BF_LOG("BF_IOCTL_ENABLE_INTERRUPT:  command,%d\n", bf_dev->irq_count);
        mutex_lock(&irq_count_lock);
        bf_dev->irq_count = 1;
        mutex_unlock(&irq_count_lock);
        break;
    case BF_IOCTL_DISABLE_INTERRUPT:
        BF_LOG("BF_IOCTL_DISABLE_INTERRUPT:  command,%d\n", bf_dev->irq_count);
        mutex_lock(&irq_count_lock);
        bf_dev->irq_count = 0;
        mutex_unlock(&irq_count_lock);
        break;
    case BF_IOCTL_ENABLE_POWER:
        BF_LOG("BF_IOCTL_ENABLE_POWER:  command\n");
        bf_hw_power(bf_dev, 1);
	//hct_finger_set_power(1);
        break;
    case BF_IOCTL_DISABLE_POWER:
        BF_LOG("BF_IOCTL_DISABLE_POWER:  command\n");
       bf_hw_power(bf_dev, 0);
	//hct_finger_set_power(0);
        break;
    case BF_IOCTL_INPUT_KEY:
        key_event = (unsigned int)arg;
        BF_LOG("key:%d\n", key_event);

        input_report_key(bf_inputdev, key_event, 1);
        input_sync(bf_inputdev);
        input_report_key(bf_inputdev, key_event, 0);
        input_sync(bf_inputdev);
        break;
#ifdef FAST_VERSION
    case BF_IOCTL_ENBACKLIGHT:
        BF_LOG("BF_IOCTL_ENBACKLIGHT arg:%d\n", (int)arg);
        g_bl229x_enbacklight = (int)arg;
        break;
    case BF_IOCTL_ISBACKLIGHT:
        BF_LOG("BF_IOCTL_ISBACKLIGHT\n");
        bl229x_enbacklight = g_bl229x_enbacklight;
        if (copy_to_user((void __user*)arg, &bl229x_enbacklight, sizeof(u32) * 1) != 0 ) {
            error = -EFAULT;
        }
        break;
    case BF_IOCTL_GET_ID:
        if (copy_to_user((void __user*)arg, &chipid, sizeof(u32) * 1) != 0 ) {
            error = -EFAULT;
        }
        break;
#endif
#ifdef BF_REE
    case BF_IOCTL_REGISTER_READ_WRITE:
        BF_LOG("BTL:BF_IOCTL_REGISTER_READ_WRITE\n");

        if(copy_from_user(&read_write_cmd, (bl_read_write_reg_command_t *)arg, sizeof(read_write_cmd))) {
            error = -EFAULT;
            BF_LOG("BF_IOCTL_REGISTER_READ_WRITE copy_to_user faile!");
            break;
        }

        if(read_write_cmd.len < 32) {
            spi_send_cmd(g_bf_dev, read_write_cmd.data_tx, read_write_cmd.data_rx, read_write_cmd.len);
            if (copy_to_user((void __user*)arg, &read_write_cmd, sizeof(bl_read_write_reg_command_t)) != 0 ) {
                BF_LOG("BF_IOCTL_REGISTER_READ_WRITE copy_to_user faile!");
                error = -EFAULT;
                break;
            }
        } else {
            dma_size = ((read_write_cmd.len / 1024) + 1) * 1024;
            memset(g_bf_dev->image_buf, 0xff, get_order(dma_size));
            spi_set_dma_en(1);
#ifdef USE_SPI1_4GB_TEST
            spi_dma_exchange(g_bf_dev, read_write_cmd, dma_size);
#else
            spi_send_cmd(g_bf_dev, read_write_cmd.data_tx, g_bf_dev->image_buf, dma_size);
#endif
            spi_set_dma_en(0);
        }
        break;
#endif
#ifdef CONFIG_MTK_CLK
    case BF_IOCTL_ENABLE_SPI_CLOCK:
        BF_LOG("BF_IOCTL_ENABLE_SPI_CLK:  command\n");
        bf_spi_clk_enable(bf_dev, 1);
        break;
    case BF_IOCTL_DISABLE_SPI_CLOCK:
        BF_LOG("BF_IOCTL_DISABLE_SPI_CLK:  command\n");
        bf_spi_clk_enable(bf_dev, 0);
        break;
#endif
    case BF_IOCTL_INPUT_KEY_DOWN:
#ifdef FAST_VERSION
        if(g_bl229x_enbacklight && g_bf_dev->need_report == 0) {
#else
        if(g_bf_dev->need_report == 0) {
#endif
            bf_key_need_report = 1;
            key_event = (int)arg;
            input_report_key(bf_inputdev, key_event, 1);
            input_sync(bf_inputdev);
        }
        break;
    case BF_IOCTL_INPUT_KEY_UP:
        if(bf_key_need_report == 1) {
            bf_key_need_report = 0;
            key_event = (int)arg;
            input_report_key(bf_inputdev, key_event, 0);
            input_sync(bf_inputdev);
        }
        break;
    case BF_IOCTL_LOW_RESET:
        BF_LOG("BF_IOCTL_LOW_RESET:  command\n");
        wake_lock_timeout(&hw_reset_lock, 2 * HZ);
       // bf_hw_reset_level(g_bf_dev, 0);
	 hct_finger_set_reset(0);

        break;

    case BF_IOCTL_HIGH_RESET:
        BF_LOG("BF_IOCTL_HIGH_RESET:  command\n");
 //       bf_hw_reset_level(g_bf_dev, 1);

 	hct_finger_set_reset(1); 
        break;

    case BF_IOCTL_NETLINK_INIT:
        BF_LOG("BF_IOCTL_NETLINK_INIT:  command\n");
        g_netlink_port = (int)arg;
        bf_close_netlink(g_bf_dev);
        error = bf_init_netlink(g_bf_dev);
        if (error < 0) {
            BF_LOG("BF_IOCTL_NETLINK_INIT:  error\n");
        }
        break;

    case BF_IOCTL_NETLINK_PORT:
        BF_LOG("BF_IOCTL_NETLINK_PORT:  command\n");
        if (copy_to_user((void __user*)arg, &g_netlink_port, sizeof(u32) * 1) != 0 ) {
            error = -EFAULT;
        }
        break;

    case BF_IOCTL_INIT_DEVICE:
        BF_LOG("BF_IOCTL_INIT_DEVICE:  command\n");
        error = bf_init_dts_and_irq(g_bf_dev);
        break;

    case BF_IOCTL_REMOVE_DEVICE:
        BF_LOG("BF_IOCTL_REMOVE_DEVICE:  command\n");
        bf_remove(g_bf_dev->pdev);
        break;

    case BF_IOCTL_RESET_FLAG:
        bf_dev->doing_reset = (u8)arg;
        BF_LOG("BF_IOCTL_RESET_FLAG:  command,%d\n", bf_dev->doing_reset);
        break;
    case BF_IOCTL_IS_OPT_POWER_ON2V8:
        BF_LOG("BF_IOCTL_IS_OPT_POWER_ON2V8:  command\n");
#if 1
        key_event = 1;
#else
        key_event = 0;
#endif
        if (copy_to_user((void __user*)arg, &key_event, sizeof(u32) * 1) != 0 ) {
            error = -EFAULT;
        }
        break;
    default:
        BF_LOG("Supportn't this command(%x)\n", cmd);
        break;
    }

    return error;
}
#ifdef CONFIG_COMPAT
static long bf_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int retval = 0;

    retval = bf_ioctl(filp, cmd, arg);

    return retval;
}
#endif

/*----------------------------------------------------------------------------*/
static int bf_open (struct inode *inode, struct file *filp)
{
    struct bf_device *bf_dev = g_bf_dev;
    int status = 0;

    filp->private_data = bf_dev;
    BF_LOG( " Success to open device.");

    return status;
}


/* -------------------------------------------------------------------- */
static ssize_t bf_write (struct file *file, const char *buff, size_t count, loff_t *ppos)
{
    return -ENOMEM;
}

/* -------------------------------------------------------------------- */
static ssize_t bf_read (struct file *filp, char  *buff, size_t count, loff_t *ppos)
{

    ssize_t status = 0;
#ifdef BF_REE
    int ret = 0;
    //    spi_read_frame(struct  bf_device *bf_dev);

    ret = copy_to_user(buff, g_bf_dev->image_buf , count); //skip
    if (ret) {
        BF_LOG("copy_to_user failed\n");
        status = -EFAULT;
    }
#endif
    BF_LOG("status: %d \n", (int)status);
    BF_LOG("  --\n");
    return status;

}

/* -------------------------------------------------------------------- */
static int bf_release (struct inode *inode, struct file *file)
{
    int status = 0 ;
    return status;
}
static int bf_suspend (struct platform_device *pdev, pm_message_t state)
{
    BF_LOG("  ++\n");
    g_bf_dev->need_report = 1;
    BF_LOG("\n");
    return 0;
}
static int bf_resume (struct platform_device *pdev)
{
    BF_LOG("  ++\n");
    g_bf_dev->need_report = 0;
    BF_LOG("\n");
    return 0;
}

/*----------------------------------------------------------------------------*/
static const struct file_operations bf_fops = {
    .owner = THIS_MODULE,
    .open  = bf_open,
    .write = bf_write,
    .read  = bf_read,
    .release =  bf_release,
    .unlocked_ioctl = bf_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = bf_compat_ioctl,
#endif
};

static struct miscdevice bf_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = BF_DEV_NAME,
    .fops = &bf_fops,
};

int bf_remove(struct platform_device *pdev)
{
    struct bf_device *bf_dev = g_bf_dev;
    BF_LOG("bf_remove++++");

    bf_remove_attributes(&bf_dev->pdev->dev);
    bf_sysfs_uninit();

    bf_hw_power(bf_dev, 0);
//hct_finger_set_power(0);
    BF_LOG("lqw 1111111111++++");
    if (bf_dev->irq_num) {
#ifndef MTK_ANDROID_L
        free_irq(bf_dev->irq_num, bf_dev);
#else
 BF_LOG("lqw 222222222++++");
        disable_irq_nosync(bf_dev->irq_num);
 BF_LOG("lqw 333333333++++");
#endif
        bf_dev->irq_count = 0;
        bf_dev->irq_num = 0;
    }
 BF_LOG("lqw 55555555++++");
    bf_main_gpio_uninit(bf_dev);
 BF_LOG("lqw 666666666666+++");
    bf_main_pin_uninit(bf_dev);
 BF_LOG("lqw 7777777777+++");
#if defined(CONFIG_FB)
    fb_unregister_client(&bf_dev->fb_notify);
#elif defined(CONFIG_HAS_EARLYSUSPEND)
    unregister_early_suspend(&bf_dev->early_suspend);
#endif
 BF_LOG("lqw 88888888+++");
    bf_destroy_inputdev();
 BF_LOG("lqw 0999999999999+++");
    misc_deregister(&bf_misc_device);
 BF_LOG("lqw 11222233333+++");
    bf_close_netlink(bf_dev);
 BF_LOG("lqw 33344444444+++");
#if defined(BF_REE)
    free_pages((unsigned long)bf_dev->image_buf, get_order(BUF_SIZE));
#endif
 BF_LOG("lqw 556664+++");
    kfree(bf_dev);
 BF_LOG("lqw 7878787878787+++");
    platform_set_drvdata(bf_dev->pdev, NULL);

    BF_LOG("bf_remove----");
    return 0;
}

static int bf_create_inputdev(void)
{
    bf_inputdev = input_allocate_device();
    if (!bf_inputdev) {
        BF_LOG("bf_inputdev create faile!\n");
        return -ENOMEM;
    }
    __set_bit(EV_KEY, bf_inputdev->evbit);
    __set_bit(KEY_F10, bf_inputdev->keybit);    //68
    __set_bit(KEY_F11, bf_inputdev->keybit);    //88
    __set_bit(KEY_F12, bf_inputdev->keybit);    //88
    __set_bit(KEY_CAMERA, bf_inputdev->keybit); //212
    __set_bit(KEY_POWER, bf_inputdev->keybit);  //116
    __set_bit(KEY_PHONE, bf_inputdev->keybit);  //call 169
    __set_bit(KEY_BACK, bf_inputdev->keybit);   //call 158
    __set_bit(KEY_HOMEPAGE, bf_inputdev->keybit);   //call 172
    __set_bit(KEY_MENU, bf_inputdev->keybit);   //call 158

    __set_bit(KEY_F1, bf_inputdev->keybit);     //69
    __set_bit(KEY_F2, bf_inputdev->keybit);     //60
    __set_bit(KEY_F3, bf_inputdev->keybit);     //61
    __set_bit(KEY_F4, bf_inputdev->keybit);     //62
    __set_bit(KEY_F5, bf_inputdev->keybit);     //63
    __set_bit(KEY_F6, bf_inputdev->keybit);     //64
    __set_bit(KEY_F7, bf_inputdev->keybit);     //65
    __set_bit(KEY_F8, bf_inputdev->keybit);     //66
    __set_bit(KEY_F9, bf_inputdev->keybit);     //67

    __set_bit(KEY_UP, bf_inputdev->keybit);     //103
    __set_bit(KEY_DOWN, bf_inputdev->keybit);   //108
    __set_bit(KEY_LEFT, bf_inputdev->keybit);   //105
    __set_bit(KEY_RIGHT, bf_inputdev->keybit);  //106

    bf_inputdev->id.bustype = BUS_HOST;
    bf_inputdev->name = "betterlife_inputdev";
    if (input_register_device(bf_inputdev)) {
        BF_LOG("register inputdev failed");
        input_free_device(bf_inputdev);
        return -ENOMEM;
    }
    return 0;
}

static int bf_init_dts_and_irq(struct bf_device *bf_dev)
{
    int32_t status = -EINVAL;
    BF_LOG( "    ++++");
    status = bf_main_get_gpio_info(bf_dev);
    if(status) {
        BF_LOG("bf_main_get_gpio_info fail:%d", status);
        return -1;
    }
	printk("zcy add %s is 11111111!\n",__func__);
    status = bf_main_pin_init(bf_dev);
    if(status) {
        BF_LOG("bf_main_init fail:%d", status);
        return -2;
    }

#ifdef MTK_ANDROID_L
    mt_eint_registration(bf_dev->irq_num, EINTF_TRIGGER_RISING/*CUST_EINTF_TRIGGER_RISING*/, bf_eint_handler_l, 0);
    INIT_WORK(&(bf_dev->fingerprint_work), work_func);
    bf_dev->fingerprint_workqueue = create_singlethread_workqueue("bf_fingerpirnt_thread");
#else
    status = request_threaded_irq (bf_dev->irq_num, NULL, bf_eint_handler,  IRQ_TYPE_EDGE_RISING /*IRQF_TRIGGER_RISING*/ | IRQF_ONESHOT, BF_DEV_NAME, bf_dev);
#endif

    if (status) {
        BF_LOG("irq thread request failed, retval=%d\n", status);
        return -3;
    }

   // bf_hw_reset(bf_dev);
 hct_finger_set_reset(0);
mdelay(5);
 hct_finger_set_reset(1);  

    enable_irq_wake(bf_dev->irq_num);
    BF_LOG( "    ----");
    return 0;
}

static int bf_probe(struct platform_device *pdev)
{
    struct bf_device *bf_dev = NULL;
    int32_t status = -EINVAL;

    BF_LOG( "++++++++++++");

    bf_dev = kzalloc(sizeof (struct bf_device), GFP_KERNEL);
    if (NULL == bf_dev) {
        BF_LOG( "kzalloc bf_dev failed.");
        status = -ENOMEM;
        goto err0;
    }

    bf_dev->pdev = pdev;
    bf_dev->irq_count = 0;
    bf_dev->doing_reset = 0;
    bf_dev->report_key = KEY_F10;
    wake_lock_init(&fp_suspend_lock, WAKE_LOCK_SUSPEND, "fp_wakelock");
    wake_lock_init(&hw_reset_lock, WAKE_LOCK_SUSPEND, "fp_reset_wakelock");
    atomic_set(&suspended, 0);
    g_bf_dev = bf_dev;
    platform_set_drvdata(pdev, bf_dev);

#if defined(BF_REE)
    g_bf_dev->image_buf = (u8*)__get_free_pages(GFP_KERNEL, get_order(BUF_SIZE));
    if (!bf_dev->image_buf) {
        BF_LOG("\ng_bf_dev->image_buf __get_free_pages failed\n");
        status = -ENOMEM;
        goto err1;
    }
#endif

#ifndef COMPATIBLE_IN_HAL
    status = bf_init_dts_and_irq(bf_dev);
    if (status == -1) {
        goto err2;
    } else if (status == -2) {
        goto err3;
    } else if (status == -3) {
        goto err4;
    }
#else
    BF_LOG("compatible in hal, do not init gpio pin hw reset and init_irq.");
#endif

    /* netlink interface init */
    BF_LOG ("bf netlink config");
    if (bf_init_netlink(bf_dev) < 0) {
        BF_LOG ("bf_netlink create failed");
        status = -EINVAL;
        goto err5;
    }

    status = misc_register(&bf_misc_device);
    if(status) {
        BF_LOG("bf_misc_device register failed\n");
        goto err6;
    }

    status = bf_create_inputdev();
    if(status) {
        BF_LOG("bf_create_inputdev failed\n");
        goto err7;
    }

#if defined(CONFIG_FB)
    bf_dev->fb_notify.notifier_call = fb_notifier_callback;
    status = fb_register_client(&bf_dev->fb_notify);
    if(status) {
        BF_LOG("bf_create_inputdev failed\n");
        goto err8;
    }
#elif defined(CONFIG_HAS_EARLYSUSPEND)
    bf_dev->early_suspend.suspend = early_suspend;
    bf_dev->early_suspend.resume = early_resume;
    bf_dev->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
    register_early_suspend(&bf_dev->early_suspend);
#endif

    status = bf_sysfs_init();
    if(status) {
        BF_LOG("bf_create_inputdev failed\n");
        goto err9;
    }

    bf_create_attributes(&bf_dev->pdev->dev);

#ifndef COMPATIBLE_IN_HAL
    bf_hw_power(bf_dev, 1);
   // bf_hw_reset(bf_dev);
 //hct_finger_set_power(1);
 hct_finger_set_reset(0);
mdelay(5);
 hct_finger_set_reset(1);
#endif

#if 0//defined(BF_REE) || defined(COMPATIBLE) || defined(CONFIG_MTK_CLK)
    status = bf_spi_init(bf_dev);
    if(status) {
        BF_LOG("bf_spi_init fail:%d", status);
        goto err10;
    }
#endif

    BF_LOG ("bf_probe success!");
    return 0;

err10:
    bf_remove_attributes(&bf_dev->pdev->dev);
    bf_sysfs_uninit();
err9:
#if defined(CONFIG_FB)
    fb_unregister_client(&bf_dev->fb_notify);
#elif defined(CONFIG_HAS_EARLYSUSPEND)
    unregister_early_suspend(&bf_dev->early_suspend);
#endif
err8:
    bf_destroy_inputdev();
err7:
    misc_deregister(&bf_misc_device);
err6:
    bf_close_netlink(bf_dev);
err5:
#ifndef MTK_ANDROID_L
    free_irq(bf_dev->irq_num, bf_dev);
#else
    disable_irq_nosync(bf_dev->irq_num);
#endif
err4:
    bf_main_gpio_uninit(bf_dev);
err3:
    bf_main_pin_uninit(bf_dev);
err2:
#if defined(BF_REE)
    free_pages((unsigned long)bf_dev->image_buf, get_order(BUF_SIZE));
#endif
err1:
BF_LOG("1111111111111");
    kfree(bf_dev);
BF_LOG("222222222222");
    platform_set_drvdata(bf_dev->pdev, NULL);

err0:
    BF_LOG("bf_probe occured error \n");
    return status;
}

#ifdef CONFIG_OF
static struct of_device_id bf_of_table[] = {
    {.compatible = "betterlife,platform",},
    {.compatible = "mediatek,hct_finger",},               
    {},
};
//MODULE_DEVICE_TABLE(of, bf_of_table);
#endif

#ifdef MTK_ANDROID_L
struct platform_device bf_plt_device = {
    .name = BF_DEV_NAME,
    .id = -1,
};
#endif

static struct platform_driver bf_plt_driver = {
    .driver = {
        .name = BF_DEV_NAME,
        .owner = THIS_MODULE,
#ifdef CONFIG_OF
        .of_match_table = bf_of_table,
#endif
    },

    .probe = bf_probe,
    .remove = bf_remove,
    .resume = bf_resume,
    .suspend = bf_suspend,
};

//module_platform_driver(bf_plt_driver);

#if 1//def MTK_ANDROID_L    /*if active this, MUST no use platform dts, otherwise kernel crashes*/
static int  bf_plt_init(void)
{
    int ret = 0;
/*
    ret = platform_device_register(&bf_plt_device);
    if(ret) {
        BF_LOG ("platform_device_register for blfp failed!");
        return -1;
    }
*/
    
    ret = platform_driver_register(&bf_plt_driver);
    if(ret)
    {
        BF_LOG ("platform_driver_register for blfp failed!");
        return -1;
    }
   
    return ret;
}


static void  bf_plt_exit(void)
{
    BF_LOG ("exit !");
}

late_initcall_sync(bf_plt_init);
module_exit(bf_plt_exit);
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR ("betterlife@blestech.com");
MODULE_DESCRIPTION ("Betterlife fingerprint sensor driver.");
