#!/usr/bin/env sh
# build.sh - A simple script to build and zip Android Kernels
# Copyright (C) 2025  Sidney PEPO <sidneypepo@disroot.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US

ARCH="arm64"	 # If your target is not an aarch64 (arm64) device, change this

MAKEOPTS=" ${2}" # If you want to avoid manually writing make options, add then
		 # here, right after the first quote

SRC="$(pwd)/$(dirname "${0}")"
TOOLCHAIN="${SRC}/aarch64-linux-android-4.9/bin/aarch64-linux-android-"
AK3="${SRC}/AnyKernel3"
OUT="${SRC}/out"
KERNEL="${OUT}/arch/arm64/boot/Image.gz-dtb"
KERNEL_ZIP="${OUT}/kernel_$(date +"%Y-%m-%d_%H-%M").zip"

if [ -t 1 ]; then
	COLOR_RESET="\e[0m"
	COLOR_RED="\e[1;31m"
	COLOR_GREEN="\e[1;32m"
	COLOR_YELLOW="\e[1;33m"
fi

SIGN_INFO="${COLOR_YELLOW}[*]${COLOR_RESET}"
SIGN_SUCCESS="${COLOR_GREEN}[+]${COLOR_RESET}"
SIGN_ERROR="${COLOR_RED}[-]${COLOR_RESET}"

func_make()
{
	[ "${1}" -eq 0 ] && return 0

	printf "%b Running make...\n" "${SIGN_INFO}"
	if ! make -C "${SRC}" O="${OUT}" ARCH="${ARCH}" CROSS_COMPILE="${TOOLCHAIN}" "${MAKEOPTS}"; then
		printf "%b Error during make (%s)!\n" "${SIGN_ERROR}" "${?}"
		return 1
	fi
	printf "%b Make finished with no errors!\n" "${SIGN_SUCCESS}"
        [ -f "${KERNEL}" ] && printf "%b The built Kernel is here: %s.\n" "${SIGN_INFO}" "${KERNEL}"
	return 0
}

func_zip()
{
	[ "${1}" -eq 0 ] && return 0

	printf "%b Zipping Kernel...\n" "${SIGN_INFO}"
	if [ ! -f "${KERNEL}" ]; then
		printf "%b Missing Kernel (%s)!\n" "${SIGN_ERROR}" "${KERNEL}"
		return 1
	fi

	rm -f "${AK3}/Image.gz-dtb" "${OUT}/"*.zip
	cp "${KERNEL}" "${AK3}/Image.gz-dtb"
	cd "${AK3}"
	zip -9r "${KERNEL_ZIP}" *
	cd -
	printf "%b Kernel zipped (%s)!\n" "${SIGN_SUCCESS}" "${KERNEL_ZIP}"
	return 0
}

func_help()
{
	cat<<-EOT
Usage: build.sh COMMAND ["MAKEOPTS"]

Commands:
 all ["MAKEOPTS"]	Equivalent to make and zip commands together
 make ["MAKEOPTS"]	Run make. You can pass a specific target or extra make
 		        options as a single quoted string (see examples below)
 zip			Zip Kernel
 help			Display this help

Examples:
 build.sh make "mrproper"
 build.sh all "-j\$(nproc) -Wextra"
EOT
}

main()
{
	if [ ! -d "$(dirname "${TOOLCHAIN}")" ]; then
		printf "%b Missing toolchain (%s)! Did you pulled it?\n" "${SIGN_ERROR}" "$(dirname "${TOOLCHAIN}")"
		return 1
	elif [ ! -d "${AK3}" ]; then
		printf "%b Missing AnyKernel3 (%s)! Did you pulled it?\n" "${SIGN_ERROR}" "${AK3}"
		return 1
	elif [ ! -d "${OUT}" ]; then
		mkdir -p "${OUT}"
	fi

	run_make=0
	run_zip=0
	case "${1}" in
		"all") run_make=1; run_zip=1;;
		"make") run_make=1;;
		"zip") run_zip=1;;
		"help"|*) func_help;;
	esac

	if ! func_make ${run_make}; then
	        return 1
	fi

	if ! func_zip ${run_zip}; then
		return 1
	fi

	return 0
}

main "${@}"
