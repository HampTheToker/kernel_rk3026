/*
 * Copyright (C) 2013 ROCKCHIP, Inc.
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/skbuff.h>
#include <linux/spi/spi.h>
#include <linux/mmc/host.h>
#include <linux/ion.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <linux/rk_fb.h>
#include <linux/regulator/machine.h>
#include <linux/rfkill-rk.h>
#include <linux/sensor-dev.h>
#include <linux/mfd/tps65910.h>
#include <linux/regulator/act8846.h>
#include <linux/regulator/act8931.h>
#include <linux/regulator/rk29-pwm-regulator.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/hardware/gic.h>

#include <mach/dvfs.h>
#include <mach/board.h>
#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#if defined(CONFIG_MODEM_SOUND)
#include "../../../drivers/misc/modem_sound.h"
#include "../../../drivers/headset_observe/rk_headset.h"
#endif
#if defined(CONFIG_SPIM_RK29)
#include "../../../drivers/spi/rk29_spim.h"
#endif

#ifdef CONFIG_SND_SOC_RK3028A_PHONE
#include "../../../sound/soc/codecs/rk3028a_phone.h"
#endif

#if defined(CONFIG_RK_HDMI)
        #include "../../../drivers/video/rockchip/hdmi/rk_hdmi.h"
#endif

#if defined(CONFIG_GPS_RK)
#include "../../../drivers/misc/gps/rk_gps/rk_gps.h"
#endif
#include "board-rk3028a-86v-camera.c"

#if defined (CONFIG_BP_AUTO)
#include <linux/bp-auto.h>
#endif

#if defined(CONFIG_ANDROID_TIMED_GPIO)
#include "../../../drivers/staging/android/timed_gpio.h"
#endif

#if defined (CONFIG_BP_AUTO)
/* Android Parameter */
static int ap_mdm = 0;
module_param(ap_mdm, int, 0644);
static int ap_has_alsa = 0;
module_param(ap_has_alsa, int, 0644);
static int ap_data_only = 2;
module_param(ap_data_only, int, 0644);
static int ap_has_earphone = 0;
module_param(ap_has_earphone, int, 0644);
#endif

/***********************************************************
*	board config
************************************************************/
//system power on
#define POWER_ON_PIN		RK30_PIN1_PA1  //PWR_HOLD

//touchscreen
#define TOUCH_RST_PIN		RK30_PIN3_PC3
#define TOUCH_RST_VALUE		GPIO_HIGH
#define TOUCH_PWR_PIN		INVALID_GPIO //RK30_PIN2_PD0
#define TOUCH_PWR_VALUE		GPIO_LOW
#define TOUCH_INT_PIN		RK30_PIN3_PC7

//backlight
#define LCD_DISP_ON_PIN
#define BL_PWM			0  // (0 ~ 2)
#define PWM_EFFECT_VALUE  	0
#define BL_EN_PIN		RK30_PIN3_PC5
#define BL_EN_VALUE		GPIO_HIGH

//fb
#define LCD_EN_PIN		RK30_PIN3_PD2
#define LCD_EN_VALUE		GPIO_LOW
#define LCD_CS_PIN		INVALID_GPIO
#define LCD_CS_VALUE		GPIO_LOW

//gsensor
#define GS_INT_PIN		RK30_PIN3_PD1

//sdmmc
//Reference to board-rk3028a-tb-sdmmc-config.c

//keyboard
//#define RK31XX_MAINBOARD_V1      //if mainboard is RK31XX_MAINBOARD_V1.0
#define PLAY_ON_PIN		RK30_PIN0_PD4	//wakeup key		

//pwm regulator
#ifdef CONFIG_PWM_LOGIC_WITH_ARM
#define REG_PWM_LOGIC			PWM_NULL // (0 ~ 2)
#define REG_PWM_ARM			1 // (0 ~ 2)
#else
#define REG_PWM_LOGIC			1 // (0 ~ 2)
#define REG_PWM_ARM			0 // (0 ~ 2)
#endif

//pmic
#define PMU_INT_PIN		RK30_PIN3_PC6
#define PMU_SLEEP_PIN		RK30_PIN0_PC6

//ion reserve memory
#define ION_RESERVE_SIZE        (80 * SZ_1M)

static int pwm_mode[] = {PWM0, PWM1, PWM2};
static inline int rk_gpio_request(int gpio, int direction, int value, const char *label)
{
	int ret = 0;
	unsigned long flags = 0;

	if(!gpio_is_valid(gpio))
		return 0;

	if(direction == GPIOF_DIR_IN)
		flags = GPIOF_IN;
	else if(value == GPIO_LOW)
		flags = GPIOF_OUT_INIT_LOW;
	else
		flags = GPIOF_OUT_INIT_HIGH;

	ret = gpio_request_one(gpio, flags, label);
	if(ret < 0)
		pr_err("Failed to request '%s'\n", label);

	return ret;
}

static struct spi_board_info board_spi_devices[] = {
};

/***********************************************************
*	touchscreen
************************************************************/
#if defined(CONFIG_TOUCHSCREEN_GSLX680_RK3028) || defined(CONFIG_TOUCHSCREEN_GSLX680_RK3028_780) 
//#define TOUCH_RESET_PIN RK30_PIN0_PC1
//#define TOUCH_EN_PIN NULL
//#define TOUCH_INT_PIN RK30_PIN0_PB4
	
	int gslx680_init_platform_hw(void)
	{
	
	       if(gpio_request(TOUCH_RST_PIN,NULL) != 0){
			gpio_free(TOUCH_RST_PIN);
			printk("gslx680_init_platform_hw gpio_request error\n");
			return -EIO;
		}
		if(gpio_request(TOUCH_INT_PIN,NULL) != 0){
			gpio_free(TOUCH_INT_PIN);
			printk("gslx680_init_platform_hw  gpio_request error\n");
			return -EIO;
		}
		gpio_direction_output(TOUCH_RST_PIN, TOUCH_RST_VALUE);
		mdelay(10);
		gpio_set_value(TOUCH_RST_PIN,!TOUCH_RST_VALUE);
		mdelay(10);
		gpio_set_value(TOUCH_RST_PIN,TOUCH_RST_VALUE);
		msleep(300);
		return 0;
	
	}
	
	struct ts_hw_data     gslx680_info = {
		//.reset_gpio = TOUCH_RST_PIN,
		.touch_en_gpio = TOUCH_INT_PIN,
		.init_platform_hw = gslx680_init_platform_hw,
	};
#endif


#if defined(CONFIG_TOUCHSCREEN_GT8XX)
static int goodix_init_platform_hw(void)
{
	int ret  = 0;

	ret = rk_gpio_request(TOUCH_PWR_PIN, GPIOF_DIR_OUT, TOUCH_PWR_VALUE, "touch_pwr");
	if(ret < 0)
		return ret; 
	msleep(100);

	ret = rk_gpio_request(TOUCH_RST_PIN, GPIOF_DIR_OUT, TOUCH_RST_VALUE, "touch_rst");
	if(ret < 0)
		return ret; 
	msleep(100);

	return 0;
}

struct goodix_platform_data goodix_info = {
	.model = 8105,
	.irq_pin = TOUCH_INT_PIN,
	.rest_pin = TOUCH_RST_PIN,
	.init_platform_hw = goodix_init_platform_hw,
};
#endif

/***********************************************************
*   support  modem
************************************************************/
#if defined(CONFIG_BP_AUTO)
static int bp_io_init(void)
{
	iomux_set(RK30_PIN1_PA4);
	iomux_set(RK30_PIN0_PD6);
	iomux_set(RK30_PIN1_PA2);
	return 0;
}

static int bp_io_deinit(void)
{
	
	return 0;
}
static int bp_id_get(void)
{	
	return ap_mdm;   //internally 3G modem ID, defined in  include\linux\Bp-auto.h
}

struct bp_platform_data bp_auto_info = {
	.init_platform_hw 	= bp_io_init,	
	.exit_platform_hw 	= bp_io_deinit,
	.get_bp_id              = bp_id_get,
	.bp_power 		= BP_UNKNOW_DATA,//RK30_PIN6_PB2, 	// 3g_power
	.bp_en 			= BP_UNKNOW_DATA,//RK30_PIN2_PB6, 	// 3g_en
	.bp_reset			= BP_UNKNOW_DATA,//RK30_PIN4_PD2,
	.bp_usb_en 		= BP_UNKNOW_DATA, 	//W_disable
	.bp_uart_en 		= BP_UNKNOW_DATA, 	//EINT9
	.bp_wakeup_ap 	= BP_UNKNOW_DATA,//RK30_PIN4_PC6,	//
	.ap_wakeup_bp 	= BP_UNKNOW_DATA,//RK30_PIN4_PC4,
	.ap_ready 		= BP_UNKNOW_DATA,	//
	.bp_ready		= BP_UNKNOW_DATA,
	.gpio_valid 		= 0,		//if 1:gpio is define in bp_auto_info,if 0:is not use gpio in bp_auto_info
};

