// SPDX-License-Identifier: GPL-2.0+
/*
 *	Copyright (C) 2002 Motorola GSG-China
 *
 * Author:
 *	Darius Augulis, Teltonika Inc.
 *
 * Desc.:
 *	Implementation of I2C Adapter/Algorithm Driver
 *	for I2C Bus integrated in Freescale i.MX/MXC processors
 *
 *	Derived from Motorola GSG China I2C example driver
 *
 *	Copyright (C) 2005 Torsten Koschorrek <koschorrek at synertronixx.de
 *	Copyright (C) 2005 Matthias Blaschke <blaschke at synertronixx.de
 *	Copyright (C) 2007 RightHand Technologies, Inc.
 *	Copyright (C) 2008 Darius Augulis <darius.augulis at teltonika.lt>
 *
 *	Copyright 2013 Freescale Semiconductor, Inc.
 *
 */

#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_data/i2c-imx.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <lx_emul/i2c.h>


/* This will be the driver name the kernel reports */
#define DRIVER_NAME "imx-i2c"

struct imx_i2c_struct {
	struct i2c_adapter	adapter;
};

static const struct platform_device_id imx_i2c_devtype[] = {
	{
		.name = "imx1-i2c",
		.driver_data = (kernel_ulong_t)0UL,
	}, {
		.name = "imx21-i2c",
		.driver_data = (kernel_ulong_t)0UL,
	}, {
		/* sentinel */
	}
};
MODULE_DEVICE_TABLE(platform, imx_i2c_devtype);

static const struct of_device_id i2c_imx_dt_ids[] = {
	{ .compatible = "fsl,imx1-i2c", .data = 0UL, },
	{ .compatible = "fsl,imx21-i2c", .data = 0UL, },
	{ .compatible = "fsl,vf610-i2c", .data = 0UL, },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, i2c_imx_dt_ids);

static int i2c_imx_xfer(struct i2c_adapter *adapter,
			struct i2c_msg *msgs, int num)
{
	switch (num) {
	case 0: return 0;
	case 1:
		{
			if (msgs[0].len != 2 || msgs[0].flags & I2C_M_RD) { break; }
			lx_emul_i2c_write_byte(msgs[0].buf[0], msgs[0].buf[1]);
			return num;
		}
	case 2:
		{
			if (msgs[0].len != 1 || msgs[1].len != 1 ||
			    !(msgs[1].flags & I2C_M_RD)) { break; }
			msgs[1].buf[0] = lx_emul_i2c_read_byte(msgs[0].buf[0]);
			return num;
		}
	default:
		break;
	}

	printk("I2C error: assuming wr/rd or wr/wr of single bytes only\n");
	printk("I2C error: num=%d msgs[0] len=%d read?=%d\n", num, msgs[0].len, msgs[0].flags & I2C_M_RD);
	if (num > 1) printk("I2C error: msgs[1] len=%d read?=%d\n", msgs[1].len, msgs[1].flags & I2C_M_RD);
	
	return 0;
}

static u32 i2c_imx_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL
		| I2C_FUNC_SMBUS_READ_BLOCK_DATA;
}

static const struct i2c_algorithm i2c_imx_algo = {
	.master_xfer = i2c_imx_xfer,
	.master_xfer_atomic = i2c_imx_xfer,
	.functionality = i2c_imx_func,
};

static int i2c_imx_probe(struct platform_device *pdev)
{
	struct imx_i2c_struct *i2c_imx;
	int ret;

	i2c_imx = devm_kzalloc(&pdev->dev, sizeof(*i2c_imx), GFP_KERNEL);
	if (!i2c_imx)
		return -ENOMEM;

	/* Setup i2c_imx driver structure */
	strlcpy(i2c_imx->adapter.name, pdev->name, sizeof(i2c_imx->adapter.name));
	i2c_imx->adapter.owner		= THIS_MODULE;
	i2c_imx->adapter.algo		= &i2c_imx_algo;
	i2c_imx->adapter.dev.parent	= &pdev->dev;
	i2c_imx->adapter.nr		= pdev->id;
	i2c_imx->adapter.dev.of_node	= pdev->dev.of_node;

	/* Set up adapter data */
	i2c_set_adapdata(&i2c_imx->adapter, i2c_imx);

	/* Set up platform driver data */
	platform_set_drvdata(pdev, i2c_imx);

	/* Add I2C adapter */
	ret = i2c_add_numbered_adapter(&i2c_imx->adapter);
	if (ret < 0)
		return ret;

	return 0;   /* Return OK */
}

static int i2c_imx_remove(struct platform_device *pdev)
{
	struct imx_i2c_struct *i2c_imx = platform_get_drvdata(pdev);

	i2c_del_adapter(&i2c_imx->adapter);
	return 0;
}

static int __maybe_unused i2c_imx_runtime_suspend(struct device *dev)
{
	return 0;
}

static int __maybe_unused i2c_imx_runtime_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops i2c_imx_pm_ops = {
	SET_RUNTIME_PM_OPS(i2c_imx_runtime_suspend,
			   i2c_imx_runtime_resume, NULL)
};

static struct platform_driver i2c_imx_driver = {
	.probe = i2c_imx_probe,
	.remove = i2c_imx_remove,
	.driver = {
		.name = DRIVER_NAME,
		.pm = &i2c_imx_pm_ops,
		.of_match_table = i2c_imx_dt_ids,
	},
	.id_table = imx_i2c_devtype,
};

static int __init i2c_adap_imx_init(void)
{
	return platform_driver_register(&i2c_imx_driver);
}
subsys_initcall(i2c_adap_imx_init);

static void __exit i2c_adap_imx_exit(void)
{
	platform_driver_unregister(&i2c_imx_driver);
}
module_exit(i2c_adap_imx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Darius Augulis");
MODULE_DESCRIPTION("I2C adapter driver for IMX I2C bus");
MODULE_ALIAS("platform:" DRIVER_NAME);
