/*
 * \brief  Post kernel activity
 * \author Sebastian Sumpf
 * \author Stefan Kalkowski
 * \date   2023-06-29
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/sched/task.h>
#include <usb_hid.h>

static int lx_user_main_task(void *arg)
{
	for (;;) {

		lx_emul_usb_client_device_update();

		lx_emul_led_state_update();

		/* block until lx_emul_task_unblock */
		lx_emul_task_schedule(true);
	}
	return 0;
}


static struct task_struct *main_task = NULL;


void lx_user_init(void)
{
	int pid = kernel_thread(lx_user_main_task, &main_task, CLONE_FS | CLONE_FILES);
	main_task = find_task_by_pid_ns(pid, NULL);
}


void lx_user_handle_io(void)
{
	lx_emul_usb_client_ticker();
	if (main_task) lx_emul_task_unblock(main_task);
}
