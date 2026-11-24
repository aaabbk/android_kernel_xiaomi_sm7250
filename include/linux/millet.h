#ifndef MILLET_H
#define MILLET_H

#include <linux/freezer.h>
#include <net/sock.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/cgroup.h>
#include <uapi/linux/millet.h>

#define RET_OK 0
#define RET_ERR -1
#define WARN_AHEAD_MSGS		3
#define RESERVE_ORDER		17
#define WARN_AHEAD_SPACE	(1 << RESERVE_ORDER)
#define CHECK_BUFFER_DETAIL_SAPCE	(1 << 16)
#define LARGE_BUFFER_THRESHOLD		(1 << 15)
#define BUFFER_CHECK_INTERVAL_MS	5000

struct millet_sock;
typedef void (*recv_hook)(void *data, unsigned int len);
typedef int (*send_hook)(struct task_struct *tsk,
		struct millet_data *data, struct millet_sock *sk);
typedef void (*init_hook)(struct millet_sock *sk);

struct millet_sock {
	atomic_t has_init;
	struct sock *sock;

	struct mod_info {
		int id;
		enum MILLET_TYPE monitor;
		atomic_t port;
		recv_hook recv_from;
		send_hook send_to;
		init_hook init;
		spinlock_t lock;
		struct {
			u64 send_suc;
			u64 send_fail;
			u64 runtime;
		} stat;
		char name[NAME_MAXLEN];
		void *priv;
	} mod[MILLET_TYPES_NUM];
};

#ifdef CONFIG_MILLET
int millet_sendmsg(enum MILLET_TYPE type, struct task_struct *t,
		struct millet_data *data);
int millet_sendto_user(struct task_struct *tsk,
		struct millet_data *data, struct millet_sock *sk);
int register_millet_hook(int type, recv_hook recv_from, send_hook send_to,
		init_hook init);
int unregister_millet_hook(int type);
bool judge_millet_freeze_switch(void);
extern int frozen_uid_min;
extern unsigned long binder_warn_ahead_space;
extern bool millet_binder_switch;
#else
static int frozen_uid_min = 10000;
static unsigned long binder_warn_ahead_space = WARN_AHEAD_SPACE;
static const bool millet_binder_switch = true;
static inline int millet_sendmsg(enum MILLET_TYPE type, struct task_struct *t,
		struct millet_data *data) { return 0; }
static inline int millet_sendto_user(struct task_struct *tsk,
		struct millet_data *data, struct millet_sock *sk) { return 0; }
static inline int register_millet_hook(int type, recv_hook recv_from,
		send_hook send_to, init_hook init) { return 0; }
static inline int unregister_millet_hook(int type) { return 0; }
static inline bool judge_millet_freeze_switch(void) { return false; }
#endif
/*
 * 4.19 keeps the cgroup v2 frozen state in task->frozen (cgroup_task_frozen())
 * and the pending freeze in CGRP_FREEZE (cgroup_task_freeze()), which is what
 * the cgroup v2 freezer used by Android uses.
 */
static inline bool frozen_task(struct task_struct *task)
{
	return cgroup_task_frozen(task);
}

static inline int frozen_task_group(struct task_struct *task)
{
	return (freezing(task) || frozen_task(task) || cgroup_task_freeze(task));
}
#endif
