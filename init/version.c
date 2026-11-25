/*
 *  linux/init/version.c
 *
 *  Copyright (C) 1992  Theodore Ts'o
 *
 *  May be freely distributed as part of Linux.
 */

#include <generated/compile.h>
#include <linux/build-salt.h>
#include <linux/export.h>
#include <linux/uts.h>
#include <linux/utsname.h>
#include <generated/utsrelease.h>
#include <linux/version.h>
#include <linux/proc_ns.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/build_bug.h>

#ifndef CONFIG_KALLSYMS
#define version(a) Version_ ## a
#define version_string(a) version(a)

extern int version_string(LINUX_VERSION_CODE);
int version_string(LINUX_VERSION_CODE);
#endif

/*
 * Android's BPF loaders gate the set of eBPF programs they load on the
 * release string returned by uname(2), so allow an opt-in override for
 * kernels whose BPF support was backported to a level newer than the
 * kernel version they report. Only the runtime UTS release is affected;
 * the vermagic and /proc/version still use the real UTS_RELEASE.
 */
#ifdef CONFIG_KERNEL_RELEASE_OVERRIDE
static int __init init_uts_release_override(void)
{
	BUILD_BUG_ON(sizeof(CONFIG_KERNEL_RELEASE_OVERRIDE) >
		     sizeof(init_uts_ns.name.release));

	if (CONFIG_KERNEL_RELEASE_OVERRIDE[0])
		strscpy(init_uts_ns.name.release,
			CONFIG_KERNEL_RELEASE_OVERRIDE,
			sizeof(init_uts_ns.name.release));
	return 0;
}
early_initcall(init_uts_release_override);
#endif

struct uts_namespace init_uts_ns = {
	.kref = KREF_INIT(2),
	.name = {
		.sysname	= UTS_SYSNAME,
		.nodename	= UTS_NODENAME,
		.release	= UTS_RELEASE,
		.version	= UTS_VERSION,
		.machine	= UTS_MACHINE,
		.domainname	= UTS_DOMAINNAME,
	},
	.user_ns = &init_user_ns,
	.ns.inum = PROC_UTS_INIT_INO,
#ifdef CONFIG_UTS_NS
	.ns.ops = &utsns_operations,
#endif
};
EXPORT_SYMBOL_GPL(init_uts_ns);

/* FIXED STRINGS! Don't touch! */
const char linux_banner[] =
	"Linux version " UTS_RELEASE " (" LINUX_COMPILE_BY "@"
	LINUX_COMPILE_HOST ") (" LINUX_COMPILER ") " UTS_VERSION "\n";

const char *linux_banner_ptr = linux_banner;
EXPORT_SYMBOL_GPL(linux_banner_ptr);

const char linux_proc_banner[] =
	"%s version %s"
	" (" LINUX_COMPILE_BY "@" LINUX_COMPILE_HOST ")"
	" (" LINUX_COMPILER ") %s\n";

BUILD_SALT;
