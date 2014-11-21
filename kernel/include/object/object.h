#ifndef __OBJECT_H
#define __OBJECT_H

#include <security/sd.h>

/* Object operations structure */
struct object;
typedef struct object_ops
{
	/* Opening a handle to the object */

	/* Closing a handle to the object */

	/* Duplicating a handle to the object */

	/* Deleting the object */
} object_ops_t;

/* Object header structure */
typedef struct object
{
	/* Object operations */
	object_ops_t *ops;

	/* Reference and hardlink count */
	atomic_t refcount;
	atomic_t link_count;

	/* Security descriptor */
	security_descriptor_t security_descriptor;
} object_t;

/* Reference and dereference an object */
void object_ref(object_t *object);
void object_unref(object_t *object);

/* Get and set the security descriptor of an object */
security_descriptor_t *object_get_security(object_t *object);
void object_set_security(object_t *object, security_descriptor_t *descriptor);

#endif