struct platform_device device_bp_auto = {	
        .name = "bp-auto",	
    	.id = -1,	
	.dev		= {
		.platform_data = &bp_auto_info,
	}    	
    };
#endif
/***********************************************************
*	rk30  backlight
************************************************************/
#ifdef CONFIG_BACKLIGHT_RK29_BL
static int rk29_backlight_io_init(void)
{
	int ret = 0;

	iomux_set(pwm_mode[BL_PWM]);
	msleep(50);
#ifdef  LCD_DISP_ON_PIN
	ret = rk_gpio_request(BL_EN_PIN, GPIOF_DIR_OUT, BL_EN_VALUE, "bl_en");
	if(ret < 0)
		return ret;
	msleep(50);
	gpio_set_value(BL_EN_PIN,BL_EN_VALUE);
#endif
	return 0;
}

static int rk29_backlight_io_deinit(void)
{
	int pwm_gpio;
#ifdef  LCD_DISP_ON_PIN
	gpio_free(BL_EN_PIN);
#endif
	pwm_gpio = iomux_mode_to_gpio(pwm_mode[BL_PWM]);
	return rk_gpio_request(BL_EN_PIN, GPIOF_DIR_OUT, GPIO_LOW, "BL_PWM");
}

static int rk29_backlight_pwm_suspend(void)
{
	int ret, pwm_gpio = iomux_mode_to_gpio(pwm_mode[BL_PWM]);

	ret = rk_gpio_request(pwm_gpio, GPIOF_DIR_OUT, GPIO_LOW, "BL_PWM");
	if(ret < 0)
		return ret;
#ifdef  LCD_DISP_ON_PIN
	gpio_direction_output(BL_EN_PIN, !BL_EN_VALUE);
#endif
	return ret;
}

static int rk29_backlight_pwm_resume(void)
{
	int pwm_gpio = iomux_mode_to_gpio(pwm_mode[BL_PWM]);

	gpio_free(pwm_gpio);
	iomux_set(pwm_mode[BL_PWM]);
#ifdef  LCD_DISP_ON_PIN
	msleep(30);
	gpio_direction_output(BL_EN_PIN, BL_EN_VALUE);
#endif
	return 0;
}

static struct rk29_bl_info rk29_bl_info = {
	.pwm_id = BL_PWM,
	.min_brightness=60,
	.max_brightness=255,
	.brightness_mode = BRIGHTNESS_MODE_CONIC,
	.bl_ref = PWM_EFFECT_VALUE,
	.io_init = rk29_backlight_io_init,
	.io_deinit = rk29_backlight_io_deinit,
	.pwm_suspend = rk29_backlight_pwm_suspend,
	.pwm_resume = rk29_backlight_pwm_resume,
};

static struct platform_device rk29_device_backlight = {
	.name	= "rk29_backlight",
	.id 	= -1,
	.dev	= {
		.platform_data  = &rk29_bl_info,
	}
};
#endif

/***********************************************************
*	fb
************************************************************/
#ifdef CONFIG_FB_ROCKCHIP
static int rk_fb_io_init(struct rk29_fb_setting_info *fb_setting)
{
	int ret = 0;

	ret = rk_gpio_request(LCD_CS_PIN, GPIOF_DIR_OUT, LCD_CS_VALUE, "lcd_cs");
	if(ret < 0)
		return ret;

	return rk_gpio_request(LCD_EN_PIN, GPIOF_DIR_OUT, LCD_EN_VALUE, "lcd_en");
}

static int rk_fb_io_disable(void)
{
	gpio_set_value(LCD_CS_PIN, !LCD_CS_VALUE);
	gpio_set_value(LCD_EN_PIN, !LCD_EN_VALUE);

	return 0;
}

static int rk_fb_io_enable(void)
{
	gpio_set_value(LCD_CS_PIN, LCD_CS_VALUE);
	gpio_set_value(LCD_EN_PIN, LCD_EN_VALUE);

	return 0;
}

#if defined(CONFIG_LCDC0_RK3066B) || defined(CONFIG_LCDC0_RK3188)
struct rk29fb_info lcdc0_screen_info = {
#if defined(CONFIG_RK_HDMI) && defined(CONFIG_HDMI_SOURCE_LCDC0) && defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
	.prop	   = EXTEND,	//extend display device
	.io_init    = NULL,
	.io_disable = NULL,
	.io_enable = NULL,
	.set_screen_info = hdmi_init_lcdc,
#else
	.prop	   = PRMRY,		//primary display device
	.io_init   = rk_fb_io_init,
	.io_disable = rk_fb_io_disable,
	.io_enable = rk_fb_io_enable,
	.set_screen_info = set_lcd_info,
#endif
};
#endif

#if defined(CONFIG_LCDC1_RK3066B) || defined(CONFIG_LCDC1_RK3188)
struct rk29fb_info lcdc1_screen_info = {
#if defined(CONFIG_RK_HDMI) && defined(CONFIG_HDMI_SOURCE_LCDC1) && defined(CONFIG_DUAL_LCDC_DUAL_DISP_IN_KERNEL)
	.prop	   = EXTEND,	//extend display device
	.io_init    = NULL,
	.io_disable = NULL,
	.io_enable = NULL,
	.set_screen_info = hdmi_init_lcdc,
#else
	.prop	   = PRMRY,		//primary display device
	.io_init   = rk_fb_io_init,
	.io_disable = rk_fb_io_disable,
	.io_enable = rk_fb_io_enable,
	.set_screen_info = set_lcd_info,
#endif
};
#endif

static struct resource resource_fb[] = {
	[0] = {
		.name  = "fb0 buf",
		.start = 0,
		.end   = 0,//RK30_FB0_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "ipp buf",  //for rotate
		.start = 0,
		.end   = 0,//RK30_FB0_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.name  = "fb2 buf",
		.start = 0,
		.end   = 0,//RK30_FB0_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device device_fb = {
	.name		= "rk-fb",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resource_fb),
	.resource	= resource_fb,
};
#endif

