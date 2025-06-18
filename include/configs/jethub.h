/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for JetHome devices
 * Copyright (C) 2021 Vyacheslav Bocharov
 * Copyright (C) 2024 JetHome
 * Author: Vyacheslav Bocharov <adeep@lexina.in>
 */

#ifndef __JETHUB_CONFIG_H
#define __JETHUB_CONFIG_H

#if defined(CONFIG_MESON_AXG)
#define BOOTENV_DEV_RESCUE(devtypeu, devtypel, instance) \
	"bootcmd_rescue=" \
		"if gpio input periphs-banks10; then " \
		"run bootcmd_mmc0; " \
		"run bootcmd_usb0;" \
		"fi;\0"
#elif defined(CONFIG_MESON_G12A)
#define BOOTENV_DEV_RESCUE(devtypeu, devtypel, instance) \
	"bootcmd_rescue=" \
    "echo \"Rescue button pressed, starting recovery...\"; " \
    "usb reset; " \
    "mmc dev 1; " \
    "setenv loadaddr 0x48000000; " \
    "setenv mmcblkstart 0; " \
    "setenv found 0; " \
    "if load usb 0:1 ${loadaddr} rescue.img 0x20000000; then " \
        "echo \"Detected rescue.img, using chunked flashing...\"; " \
        "run rescue_chunked; " \
        "setenv found 1; " \
    "else " \
        "if load usb 0:1 ${loadaddr} rescueall.img; then " \
            "echo \"Detected rescueall.img, using block-wise flashing...\"; " \
            "run rescue_single; " \
            "setenv found 1; " \
        "else " \
            "if load usb 0:1 ${loadaddr} rescue.img.part.00; then " \
                "echo \"Detected split parts, using multi-part flashing...\"; " \
                "run rescue_multipart; " \
                "setenv found 1; " \
            "fi; " \
        "fi; " \
    "fi; " \
    "if test ${found} -eq 0; then " \
        "echo \"No rescue image found. Aborting...\"; " \
    "fi; " \
    "reset; \0"
#else
#define BOOTENV_DEV_RESCUE(devtypeu, devtypel, instance) \
	"bootcmd_rescue=" \
		"if test \"${userbutton}\" = \"true\"; then " \
		"run bootcmd_mmc0; " \
		"fi;\0"
#endif

#define BOOTENV_DEV_NAME_RESCUE(devtypeu, devtypel, instance) \
	"rescue "

#ifndef BOOT_TARGET_DEVICES
#define BOOT_TARGET_DEVICES(func) \
	func(RESCUE, rescue, na) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	BOOT_TARGET_DEVICES_USB(func) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)
#endif

#include <configs/meson64.h>

#endif /* __JETHUB_CONFIG_H */
