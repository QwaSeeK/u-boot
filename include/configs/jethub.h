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
	"bootcmd_check_button=" \
		"if gpio input periphs-banks10; then " \
		"run bootcmd_rescue;" \
		"else " \
		"run bootcmd_mmc0; " \
		"fi;\0" \
	\
	"bootcmd_rescue=" \
	"echo \"Rescue: checking USB...\"; " \
	"usb start; " \
	"mmc dev 1; " \
	"setenv loadaddr 0x06000000; " \
	"setenv rescue_writing 0; " \
	"setenv rescue_wrote 0; " \
	"if usb dev 0; then " \
		"if part list usb 0; then " \
			"if test -e usb 0:1 rescued1.img.gz; then " \
				"echo \"Rescue: flashing rescued image to eMMC...\"; " \
				"gpio set periphs-banks27; " \
				"if load usb 0:1 ${loadaddr} rescued1.img.gz; then " \
					"gpio clear periphs-banks26; " \
					"setexpr imgsize ${filesize}; " \
					"setenv rescue_writing 1; " \
					"if gzwrite mmc 1 ${loadaddr} ${imgsize}; then " \
						"echo \"Rescue: flash OK.\"; " \
						"gpio set periphs-banks26; " \
						"i=0; while test $i -lt 3; do " \
							"gpio clear periphs-banks27; sleep 0.3; " \
							"gpio set periphs-banks27; sleep 0.3; " \
							"setexpr i $i + 1; " \
						"done; " \
						"setenv rescue_wrote 1; " \
					"else " \
						"echo \"Rescue: gzwrite failed!\"; " \
					"fi; " \
				"else " \
					"echo \"Rescue: load failed!\"; " \
				"fi; " \
			"else " \
				"echo \"Rescue: image not found on USB.\"; " \
			"fi; " \
		"else " \
			"echo \"Rescue: no partitions on usb0.\"; " \
		"fi; " \
	"else " \
		"echo \"Rescue: no USB storage present.\"; " \
	"fi; " \
	"if test \"${rescue_writing}\" = \"1\"; then " \
		"echo \"Rescue: rebooting...\"; reset; " \
	"else " \
		"echo \"Rescue: boot installed system...\"; run bootcmd_mmc0; " \
	"fi;\0"
#elif defined(CONFIG_MESON_G12A)
#define BOOTENV_DEV_RESCUE(devtypeu, devtypel, instance) \
	"bootcmd_rescue=" \
		"echo \"Rescue button pressed, checking USB...\"; " \
		"usb reset; " \
		"mmc dev 1; " \
		"setenv loadaddr 0x48000000; " \
		"if test -e usb 0:1 rescued2.img.gz; then " \
			"echo \"Found rescue image, starting eMMC flash...\"; " \
			"run rescue_flash; " \
		"else " \
			"echo \"Rescue image not found, skipping eMMC flash.\"; " \
		"fi; " \
		"if test -e usb 0:1 spi-nor.img; then " \
			"echo \"Found spi-nor.img, checking for SPI update...\"; " \
			"run update_spi; " \
		"else " \
			"echo \"spi-nor.img not found, skipping SPI update.\"; " \
		"fi; " \
		"echo \"Done. Booting rescue system...\"; " \
		"run boot_script; " \
    "\0" \
	\
	"rescue_flash=" \
    "echo \"Flashing rescue image to eMMC...\"; " \
	"gpio set aobus-banks11; " \
    "if load usb 0:1 ${loadaddr} rescued2.img.gz; then " \
        "setexpr imgsize ${filesize}; " \
        "gzwrite mmc 1 ${loadaddr} ${imgsize}; " \
        "echo \"Flashing complete.\"; " \
		"gpio clear aobus-banks11; " \
		"i=0; " \
        "while test $i -lt 3; do " \
            "gpio set periphs-banks21; " \
            "sleep 0.3; " \
            "gpio clear periphs-banks21; " \
            "sleep 0.3; " \
            "setexpr i $i + 1; " \
        "done; " \
		"echo \"Resetting...\"; " \
		"reset; " \
    "else " \
        "echo \"Failed to load rescue image! Resetting...\"; " \
        "reset; " \
    "fi\0" \
	\
	"update_spi=" \
	"echo \"Checking for spi-nor.img on USB...\"; " \
	"sf probe; " \
	"if test -e usb 0:1 spi-nor.img; then " \
		"echo \"Found spi-nor.img, reading new version...\"; " \
		"load usb 0:1 ${loadaddr} spi-nor.img 0x40 0x00810000; " \
		"env import -t ${loadaddr} 0x40; " \
		"setenv rescue_version_new ${rescue_version}; " \
		"echo \"New version: ${rescue_version_new}\"; " \
		"sf read ${loadaddr} 0x00810000 0x40; " \
		"env import -t ${loadaddr} 0x40; " \
		"echo \"Current SPI version: ${rescue_version}\"; " \
		"if itest ${rescue_version_new} -gt ${rescue_version}; then " \
			"echo \"Newer version found, flashing SPI...\"; " \
			"gpio set aobus-banks11; " \
			"load usb 0:1 ${loadaddr} spi-nor.img; " \
			"setexpr imgsize ${filesize}; " \
			"sf update ${loadaddr} 0 ${imgsize}; " \
			"gpio clear aobus-banks11; " \
			"echo \"SPI update complete. Rebooting...\"; " \
			"i=0; " \
			"while test $i -lt 3; do " \
				"gpio set periphs-banks21; " \
				"sleep 0.3; " \
				"gpio clear periphs-banks21; " \
				"sleep 0.3; " \
				"setexpr i $i + 1; " \
			"done; " \
			"run boot_script; " \
		"else " \
			"echo \"SPI is already newest version.\"; " \
			"run boot_script; " \
		"fi; " \
		"else " \
		"echo \"spi-nor.img not found on USB.\"; " \
		"run boot_script; " \
	"fi\0" \
	\
	"boot_script=" \
		"sf probe;" \
		"setenv bootargs_root \"root=/dev/mtdblock5 ro rootwait rootfs=erofs\"; " \
		"setenv bootargs_console \"console=tty0 console=ttyAML0,115200n8 fbcon=map:1\"; " \
		"setenv bootargs_mtdparts \"mtdparts=spi0.0:2M(uboot),64k(env),6M(kernel),64k(version),-(rootfs) mtdids=nor0=spi0.0\"; " \
		"setenv bootargs \"${bootargs_console} ${bootargs_root} ${bootargs_mtdparts}\"; " \
		"echo \"Loading LZMA-compressed kernel from spi\"; " \
		"if sf read 0x10000000 0x00210000 0x00600000; then " \
			"echo \"Image.lzma loaded successfully.\"; " \
			"if lzmadec 0x10000000 ${kernel_addr_r}; then " \
				"echo \"LZMA decompression successful.\"; " \
			"else " \
				"echo \"Error: LZMA decompression failed!\"; " \
				"reset; " \
			"fi; " \
		"else " \
			"echo \"Error: Failed to load Image.lzma from SPI!\"; " \
			"reset; " \
		"fi; " \
		"printenv bootargs;" \
		"echo \"Starting kernel\"; " \
		"fdt addr ${fdtcontroladdr}; " \
		"booti ${kernel_addr_r} - ${fdtcontroladdr}; " \
		"echo \"Boot failed, resetting...\"; " \
	"reset\0" \
	\
	"bootcmd_check_button=" \
		"if gpio input periphs-banks70; then " \
			"run bootcmd_rescue; " \
		"else " \
			"run boot_script; " \
		"fi\0"
