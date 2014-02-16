/* Copyright (c) 2013 The F9 Microkernel Project. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/link.h>
#include <user_runtime.h>
#include <l4/ipc.h>
#include <l4/utcb.h>

#include <l4io.h>
#define STACK_SIZE 256

static L4_ThreadId_t thread __USER_DATA;

static L4_Word_t last_thread __USER_DATA;
static L4_Word_t free_mem __USER_DATA;

void __USER_TEXT hello_thread(void)
{
	printf("Hello World\n");
	while(1);
}

static void __USER_TEXT start_thread(L4_ThreadId_t t, L4_Word_t ip,
		L4_Word_t sp, L4_Word_t stack_size)
{
	L4_Msg_t msg;
	L4_MsgClear(&msg);
	L4_MsgAppendWord(&msg, ip);
	L4_MsgAppendWord(&msg, sp);
	L4_MsgAppendWord(&msg, stack_size);
	L4_MsgLoad(&msg);

	L4_Send(t);
}

static L4_ThreadId_t __USER_TEXT create_thread(user_struct *user, void (*func)(void))
{
	L4_ThreadId_t myself = L4_MyGlobalId();
	L4_ThreadId_t child;

	child.raw = myself.raw + (++last_thread << 14);

	L4_ThreadControl(child, myself, L4_nilthread, myself, (void *) free_mem);
	free_mem += UTCB_SIZE + STACK_SIZE;

	start_thread(child, (L4_Word_t)func, free_mem, STACK_SIZE);

	return child;
}

static void __USER_TEXT main(user_struct *user)
{
	free_mem = user->fpages[0].base;

	thread = create_thread(user, hello_thread);
}

DECLARE_USER(
	255,
	hello,
	main,
	DECLARE_FPAGE(0x0, 2 * UTCB_SIZE + 2 *STACK_SIZE)
);
