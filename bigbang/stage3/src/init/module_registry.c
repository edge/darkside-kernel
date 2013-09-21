#include <init/module_registry.h>
#include <fs/ext2.h>
#include <lib/libc/string.h>
#include <init/os_info.h>
#include <init/kprintf.h>
#include <vga/vga.h>
#include <lib/libc/string.h>
#include <lib/libc/str2int.h>
#include <lib/libc/types.h>
#include <lib/libadt/map.h>
#include <lib/libadt/index_tree.h>

extern partition_t *part;
extern superblock_t *superblock;
extern inode_t *root_inode;
extern inode_t *boot_inode;

/* Hash a key */
unsigned int hash(unsigned char *key)
{
	unsigned int hash_key = 0;

	while (*key)
	{
		hash_key *= 0x1F;
		hash_key += *key;

		key++;
	}

	return hash_key;
}

unsigned int tree_index(unsigned char *line)
{
	if(*line == '$')
	{
		if(strequal(line, "$OTHER"))
		{
			return 0xFFFFFFFF;
		}
	}
	else if(*line == '#')
	{
		return str2dec(line + 1);
	}
	else if(*line == '"')
	{
		unsigned char *noquotes = kmalloc(strlen(line) - 1);
		memset(noquotes, 0, strlen(line) - 1);
		
		strncpy(noquotes, line + 1, strlen(line) - 1);
		return hash(noquotes);
	}
	else
	{
		return 0;
	}
}

unsigned int separate_indents(unsigned char **line)
{
	unsigned int indents = 0;
	
	while(**line == '\t')
	{
		indents++;
		(*line)++;
	}
	
	/* The "root" node in the tree has 0 indents */
	return indents + 1;
}

void parse_registry(os_info_t *os_info)
{
	unsigned int modules = ext2_finddir(part, superblock, boot_inode, "modules");
	inode_t *modules_inode = read_inode(part, superblock, modules);

	unsigned int registry = ext2_finddir(part, superblock, modules_inode, "registry");
	inode_t *registry_inode = read_inode(part, superblock, registry);
	
	unsigned char *registry_data = kmalloc(registry_inode->low_size + 1);
	memset(registry_data, 0, registry_inode->low_size + 1);
	kprintf(LOG_DEBUG, "Size: %d\n", registry_inode->low_size);
	ext2_read(part, superblock, registry_inode, registry_data, registry_inode->low_size);
	
	kprintf(LOG_INFO, "Allocated and read module registry\n");
	
	unsigned char *saveptr = 0;
	unsigned char *line = strtok(registry_data, "\n", &saveptr);
	unsigned int lineNumber = 0;
	
	module_t *module = 0;
	int lastIndents = 0;
	
	index_tree_t tree = index_tree_create();
	struct index_tree_node *parent = tree.root;
	
	while(line != 0)
	{
		/* Separate out tabs from data, returning the number of tabs and advancing the string pointer */
		unsigned int indents = separate_indents(&line);
		
		/* We're in "module mode" */
		if(module)
		{
			/* Continuing a module declaration */
			if(indents == lastIndents)
			{
				kprintf(LOG_DEBUG, "Continuing module declaration\n");
				if(strnequal("@NAME", line, 5))
				{
					module->name = line + 6;
				}
				else if(strnequal("@DESC", line, 5))
				{
					module->desc = line + 6;
				}
				else if(strnequal("@AUTHOR", line, 7))
				{
					module->author = line + 8;
				}
				else if(strnequal("@PATH", line, 5))
				{
					module->path = line + 6;
				}
				else if(strequal("@REQBOOT", line))
				{
					/* EVENTUALLY load the module */
				}
				else if(strnequal("@VERSION", line, 8))
				{
					/* EVENTUALLY use substr + indexof to find the version numbers */
				}
				else
				{
					kprintf(LOG_PANIC, "Error: module registry command unrecognized at line %d: %s\n", line);
					while(1);
				}
			}
			/* Ending a module declaration */
			else
			{
				/* Write the module to the tree */
				index_tree_node_set_data(parent, module);
				module = 0;
				
				kprintf(LOG_DEBUG, "Writing module data to tree\n", indents);
			}
		}
		
		/* Sub out */
		while (lastIndents > indents)
		{
			parent = index_tree_node_parent(parent);
			lastIndents--;
			kprintf(LOG_DEBUG, "Subbing out to level %d\n", indents);
		}
			
		if(!module)
		{
			/* We are beginning a module declaration */
			if(strequal(line, "@MODULE"))
			{
				kprintf(LOG_DEBUG, "Beginning module declaration.\n");
				module = kmalloc(sizeof(module_t));
			}
			/* We are "subbing in" -- going another level into the tree */
			else if(!strequal(line, ""))
			{
				kprintf(LOG_DEBUG, "Subbing in to level %d\n", indents);
				struct index_tree_node *child = index_tree_node_create(parent, 0);
				index_tree_node_insert(parent, child, tree_index(line));
				parent = child;
			}
		}
		
		lastIndents = indents;
		line = strtok(0, "\n", &saveptr);
		lineNumber++;
	}
	
	os_info->module_registry = tree;
	
	kprintf(LOG_INFO, "Parsed module registry\n");
}