#elif defined(CONFIG_MESON_GXL)
#define BOOTENV_DEV_RESCUE(devtypeu, devtypel, instance) \
	"bootcmd_rescue=" \
		"echo \"Rescue: checking USB...\"; " \
		"usb start; " \
		"mmc dev 1; " \
		"setenv loadaddr 0x10200000; " \
		"setenv rescue_writing 0; " \
		"setenv rescue_wrote 0; " \
		"if usb dev 0; then " \
			"if part list usb 0; then " \
				"if test -e usb 0:1 rescueh1.img.gz; then " \
					"echo \"Rescue: flashing rescueh1.img.gz to eMMC...\"; " \
					"gpio set periphs-banks73; " \
					"if load usb 0:1 ${loadaddr} rescueh1.img.gz; then " \
						"setexpr imgsize ${filesize}; " \
						"setenv rescue_writing 1; " \
						"if gzwrite mmc 1 ${loadaddr} ${imgsize}; then " \
							"echo \"Rescue: flash OK.\"; " \
							"gpio clear periphs-banks73; " \
							"i=0; while test $i -lt 3; do " \
								"gpio set periphs-banks73; sleep 0.3; " \
								"gpio clear periphs-banks73; sleep 0.3; " \
								"setexpr i $i + 1; " \
							"done; " \
							"setenv rescue_wrote 1; " \
						"else " \
							"echo \"Rescue: gzwrite failed!\"; " \
						"fi; " \
					"else " \
						"echo \"Rescue: load failed!\"; " \
					"fi; " \
				"else " \
					"echo \"Rescue: image not found on USB.\"; " \
				"fi; " \
			"else " \
				"echo \"Rescue: no partitions on usb0.\"; " \
			"fi; " \
		"else " \
			"echo \"Rescue: no USB storage present.\"; " \
		"fi; " \
		"if test \"${rescue_writing}\" = \"1\"; then " \
			"echo \"Rescue: rebooting...\"; reset; " \
		"else " \
			"echo \"Rescue: boot installed system...\"; run bootcmd_mmc0; " \
		"fi;\0" \
	\
	"bootcmd_check_button=" \
	"if test \"${userbutton}\" = \"true\"; then " \
			"run bootcmd_rescue; " \
		"else " \
			"run bootcmd_mmc0; " \
		"fi\0"
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