#if defined(CONFIG_LCDC0_RK3066B) || defined(CONFIG_LCDC0_RK3188)
static struct resource resource_lcdc0[] = {
	[0] = {
		.name  = "lcdc0 reg",
		.start = RK3026_LCDC0_PHYS,
		.end   = RK3026_LCDC0_PHYS + RK3026_LCDC0_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	
	[1] = {
		.name  = "lcdc0 irq",
		.start = IRQ_LCDC,
		.end   = IRQ_LCDC,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device device_lcdc0 = {
	.name		  = "rk30-lcdc",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(resource_lcdc0),
	.resource	  = resource_lcdc0,
	.dev 		= {
		.platform_data = &lcdc0_screen_info,
	},
};
#endif

#if defined(CONFIG_LCDC1_RK3066B) || defined(CONFIG_LCDC1_RK3188)
static struct resource resource_lcdc1[] = {
	[0] = {
		.name  = "lcdc1 reg",
		.start = RK3026_LCDC1_PHYS,
		.end   = RK3026_LCDC1_PHYS + RK3026_LCDC1_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "lcdc1 irq",
		.start = IRQ_LCDC1,
		.end   = IRQ_LCDC1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device device_lcdc1 = {
	.name		  = "rk30-lcdc",
	.id		  = 1,
	.num_resources	  = ARRAY_SIZE(resource_lcdc1),
	.resource	  = resource_lcdc1,
	.dev 		= {
		.platform_data = &lcdc1_screen_info,
	},
};
#endif

static int rk_platform_add_display_devices(void)
{
	struct platform_device *fb = NULL;  //fb
	struct platform_device *lcdc0 = NULL; //lcdc0
	struct platform_device *lcdc1 = NULL; //lcdc1
	struct platform_device *bl = NULL; //backlight
#ifdef CONFIG_FB_ROCKCHIP
	fb = &device_fb;
#endif

#if defined(CONFIG_LCDC0_RK3066B) || defined(CONFIG_LCDC0_RK3188)
	lcdc0 = &device_lcdc0,
#endif

#if defined(CONFIG_LCDC1_RK3066B) || defined(CONFIG_LCDC1_RK3188)
	lcdc1 = &device_lcdc1,
#endif

#ifdef CONFIG_BACKLIGHT_RK29_BL
	bl = &rk29_device_backlight,
#endif
	__rk_platform_add_display_devices(fb,lcdc0,lcdc1,bl);

	return 0;
}


/***********************************************************
*	gsensor
************************************************************/
// mma 8452 gsensor
#if defined (CONFIG_GS_MMA8452)
#define MMA8452_INT_PIN GS_INT_PIN
static int mma8452_init_platform_hw(void)
{
	return 0;
}

static struct sensor_platform_data mma8452_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
        .init_platform_hw = mma8452_init_platform_hw,
        .orientation = {-1, 0, 0, 0, -1, 0, 0, 0, 1},
};
#endif

// lsm303d gsensor
#if defined (CONFIG_GS_LSM303D)
#define LSM303D_INT_PIN GS_INT_PIN
static int lms303d_init_platform_hw(void)
{
	return 0;
}

static struct sensor_platform_data lms303d_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
        .init_platform_hw = lms303d_init_platform_hw,
        .orientation = {-1, 0, 0, 0, -1, 0, 0, 0, 1},
};
#endif


/*MMA7660 gsensor*/
#if defined (CONFIG_GS_MMA7660)
#define MMA7660_INT_PIN   GS_INT_PIN

static int mma7660_init_platform_hw(void)
{
	//rk30_mux_api_set(GPIO1B2_SPI_RXD_UART1_SIN_NAME, GPIO1B_GPIO1B2);

	return 0;
}

static struct sensor_platform_data mma7660_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
    .init_platform_hw = mma7660_init_platform_hw,
#ifndef CONFIG_MFD_RK616
	#ifdef CONFIG_TOUCHSCREEN_GSLX680_RK3168
	.orientation = {-1, 0, 0, 0, -1, 0, 0, 0, 1},
	#else
    .orientation = {0, -1, 0, -1, 0, 0, 0, 0, -1},
    #endif  
#else
	.orientation = {1, 0, 0, 0, -1, 0, 0, 0, -1},
#endif	
};
#endif


#if defined (CONFIG_GS_MXC6225)
#define MXC6225_INT_PIN   GS_INT_PIN

static int mxc6225_init_platform_hw(void)
{
        return 0;
}

static struct sensor_platform_data mxc6225_info = {
        .type = SENSOR_TYPE_ACCEL,
        .irq_enable = 0,
        .poll_delay_ms = 30,
        .init_platform_hw = mxc6225_init_platform_hw,
        .orientation = { 0, -1, 0, 1, 0, 0, 0, 0, 0},
};
#endif
#if defined (CONFIG_GS_LIS3DH)
#define LIS3DH_INT_PIN   GS_INT_PIN
	
	static int lis3dh_init_platform_hw(void)
	{
	
		return 0;
	}
	
	static struct sensor_platform_data lis3dh_info = {
		.type = SENSOR_TYPE_ACCEL,
		.irq_enable = 1,
		.poll_delay_ms = 30,
		.init_platform_hw = lis3dh_init_platform_hw,
		.orientation = {1, 0, 0, 0, 1, 0, 0, 0, 1},
	};
#endif

#if defined (CONFIG_COMPASS_AK8975)
static struct sensor_platform_data akm8975_info =
{
	.type = SENSOR_TYPE_COMPASS,
	.irq_enable = 1,
	.poll_delay_ms = 30,
	.m_layout = 
	{
		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},
	}
};

#endif

#if defined (CONFIG_COMPASS_AK8963)
static struct sensor_platform_data akm8963_info =
{
       .type = SENSOR_TYPE_COMPASS,
       .irq_enable = 1,
       .poll_delay_ms = 30,
       .m_layout = 
       {
               {
                       {0, 1, 0},
                       {1, 0, 0},
                       {0, 0, -1},
               },

               {
                       {1, 0, 0},
                       {0, 1, 0},
                       {0, 0, 1},
               },

               {
                       {0, -1, 0},
                       {-1, 0, 0},
                       {0, 0, -1},
               },

               {
                       {1, 0, 0},
                       {0, 1, 0},
                       {0, 0, 1},
               },
       }
};

#endif


#if defined(CONFIG_GYRO_L3G4200D)

#include <linux/l3g4200d.h>
#define L3G4200D_INT_PIN  RK30_PIN0_PB4

static int l3g4200d_init_platform_hw(void)
{
	return 0;
}

static struct sensor_platform_data l3g4200d_info = {
	.type = SENSOR_TYPE_GYROSCOPE,
	.irq_enable = 1,
	.poll_delay_ms = 30,
	.orientation = {0, 1, 0, -1, 0, 0, 0, 0, 1},
	.init_platform_hw = l3g4200d_init_platform_hw,
	.x_min = 40,//x_min,y_min,z_min = (0-100) according to hardware
	.y_min = 40,
	.z_min = 20,
};

#endif

#ifdef CONFIG_LS_CM3217
static struct sensor_platform_data cm3217_info = {
	.type = SENSOR_TYPE_LIGHT,
	.irq_enable = 0,
	.poll_delay_ms = 500,
};

#endif

/***********************************************************
*	keyboard
************************************************************/
#include <plat/key.h>

static struct rk29_keys_button key_button[] = {
        {
                .desc   = "play",
                .code   = KEY_POWER,
                .gpio   = PLAY_ON_PIN,
                .active_low = PRESS_LEV_LOW,
                .wakeup = 1,
        },
	{
		.desc	= "vol+",
		.code	= KEY_VOLUMEUP,
		.gpio = INVALID_GPIO,
		.adc_value	= 1,
		.active_low = PRESS_LEV_LOW,
	},
	{
		.desc	= "vol-",
		.code	= KEY_VOLUMEDOWN,
		.gpio = INVALID_GPIO,
		.adc_value	= 512,
		.active_low = PRESS_LEV_LOW,
	},

};

struct rk29_keys_platform_data rk29_keys_pdata = {
	.buttons	= key_button,
	.nbuttons	= ARRAY_SIZE(key_button),
	.chn	= 1,  //chn: 0-7, if do not use ADC,set 'chn' -1
};

#if defined(CONFIG_MODEM_SOUND)

struct modem_sound_data modem_sound_info = {
	.spkctl_io = RK2928_PIN3_PD4,
	.modemctl_io = INVALID_GPIO,
	.spkctl_active = GPIO_HIGH,
	.modemctl_active = GPIO_HIGH,
};

struct platform_device modem_sound_device = {
	.name = "modem_sound",
	.id = -1,
	.dev		= {
	.platform_data = &modem_sound_info,
		}
	};
#endif

/***********************************************************
*	sdmmc
************************************************************/
#ifdef CONFIG_SDMMC_RK29
#include "board-rk3028a-86v-sdmmc-config.c"
#include "../plat-rk/rk-sdmmc-ops.c"
#include "../plat-rk/rk-sdmmc-wifi.c"
#endif //endif ---#ifdef CONFIG_SDMMC_RK29

#ifdef CONFIG_SDMMC0_RK29
#define CONFIG_SDMMC0_USE_DMA   
static int rk29_sdmmc0_cfg_gpio(void)
{
	rk29_sdmmc_set_iomux(0, 0xFFFF);
	#if defined(CONFIG_SDMMC0_RK29_SDCARD_DET_FROM_GPIO)
        iomux_set_gpio_mode(iomux_gpio_to_mode(RK29SDK_SD_CARD_DETECT_N));
    	#else
        iomux_set(MMC0_DETN);
    	#endif	

	#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
	gpio_request(SDMMC0_WRITE_PROTECT_PIN, "sdmmc-wp");
	gpio_direction_input(SDMMC0_WRITE_PROTECT_PIN);
	#endif
	return 0;
}

struct rk29_sdmmc_platform_data default_sdmmc0_data = {
	.host_ocr_avail =
	    (MMC_VDD_25_26 | MMC_VDD_26_27 | MMC_VDD_27_28 | MMC_VDD_28_29 |
	     MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
	     MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36),
	.host_caps =
	    (MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
	.io_init = rk29_sdmmc0_cfg_gpio,

	.set_iomux = rk29_sdmmc_set_iomux,

	.dma_name = "sd_mmc",
#ifdef CONFIG_SDMMC0_USE_DMA
	.use_dma = 1,
#else
	.use_dma = 0,
#endif

#if defined(CONFIG_WIFI_COMBO_MODULE_CONTROL_FUNC) && defined(CONFIG_USE_SDMMC0_FOR_WIFI_DEVELOP_BOARD)
	.status = rk29sdk_wifi_mmc0_status,
	.register_status_notify = rk29sdk_wifi_mmc0_status_register,
#endif

#if defined(RK29SDK_SD_CARD_PWR_EN) || (INVALID_GPIO != RK29SDK_SD_CARD_PWR_EN)
	.power_en = RK29SDK_SD_CARD_PWR_EN,
	.power_en_level = RK29SDK_SD_CARD_PWR_EN_LEVEL,
#else
	.power_en = INVALID_GPIO,
	.power_en_level = GPIO_LOW,
#endif    
	.enable_sd_wakeup = 0,

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
	.write_prt = SDMMC0_WRITE_PROTECT_PIN,
	.write_prt_enalbe_level = SDMMC0_WRITE_PROTECT_ENABLE_VALUE;
#else
	.write_prt = INVALID_GPIO,
#endif

	.det_pin_info = {    
    		#if defined(RK29SDK_SD_CARD_DETECT_N) || (INVALID_GPIO != RK29SDK_SD_CARD_DETECT_N)  
        	.io             = RK29SDK_SD_CARD_DETECT_N, //INVALID_GPIO,
        	.enable         = RK29SDK_SD_CARD_INSERT_LEVEL,
    		#else
        	.io             = INVALID_GPIO,
        	.enable         = GPIO_LOW,
    		#endif    
    	}, 

};
#endif // CONFIG_SDMMC0_RK29

#ifdef CONFIG_SDMMC1_RK29
#define CONFIG_SDMMC1_USE_DMA
static int rk29_sdmmc1_cfg_gpio(void)
{
#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
	gpio_request(SDMMC1_WRITE_PROTECT_PIN, "sdio-wp");
	gpio_direction_input(SDMMC1_WRITE_PROTECT_PIN);
#endif
	return 0;
}

struct rk29_sdmmc_platform_data default_sdmmc1_data = {
	.host_ocr_avail =
	    (MMC_VDD_25_26 | MMC_VDD_26_27 | MMC_VDD_27_28 | MMC_VDD_28_29 |
	     MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
	     MMC_VDD_33_34),

#if !defined(CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD)
	.host_caps = (MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ |
		      MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
#else
	.host_caps =
	    (MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
#endif

	.io_init = rk29_sdmmc1_cfg_gpio,

	.set_iomux = rk29_sdmmc_set_iomux,

	.dma_name = "sdio",
#ifdef CONFIG_SDMMC1_USE_DMA
	.use_dma = 1,
#else
	.use_dma = 0,
#endif

#if defined(CONFIG_WIFI_CONTROL_FUNC) || defined(CONFIG_WIFI_COMBO_MODULE_CONTROL_FUNC)
	.status = rk29sdk_wifi_status,
	.register_status_notify = rk29sdk_wifi_status_register,
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
	.write_prt = SDMMC1_WRITE_PROTECT_PIN,
	.write_prt_enalbe_level = SDMMC1_WRITE_PROTECT_ENABLE_VALUE;
#else
	.write_prt = INVALID_GPIO,
#endif

    #if defined(CONFIG_RK29_SDIO_IRQ_FROM_GPIO)
	.sdio_INT_gpio = RK29SDK_WIFI_SDIO_CARD_INT,
    #endif

	.det_pin_info = {    
	#if defined(CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD)
		#if defined(RK29SDK_SD_CARD_DETECT_N) || (INVALID_GPIO != RK29SDK_SD_CARD_DETECT_N)  
        	.io             = RK29SDK_SD_CARD_DETECT_N,
     		#else
         	.io             = INVALID_GPIO,
		#endif   

        	.enable         = RK29SDK_SD_CARD_INSERT_LEVEL,
	#else
        	.io             = INVALID_GPIO,
        	.enable         = GPIO_LOW,
	#endif
	},
	.enable_sd_wakeup = 0,
};
#endif //endif--#ifdef CONFIG_SDMMC1_RK29

#ifdef CONFIG_ANDROID_TIMED_GPIO
static struct timed_gpio timed_gpios[] = {
	{
		.name = "vibrator",
		.gpio = RK30_PIN1_PB6,
		.max_timeout = 1000,
		.active_low = 0,
		.adjust_time =20,      //adjust for diff product
	},
};

static struct timed_gpio_platform_data rk29_vibrator_info = {
	.num_gpios = 1,
	.gpios = timed_gpios,
};

static struct platform_device rk29_device_vibrator = {
	.name = "timed-gpio",
	.id = -1,
	.dev = {
		.platform_data = &rk29_vibrator_info,
	},

};
#endif


#ifdef CONFIG_BATTERY_RK30_ADC_FAC
static struct rk30_adc_battery_platform_data rk30_adc_battery_platdata = {
        .dc_det_pin      = RK30_PIN1_PB4,
        .batt_low_pin    = INVALID_GPIO, 
        .charge_set_pin  = INVALID_GPIO,
        .charge_ok_pin   = RK30_PIN1_PA0,
	 .usb_det_pin = INVALID_GPIO,
        .dc_det_level    = GPIO_LOW,
        .charge_ok_level = GPIO_HIGH,

	.reference_voltage = 3300, // the rK2928 is 3300;RK3066 and rk29 are 2500;rk3066B is 1800;
       .pull_up_res = 200,     //divider resistance ,  pull-up resistor
       .pull_down_res = 120, //divider resistance , pull-down resistor

	.is_reboot_charging = 1,
        .save_capacity   = 1 ,
        .low_voltage_protection = 3600,    
};

static struct platform_device rk30_device_adc_battery = {
        .name   = "rk30-battery",
        .id     = -1,
        .dev = {
                .platform_data = &rk30_adc_battery_platdata,
        },
};
#endif


/***********************************************************
*	rfkill
************************************************************/
#ifdef CONFIG_RFKILL_RK
// bluetooth rfkill device, its driver in net/rfkill/rfkill-rk.c
static struct rfkill_rk_platform_data rfkill_rk_platdata = {
	.type               = RFKILL_TYPE_BLUETOOTH,

	.poweron_gpio       = { // BT_REG_ON
		.io             = INVALID_GPIO, //RK30_PIN3_PC7,
		.enable         = GPIO_HIGH,
		.iomux          = {
			.name       = "bt_poweron",
			//.fgpio      = GPIO3_C7,
		},
	},

	.reset_gpio         = { // BT_RST
		.io             = RK30_PIN3_PC7, // set io to INVALID_GPIO for disable it
		.enable         = GPIO_LOW,
		.iomux          = {
			.name       = "bt_reset",
			.fgpio      = GPIO1_B3,
		},
	}, 

	.wake_gpio          = { // BT_WAKE, use to control bt's sleep and wakeup
		.io             = RK30_PIN0_PC4, // set io to INVALID_GPIO for disable it
		.enable         = GPIO_HIGH,
		.iomux          = {
			.name       = "bt_wake",
			.fgpio      = GPIO1_B2,
		},
	},

	.wake_host_irq      = { // BT_HOST_WAKE, for bt wakeup host when it is in deep sleep
		.gpio           = {
			.io         = RK30_PIN0_PA4, // set io to INVALID_GPIO for disable it
			.enable     = GPIO_LOW,      // set GPIO_LOW for falling, set 0 for rising
			.iomux      = {
				.name   = "bt_wake_host",
				//.fgpio  = GPIO0_A4,  
			},
		},
	},

	.rts_gpio           = { // UART_RTS, enable or disable BT's data coming
		.io             = RK30_PIN1_PA3, // set io to INVALID_GPIO for disable it
		.enable         = GPIO_LOW,
		.iomux          = {
			.name       = "bt_rts",
			.fgpio      = GPIO1_A3,
			.fmux       = UART0_RTSN,
		},
	}
};

static struct platform_device device_rfkill_rk = {
    .name   = "rfkill_rk",
    .id     = -1,
    .dev    = {
        .platform_data = &rfkill_rk_platdata,
    },
};
#endif

/***********************************************************
*	ion
************************************************************/
#ifdef CONFIG_ION
static struct ion_platform_data rk30_ion_pdata = {
	.nr = 1,
	.heaps = {
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_NOR_HEAP_ID,
			.name = "norheap",
			.size = ION_RESERVE_SIZE,
		}
	},
};

static struct platform_device device_ion = {
	.name = "ion-rockchip",
	.id = 0,
	.dev = {
		.platform_data = &rk30_ion_pdata,
	},
};
#endif

/***********************************************************
*	pwm regulator
************************************************************/
#ifdef CONFIG_RK30_PWM_REGULATOR
static int pwm_voltage_map[] = {
	800000,  825000,  850000,  875000,  900000,  925000 ,
	950000,  975000,  1000000, 1025000, 1050000, 1075000, 
	1100000, 1125000, 1150000, 1175000, 1200000, 1225000, 
	1250000, 1275000, 1300000, 1325000, 1350000, 1375000,
	1400000
};



static struct regulator_consumer_supply pwm_dcdc1_consumers[] = {
#ifdef CONFIG_PWM_LOGIC_WITH_ARM
	{
		.supply = "vdd_cpu",
	}
#else
	{
		.supply = "vdd_core",
	}
#endif
};

static struct regulator_consumer_supply pwm_dcdc2_consumers[] = {
#ifdef CONFIG_PWM_LOGIC_WITH_ARM
	{
		.supply = "vdd_core",
	}
#else
	{
		.supply = "vdd_cpu",
	}
#endif
};

struct regulator_init_data pwm_regulator_init_dcdc[2] = {
	{
		.constraints = {
			.name = "PWM_DCDC1",
			.min_uV = 600000,
			.max_uV = 1800000,      //0.6-1.8V
			.apply_uV = true,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
		},
		.num_consumer_supplies = ARRAY_SIZE(pwm_dcdc1_consumers),
		.consumer_supplies = pwm_dcdc1_consumers,
	},
	{
		.constraints = {
			.name = "PWM_DCDC2",
			.min_uV = 600000,
			.max_uV = 1800000,      //0.6-1.8V
			.apply_uV = true,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
		},
		.num_consumer_supplies = ARRAY_SIZE(pwm_dcdc2_consumers),
		.consumer_supplies = pwm_dcdc2_consumers,
	},
};

static struct pwm_platform_data pwm_regulator_info[2] = {
	{
#ifdef CONFIG_PWM_LOGIC_WITH_ARM
		.pwm_id = REG_PWM_ARM,
#else
		.pwm_id = REG_PWM_LOGIC,
#endif
		.pwm_voltage = 1200000,
		.suspend_voltage = 1050000,
#ifdef CONFIG_PWM_LOGIC_WITH_ARM
		.min_uV = 900000,
		.max_uV = 1400000,
#else
		.min_uV = 800000,
		.max_uV = 1375000,
#endif
		.coefficient = 480,     //50.4%
		.pwm_voltage_map = pwm_voltage_map,
		.init_data      = &pwm_regulator_init_dcdc[0],
	},
	{
#ifdef CONFIG_PWM_LOGIC_WITH_ARM
		.pwm_id = REG_PWM_LOGIC,
#else
		.pwm_id = REG_PWM_ARM,
#endif
		.pwm_voltage = 1200000,
		.suspend_voltage = 1050000,
		.min_uV = 900000,
		.max_uV = 1400000,
		.coefficient = 480,     //50.4%
		.pwm_voltage_map = pwm_voltage_map,
		.init_data      = &pwm_regulator_init_dcdc[1],
	},
};
struct platform_device pwm_regulator_device[2] = {
	{
		.name = "pwm-voltage-regulator",
		.id = 0,
		.dev            = {
			.platform_data = &pwm_regulator_info[0],
		}
	},
	{
		.name = "pwm-voltage-regulator",
		.id = 1,
		.dev            = {
			.platform_data = &pwm_regulator_info[1],
		}
	},
};

static void pwm_regulator_init(void)
{
#ifdef CONFIG_PWM_LOGIC_WITH_ARM
	pwm_regulator_info[0].pwm_gpio = iomux_mode_to_gpio(pwm_mode[REG_PWM_ARM]);;
	pwm_regulator_info[0].pwm_iomux_pwm = pwm_mode[REG_PWM_ARM];
	pwm_regulator_info[0].pwm_iomux_gpio = iomux_switch_gpio_mode(pwm_mode[REG_PWM_ARM]);
#else
	pwm_regulator_info[0].pwm_gpio = iomux_mode_to_gpio(pwm_mode[REG_PWM_LOGIC]);;
	pwm_regulator_info[0].pwm_iomux_pwm = pwm_mode[REG_PWM_LOGIC];
	pwm_regulator_info[0].pwm_iomux_gpio = iomux_switch_gpio_mode(pwm_mode[REG_PWM_LOGIC]);
#endif
#ifdef CONFIG_PWM_CONTROL_ARM
	pwm_regulator_info[1].pwm_gpio = iomux_mode_to_gpio(pwm_mode[REG_PWM_ARM]);
	pwm_regulator_info[1].pwm_iomux_pwm = pwm_mode[REG_PWM_ARM];
	pwm_regulator_info[1].pwm_iomux_gpio = iomux_switch_gpio_mode(pwm_mode[REG_PWM_ARM]);
#endif
}
#endif

int __sramdata pwm_iomux_logic, pwm_do_logic, pwm_dir_logic, pwm_en_logic;
int __sramdata pwm_iomux_arm, pwm_do_arm, pwm_dir_arm, pwm_en_arm;
#define grf_readl(offset)       readl_relaxed(RK30_GRF_BASE + offset)
#define grf_writel(v, offset)   do { writel_relaxed(v, RK30_GRF_BASE + offset); dsb(); } while (0)

#define gpio_readl(offset)       readl_relaxed(RK2928_GPIO0_BASE + offset)
#define gpio_writel(v, offset)   do { writel_relaxed(v, RK2928_GPIO0_BASE + offset); dsb(); } while (0)

#define GPIO_DIR 0x04
#define GPIO_D0  0x00

#define GPIO0_D2_OFFSET		20
void __sramfunc rk30_pwm_logic_suspend_voltage(void)
{
	/* pwm0: GPIO0_D2, pwm1: GPIO0_D3, pwm2: GPIO0_D4 */
	sram_udelay(10000);

#ifdef CONFIG_PWM_CONTROL_LOGIC
#ifdef CONFIG_PWM_LOGIC_WITH_ARM	
	int off = GPIO0_D2_OFFSET + 2*REG_PWM_ARM;
#else
	int off = GPIO0_D2_OFFSET + 2*REG_PWM_LOGIC;
#endif
	pwm_iomux_logic = grf_readl(GRF_GPIO0D_IOMUX);
	pwm_dir_logic = gpio_readl(GPIO_DIR);
	pwm_do_logic = gpio_readl(GPIO_D0);

	grf_writel(1<<off, GRF_GPIO0D_IOMUX);
	gpio_writel(pwm_dir_logic | 0x08000000, GPIO_DIR);
	gpio_writel(pwm_do_logic | 0x08000000, GPIO_D0); 	
#endif

#ifdef CONFIG_PWM_CONTROL_ARM
        off = GPIO0_D2_OFFSET + 2*REG_PWM_ARM;

	pwm_iomux_arm = grf_readl(GRF_GPIO0D_IOMUX);
	pwm_dir_arm = gpio_readl(GPIO_DIR);
	pwm_do_arm = gpio_readl(GPIO_D0);

	grf_writel(1<<off, GRF_GPIO0D_IOMUX);
	gpio_writel(pwm_dir_arm | 0x04000000, GPIO_DIR);
	gpio_writel(pwm_do_arm | 0x04000000, GPIO_D0);	
#endif
}

void __sramfunc rk30_pwm_logic_resume_voltage(void)
{
	/* pwm0: GPIO0_D2, pwm1: GPIO0_D3, pwm2: GPIO0_D4 */

#ifdef CONFIG_PWM_CONTROL_LOGIC
	int off = GPIO0_D2_OFFSET + 2*REG_PWM_LOGIC;
	grf_writel((1<<off)|pwm_iomux_logic, GRF_GPIO0D_IOMUX);
	gpio_writel(pwm_dir_logic, GPIO_DIR);
	gpio_writel(pwm_do_logic, GPIO_D0);
	sram_udelay(10000);
	
#endif
#ifdef CONFIG_PWM_CONTROL_ARM
	off = GPIO0_D2_OFFSET + 2*REG_PWM_ARM;

	grf_writel((1<<off)|pwm_iomux_arm, GRF_GPIO0D_IOMUX);
	gpio_writel(pwm_dir_arm, GPIO_DIR);
	gpio_writel(pwm_do_arm, GPIO_D0);
	sram_udelay(10000);
#endif
}

extern void pwm_suspend_voltage(void);
extern void pwm_resume_voltage(void);

void  rk30_pwm_suspend_voltage_set(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
	pwm_suspend_voltage();
#endif
}

void  rk30_pwm_resume_voltage_set(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
	pwm_resume_voltage();
#endif
}

/***********************************************************
*	pmic
************************************************************/
int __sramdata g_pmic_type =  0;

#ifdef CONFIG_MFD_TPS65910
#define TPS65910_HOST_IRQ 	PMU_INT_PIN
#define PMU_POWER_SLEEP		PMU_SLEEP_PIN
static struct pmu_info  tps65910_dcdc_info[] = {
	{
		.name          = "vdd_core",   //logic
		.min_uv          = 1100000,
		.max_uv         = 1100000,
	},
	{
		.name          = "vdd_cpu",    //arm
		.min_uv          = 1100000,
		.max_uv         = 1100000,
	},
	{
		.name          = "vio",   //vcc_io
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
	
};
static  struct pmu_info  tps65910_ldo_info[] = {
	/*{
		.name          = "vpll",   //vdd10
		.min_uv          = 1000000,
		.max_uv         = 1000000,
	},*/
	{
		.name          = "vdig1",    //vcc18_cif
		.min_uv          = 2800000,//1800000,
		.max_uv         = 2800000,//1800000,
	},
	{
		.name          = "vdig2",   //vdd11
		.min_uv          = 1100000,
		.max_uv         = 1100000,
	},
	{
		.name          = "vaux1",   //vcc28_cif
		.min_uv          = 2800000,
		.max_uv         = 2800000,
	},
	{
		.name          = "vaux2",   //vcc33
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
	{
		.name          = "vaux33",   //vcc_tp
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
	{
		.name          = "vmmc",   //vcca30
		.min_uv          = 3000000,
		.max_uv         = 3000000,
	},
	{
		.name          = "vdac",   //
		.min_uv          = 1800000,
		.max_uv         = 1800000,
	},
};
#include "../mach-rk30/board-pmu-tps65910.c"
#endif

#if defined (CONFIG_RK_HEADSET_DET) || defined (CONFIG_RK_HEADSET_IRQ_HOOK_ADC_DET)
#define MAIN_MIC_ACTIVE  GPIO_LOW
#define MAIN_MIC_IO      RK2928_PIN1_PA5
static int rk_headset_io_init(int gpio, char *iomux_name, int iomux_mode)
{
	int ret;
	ret = gpio_request(gpio, "headset_io");
	if(ret) 
		return ret;

//	rk30_mux_api_set(iomux_name, iomux_mode);
	iomux_set(gpio);
	gpio_pull_updown(gpio, PullDisable);
	gpio_direction_input(gpio);
	mdelay(50);

	if(MAIN_MIC_IO == INVALID_GPIO)
		return ret;
	ret = gpio_request(MAIN_MIC_IO, "sw_mic_io");
        if(ret)
                return ret;
        gpio_direction_output(MAIN_MIC_IO,MAIN_MIC_ACTIVE);

	return 0;
};

static int rk_hook_io_init(int gpio, char *iomux_name, int iomux_mode)
{
	int ret;
	ret = gpio_request(gpio, "hook_io");
	if(ret) 
		return ret;

//	rk30_mux_api_set(iomux_name, iomux_mode);
	iomux_set(gpio);
	gpio_pull_updown(gpio, PullDisable);
	gpio_direction_input(gpio);
	mdelay(50);
	return 0;
};

struct rk_headset_pdata rk_headset_info = {
		.Headset_gpio		= RK2928_PIN1_PA3,
		.Hook_gpio  = RK2928_PIN1_PA7,//0
		.Hook_down_type = HOOK_DOWN_HIGH,
		.headset_in_type = HEADSET_IN_LOW,
		.hook_key_code = KEY_MEDIA,
//		.headset_gpio_info = { GPIO1A3_I2S_LRCKTX_NAME ,GPIO1A_GPIO1A3},
		.headset_io_init = rk_headset_io_init,
//		.hook_gpio_info = { GPIO1A7_MMC0_WRPRT_NAME , GPIO1A_GPIO1A7},
		.hook_io_init = rk_hook_io_init,
		.Main_Mic_value = MAIN_MIC_ACTIVE,
		.Sw_Mic_IO      = MAIN_MIC_IO,
};
struct platform_device rk_device_headset = {
		.name	= "rk_headsetdet",
		.id 	= 0,
		.dev    = {
			    .platform_data = &rk_headset_info,
		}
};
#endif
void __sramfunc board_pmu_suspend(void)
{
	#if defined (CONFIG_MFD_TPS65910)
	if(pmic_is_tps65910())
		board_pmu_tps65910_suspend();
	#endif
}

void __sramfunc board_pmu_resume(void)
{
	#if defined (CONFIG_MFD_TPS65910)
	if(pmic_is_tps65910())
		board_pmu_tps65910_resume();
	#endif
}

#ifdef CONFIG_SND_SOC_RK3028A_PHONE
struct rk3026_codec_pdata rk3026_codec_pdata_info={
    .spk_ctl_gpio = RK2928_PIN3_PD4,
    .hp_ctl_gpio = RK2928_PIN3_PC2,
    .ear_ctl_gpio = RK2928_PIN3_PC4,
};

static struct resource resources_acodec[] = {
	{
		.start 	= RK2928_ACODEC_PHYS,
		.end 	= RK2928_ACODEC_PHYS + RK2928_ACODEC_SIZE - 1,
		.flags 	= IORESOURCE_MEM,
	},
};

static struct platform_device rk3026_codec = {
	.name	= "rk3026-codec",
	.id		= -1,
	.resource = resources_acodec,
    	.dev = {
        	.platform_data = &rk3026_codec_pdata_info,
    }
};
#endif
#if defined(CONFIG_GPS_RK)
#define GPS_OSCEN_PIN 	RK2928_PIN1_PB0
#define GPS_RXEN_PIN 	RK2928_PIN1_PB0

static int rk_gps_io_init(void)
{
	printk("%s \n", __FUNCTION__);
	
	gpio_request(GPS_OSCEN_PIN, NULL);
	gpio_direction_output(GPS_OSCEN_PIN, GPIO_LOW);	
	
	iomux_set(GPS_CLK);//GPS_CLK
	iomux_set(GPS_MAG);//GPS_MAG
	iomux_set(GPS_SIGN);//GPS_SIGN
#if 0
	gpio_request(RK30_PIN1_PA6, NULL);
	gpio_direction_output(RK30_PIN1_PA6, GPIO_LOW);

	gpio_request(RK30_PIN1_PA5, NULL);
	gpio_direction_output(RK30_PIN1_PA5, GPIO_LOW);	

	gpio_request(RK30_PIN1_PA7, NULL);
	gpio_direction_output(RK30_PIN1_PA7, GPIO_LOW);		
#endif	
	return 0;
}
static int rk_gps_power_up(void)
{
	printk("%s \n", __FUNCTION__);

	return 0;
}

static int rk_gps_power_down(void)
{
	printk("%s \n", __FUNCTION__);

	return 0;
}

static int rk_gps_reset_set(int level)
{
	return 0;
}
static int rk_enable_hclk_gps(void)
{
	struct clk *gps_aclk = NULL;
	gps_aclk = clk_get(NULL, "aclk_gps");
	if(gps_aclk) {
		clk_enable(gps_aclk);
		clk_put(gps_aclk);
		printk("%s \n", __FUNCTION__);
	}
	else
		printk("get gps aclk fail\n");
	return 0;
}
static int rk_disable_hclk_gps(void)
{
	struct clk *gps_aclk = NULL;
	gps_aclk = clk_get(NULL, "aclk_gps");
	if(gps_aclk) {
		//TO wait long enough until GPS ISR is finished.
		msleep(5);
		clk_disable(gps_aclk);
		clk_put(gps_aclk);
		printk("%s \n", __FUNCTION__);
	}	
	else
		printk("get gps aclk fail\n");
	return 0;
}
static struct rk_gps_data rk_gps_info = {
	.io_init = rk_gps_io_init,
	.power_up = rk_gps_power_up,
	.power_down = rk_gps_power_down,
	.reset = rk_gps_reset_set,
	.enable_hclk_gps = rk_enable_hclk_gps,
	.disable_hclk_gps = rk_disable_hclk_gps,
	.GpsSign = RK2928_PIN1_PA5,
	.GpsMag = RK2928_PIN1_PA4,        //GPIO index
	.GpsClk = RK2928_PIN1_PA2,        //GPIO index
	.GpsVCCEn = GPS_OSCEN_PIN,     //GPIO index
#if 0	
	.GpsSpi_CSO = RK30_PIN1_PA4,    //GPIO index
	.GpsSpiClk = RK30_PIN1_PA5,     //GPIO index
	.GpsSpiMOSI = RK30_PIN1_PA7,	  //GPIO index
#endif	
	.GpsIrq = IRQ_GPS,
	.GpsSpiEn = 0,
	.GpsAdcCh = 2,
	.u32GpsPhyAddr = RK2928_GPS_PHYS,
	.u32GpsPhySize = RK2928_GPS_SIZE,
};

static struct platform_device rk_device_gps = {
	.name = "gps_hv5820b",
	.id = -1,
	.dev		= {
	.platform_data = &rk_gps_info,
		}
	};
#endif


/***********************************************************
*	i2c
************************************************************/
#ifdef CONFIG_KP_AXP
#include "../../../drivers/power/axp_power/axp-board.h"
#endif

#ifdef CONFIG_REGULATOR_ACT8931
#define ACT8931_HOST_IRQ		RK30_PIN1_PB1//depend on your hardware


#define ACT8931_CHGSEL_PIN    INVALID_GPIO //RK30_PIN0_PD0 //depend on your hardware


static struct pmu_info  act8931_dcdc_info[] = {
	{
		.name          = "act_dcdc1",   //vdd_io
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
	{
		.name          = "act_dcdc2",    //ddr
		.min_uv          = 1500000,
		.max_uv         = 1500000,
	},
	{
		.name          = "vdd_cpu",   //vdd_arm
		.min_uv          = 1200000,
		.max_uv         = 1200000,
	},
};
static  struct pmu_info  act8931_ldo_info[] = {
	{
		.name          = "act_ldo1",   //vcc28_cif
		.min_uv          = 2800000,
		.max_uv         = 2800000,
	},
	{
		.name          = "act_ldo2",    //vcc18_cif
		.min_uv          = 1800000,
		.max_uv         = 1800000,
	},
	{
		.name          = "act_ldo3",    //vcca30
		.min_uv          = 3000000,
		.max_uv         = 3000000,
	},
	{
		.name          = "act_ldo4",    //vcc_wl
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
};
#include "../mach-rk2928/board-rk2928-sdk-act8931.c"
#endif


#ifdef CONFIG_I2C0_RK30
static struct i2c_board_info __initdata i2c0_info[] = {
#if defined (CONFIG_MFD_TPS65910)
	{
		.type           = "tps65910",
		.addr           = TPS65910_I2C_ID0,
		.flags          = 0,
		.irq            = TPS65910_HOST_IRQ,
		.platform_data = &tps65910_data,
	},
#endif
#if defined (CONFIG_RTC_HYM8563)
    {    
        .type           = "rtc_hym8563",
        .addr           = 0x51,
        .flags          = 0, 
        .irq            = RK30_PIN1_PA5,
    },   
#endif
#ifdef  CONFIG_KP_AXP19
	{
		.type = "axp_mfd",
		.addr = 0x34,
		.flags  = 0,		
		.irq =RK30_PIN1_PB1,
		.platform_data = &axp_pdata,
	},
#endif
#if defined (CONFIG_REGULATOR_ACT8931)
	{
		.type    		= "act8931",
		.addr           = 0x5b, 
		.flags			= 0,
		.irq            = ACT8931_HOST_IRQ,
		.platform_data=&act8931_data,
	},
#endif
};
#endif

#if defined CONFIG_TCC_BT_DEV
static struct tcc_bt_platform_data tcc_bt_platdata = {

    .power_gpio   = { // ldoon
        .io             = INVALID_GPIO,//difined depend on your harware
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = NULL,
            },
        },

    .wake_host_gpio  = { // BT_HOST_WAKE, for bt wakeup host when it is in deep sleep
        .io         = RK30_PIN0_PC5, // set io to INVALID_GPIO for disable it,it's depend on your hardware
        .enable     = IRQF_TRIGGER_RISING,// set IRQF_TRIGGER_FALLING for falling, set IRQF_TRIGGER_RISING for rising
        .iomux      = {
            .name       = NULL,
        },
    },
};

static struct platform_device device_tcc_bt = {
    .name   = "tcc_bt_dev",
    .id     = -1,
    .dev    = {
        .platform_data = &tcc_bt_platdata,
        },
};
#endif

#ifdef CONFIG_I2C1_RK30
static struct i2c_board_info __initdata i2c1_info[] = {
#if defined (CONFIG_GS_MXC6225)
        {
                .type           = "gs_mxc6225",
                .addr           = 0x15,
                .flags          = 0,
                .irq            = MXC6225_INT_PIN,
                .platform_data  = &mxc6225_info,
        },
#endif
#ifdef CONFIG_RDACOMBO
	{
		I2C_BOARD_INFO("RDAWIFI_CORE",0x13),
	},
	{
		I2C_BOARD_INFO("RDAWIFI_RF",  0x14),
	},
	{
		I2C_BOARD_INFO("RDABT_CORE",  0x15),
	},
	{
		I2C_BOARD_INFO("RDABT_RF",    0x16),
	},
	{
		I2C_BOARD_INFO("RDA_FM",      0x11),
	},
#endif
#if defined (CONFIG_GS_DMARD10)                                                                                                                                             
     {
         .type           = "gs_dmard10",
         .addr           = 0x18,
         .flags          = 0,
         .irq            = GS_INT_PIN,
     },
#endif
#ifdef  CONFIG_KP_AXP20
	{
		.type = "axp_mfd",
		.addr = 0x34,
		.flags  = 0,		
		.irq =RK30_PIN1_PB1,
		.platform_data = &axp_pdata,
	},
#endif
#if defined (CONFIG_GS_MMA7660)
	{
		.type	        = "gs_mma7660",//gs_mma7660
		.addr	        = 0x4c,
		.flags	        = 0,
		.irq	        = MMA7660_INT_PIN,
		.platform_data = &mma7660_info,
	},
#endif
#if defined (CONFIG_GS_MMA8452)
	{
		.type	        = "gs_mma8452",
		.addr	        = 0x1d,
		.flags	        = 0,
		.irq	        = MMA8452_INT_PIN,
		.platform_data = &mma8452_info,
	},
#endif
#if defined (CONFIG_GS_LIS3DH)
	{
		.type	        = "gs_lis3dh",
		.addr	        = 0x19,   //0x19(SA0-->VCC), 0x18(SA0-->GND)
		.flags	        = 0,
		.irq	        = LIS3DH_INT_PIN,
		.platform_data = &lis3dh_info,
	},
#endif
#if defined (CONFIG_GS_LSM303D)
        {
            .type           = "gs_lsm303d",
            .addr           = 0x1d,   //0x19(SA0-->VCC), 0x18(SA0-->GND)
            .flags          = 0,
            .irq            = LSM303D_INT_PIN,         
	    .platform_data  = &lms303d_info,           
        },
#endif
#if defined (CONFIG_COMPASS_AK8975)
	{
		.type          = "ak8975",
		.addr          = 0x0d,
		.flags         = 0,
		.irq           = RK30_PIN3_PD7,	
		.platform_data = &akm8975_info,
		.irq           = RK30_PIN3_PD7,	
		.platform_data = &akm8975_info,
	},
#endif
#if defined (CONFIG_COMPASS_AK8963)
	{
		.type          = "ak8963",
		.addr          = 0x0d,
		.flags         = 0,
		.irq           = RK30_PIN3_PD7,	
		.platform_data = &akm8963_info,
	},
#endif
#if defined (CONFIG_GYRO_L3G4200D)
	{
		.type          = "l3g4200d_gryo",
		.addr          = 0x69,
		.flags         = 0,
		.irq           = L3G4200D_INT_PIN,
		.platform_data = &l3g4200d_info,
	},
#endif

};
#endif

#ifdef CONFIG_I2C2_RK30
static struct i2c_board_info __initdata i2c2_info[] = {
#if defined (CONFIG_TOUCHSCREEN_GT8XX)
	{
		.type          = "Goodix-TS",
		.addr          = 0x55,
		.flags         = 0,
		.irq           = TOUCH_INT_PIN,
		.platform_data = &goodix_info,
	},
#endif
#if defined (CONFIG_TOUCHSCREEN_GSLX680_RK3028) || defined (CONFIG_TOUCHSCREEN_GSLX680_RK3028_780)
    {
        .type           = "gslX680",
        .addr           = 0x40,
        .flags          = 0,
        .platform_data =&gslx680_info,
    },
#endif

};
#endif

#ifdef CONFIG_I2C3_RK30
static struct i2c_board_info __initdata i2c3_info[] = {
};
#endif

#ifdef CONFIG_I2C_GPIO_RK30
#define I2C_SDA_PIN     INVALID_GPIO// RK30_PIN2_PD6   //set sda_pin here
#define I2C_SCL_PIN     INVALID_GPIO//RK30_PIN2_PD7   //set scl_pin here
static int rk30_i2c_io_init(void)
{
        return 0;
}

struct i2c_gpio_platform_data default_i2c_gpio_data = {
       .sda_pin = I2C_SDA_PIN,
       .scl_pin = I2C_SCL_PIN,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(100),
       .bus_num    = 5,
       .io_init = rk30_i2c_io_init,
};

static struct i2c_board_info __initdata i2c_gpio_info[] = {
};
#endif

static void __init rk30_i2c_register_board_info(void)
{
#ifdef CONFIG_I2C0_RK30
	i2c_register_board_info(0, i2c0_info, ARRAY_SIZE(i2c0_info));
#endif
#ifdef CONFIG_I2C1_RK30
	i2c_register_board_info(1, i2c1_info, ARRAY_SIZE(i2c1_info));
#endif
#ifdef CONFIG_I2C2_RK30
	i2c_register_board_info(2, i2c2_info, ARRAY_SIZE(i2c2_info));
#endif
#ifdef CONFIG_I2C3_RK30
	i2c_register_board_info(3, i2c3_info, ARRAY_SIZE(i2c3_info));
#endif
#ifdef CONFIG_I2C_GPIO_RK30
	i2c_register_board_info(4, i2c_gpio_info, ARRAY_SIZE(i2c_gpio_info));
#endif
}

/***********************************************************
*	board init
************************************************************/
static struct platform_device *devices[] __initdata = {
#ifdef CONFIG_ION
	&device_ion,
#endif
#ifdef CONFIG_ANDROID_TIMED_GPIO
	&rk29_device_vibrator,
#endif
#ifdef CONFIG_WIFI_CONTROL_FUNC
	&rk29sdk_wifi_device,
#endif
#ifdef CONFIG_RFKILL_RK
	&device_rfkill_rk,
#endif
#ifdef CONFIG_BATTERY_RK30_ADC_FAC
 	&rk30_device_adc_battery,
#endif
#ifdef CONFIG_SND_SOC_RK3028A_PHONE
	&rk3026_codec,
#endif
#ifdef CONFIG_GPS_RK
	&rk_device_gps,
#endif
#ifdef CONFIG_TCC_BT_DEV
        &device_tcc_bt,
#endif
#ifdef CONFIG_PWM_CONTROL_LOGIC
	&pwm_regulator_device[0],
#endif

#if defined(CONFIG_PWM_CONTROL_ARM) || defined(CONFIG_PWM_LOGIC_WITH_ARM)
	&pwm_regulator_device[1],
#endif
#if defined(CONFIG_BP_AUTO)
	&device_bp_auto,
#endif
#if defined (CONFIG_MODEM_SOUND)
 &modem_sound_device,
#endif
#if defined (CONFIG_RK_HEADSET_DET) ||  defined (CONFIG_RK_HEADSET_IRQ_HOOK_ADC_DET)
	&rk_device_headset,                                                                                                     
#endif
};

#if  defined(CONFIG_KP_AXP)
extern  void axp_power_off(void);
#endif

static void rk30_pm_power_off(void)
{
#if defined(CONFIG_MFD_TPS65910)
	if(pmic_is_tps65910())
		tps65910_device_shutdown();//tps65910 shutdown
#endif

#if  defined(CONFIG_KP_AXP)
	if(pmic_is_axp202())
		axp_power_off();

#endif

#if defined(CONFIG_REGULATOR_ACT8931)
	if(pmic_is_act8931()){
		 printk("enter dcdet pmic_is_act8931===========\n");
               if(gpio_get_value (RK30_PIN0_PB2) == GPIO_LOW)
               {
                       printk("enter restart===========\n");
                       arm_pm_restart(0, "charge");
               }
              act8931_device_shutdown();
	}
#endif

	gpio_direction_output(POWER_ON_PIN, GPIO_LOW);
	while(1);
}

static void __init machine_rk30_board_init(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
	pwm_regulator_init();
#endif
	avs_init();
	pm_power_off = rk30_pm_power_off;
	rk_gpio_request(POWER_ON_PIN, GPIOF_DIR_OUT, GPIO_HIGH, "system power on");
	rk30_i2c_register_board_info();
	spi_register_board_info(board_spi_devices, ARRAY_SIZE(board_spi_devices));
	platform_add_devices(devices, ARRAY_SIZE(devices));
	rk_platform_add_display_devices();	
#if defined(CONFIG_WIFI_CONTROL_FUNC)
	rk29sdk_wifi_bt_gpio_control_init();
#elif defined(CONFIG_WIFI_COMBO_MODULE_CONTROL_FUNC)
    rk29sdk_wifi_combo_module_gpio_init();
#endif
#ifdef CONFIG_RDA5990_SUPPORT                                                                                                                                               
    gpio_request(RK30_PIN3_PD3, "WIFI_POWER");
    gpio_direction_output(RK30_PIN3_PD3, 1);
    gpio_free(RK30_PIN3_PD3);
#endif
	
}

static void __init rk30_reserve(void)
{
	//fb reserve
#ifdef CONFIG_FB_ROCKCHIP
	resource_fb[0].start = board_mem_reserve_add("fb0 buf", get_fb_size());
	resource_fb[0].end = resource_fb[0].start + get_fb_size()- 1;
	#if 0
	resource_fb[1].start = board_mem_reserve_add("ipp buf", RK30_FB0_MEM_SIZE);
	resource_fb[1].end = resource_fb[1].start + RK30_FB0_MEM_SIZE - 1;
	#endif

	#if defined(CONFIG_FB_ROTATE) || !defined(CONFIG_THREE_FB_BUFFER)
	resource_fb[2].start = board_mem_reserve_add("fb2 buf",get_fb_size());
	resource_fb[2].end = resource_fb[2].start + get_fb_size() - 1;
	#endif
#endif
	//ion reserve
#ifdef CONFIG_ION
	rk30_ion_pdata.heaps[0].base = board_mem_reserve_add("ion", ION_RESERVE_SIZE);
#endif

#ifdef CONFIG_VIDEO_RK29
	rk30_camera_request_reserve_mem();
#endif
#ifdef CONFIG_GPS_RK
	//it must be more than 8MB
	rk_gps_info.u32MemoryPhyAddr = board_mem_reserve_add("gps", SZ_8M);
#endif
	board_mem_reserved();
}

/***********************************************************
*	clock
************************************************************/
static struct cpufreq_frequency_table dvfs_arm_table[] = {
	{.frequency = 312 * 1000,       .index = 1150 * 1000},
	{.frequency = 504 * 1000,       .index = 1150 * 1000},
	{.frequency = 816 * 1000,       .index = 1200 * 1000},
	{.frequency = 1008 * 1000,      .index = 1250 * 1000},
	{.frequency = CPUFREQ_TABLE_END},
};

static struct cpufreq_frequency_table dvfs_gpu_table[] = {
	//{.frequency = 100 * 1000,       .index = 1200 * 1000},
	{.frequency = 200 * 1000,       .index = 1200 * 1000},
	{.frequency = 266 * 1000,       .index = 1200 * 1000},
	//{.frequency = 300 * 1000,       .index = 1200 * 1000},
	{.frequency = 400 * 1000,       .index = 1200 * 1000},
	{.frequency = CPUFREQ_TABLE_END},
};

static struct cpufreq_frequency_table dvfs_ddr_table[] = {
	//{.frequency = 200 * 1000 + DDR_FREQ_SUSPEND,    .index = 1200 * 1000},
	//{.frequency = 300 * 1000 + DDR_FREQ_VIDEO,      .index = 1200 * 1000},
	{.frequency = 360 * 1000 + DDR_FREQ_NORMAL,     .index = 1200 * 1000},
	{.frequency = CPUFREQ_TABLE_END},
};

void __init board_clock_init(void)
{
	rk2928_clock_data_init(periph_pll_default, codec_pll_default, RK30_CLOCKS_DEFAULT_FLAGS);
	//dvfs_set_arm_logic_volt(dvfs_cpu_logic_table, cpu_dvfs_table, dep_cpu2core_table);	
	dvfs_set_freq_volt_table(clk_get(NULL, "cpu"), dvfs_arm_table);
	dvfs_set_freq_volt_table(clk_get(NULL, "gpu"), dvfs_gpu_table);
	dvfs_set_freq_volt_table(clk_get(NULL, "ddr"), dvfs_ddr_table);
}

/************************ end *****************************/
MACHINE_START(RK30, "RK30board")
	.boot_params	= PLAT_PHYS_OFFSET + 0x800,
	.fixup		= rk2928_fixup,
	.reserve	= &rk30_reserve,
	.map_io		= rk2928_map_io,
	.init_irq	= rk2928_init_irq,
	.timer		= &rk2928_timer,
	.init_machine	= machine_rk30_board_init,
MACHINE_END
