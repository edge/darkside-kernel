#ifndef __PROCESS_H
#define __PROCESS_H

#include <lib/libc/types.h>
#include <lib/libadt/list.h>

/* Process structure */
typedef struct process
{
	/* Process ID and name */
	uint32_t pid;
	uint8_t *name;

	/* User and group ID */
	int32_t uid, gid;

	/* Threads */
	list_t threads;

	/* Address space */
	uint32_t address_space;

	/* Files opened by the process */
	list_t files;

	/* Child processes */
} process_t;

/* Create and destroy a process */
process_t *process_create(uint8_t *name, void (*fn)(void *arg), void *arg);
void process_destroy(process_t *process);

/* Kill, suspend, and resume a process */
void process_kill(process_t *process, int32_t status);
void process_suspend(process_t *process);
void process_resume(process_t *process);

#endif
