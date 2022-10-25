

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <pwd.h>
#include <ctype.h>

// #define _DEFAULT_SOURCE
#define _BSD_SOURCE

#ifndef _proclib_header
#define _proclib_header

#define PROCFS_ROOT "/proc"

/* In-place time of string, s, to remove whitespace */
void trim(char *s);
void trim(char *s)
{
	char *p = s;
	int l = strlen(p);

	while (isspace(p[l - 1]))
		p[--l] = 0;
	while (*p && isspace(*p))
		++p, --l;

	memmove(s, p, l + 1);
}

#endif
/* acts as node in both linked list and tree */
struct pstree_node
{
	char name[128];
	pid_t pid;
	pid_t ppid;
	int visited;
	struct pstree_node *parent;
	struct pstree_node *children[128];
	struct pstree_node *next;
};
static struct pstree_node *ll_head;

struct pstree_node *find_node_with_pid(pid_t pid)
{
	struct pstree_node *node;
	for (node = ll_head; node != NULL; node = node->next)
	{
		if (node->pid == pid)
		{
			return node;
		}
	}
	return NULL;
}

int catalog_process(char *dirname)
{
	char filename[256];
	char linebuf[256];
	char procname[256];
	char pid[32];
	char ppid[32];
	char *key;
	char *value;
	FILE *p_file;
	strcpy(filename, dirname);
	strcat(filename, "/status");
	p_file = fopen(filename, "r");
	if (p_file == NULL)
	{
		return 1; /* just ignore, this is fine I guess */
	}
	while (fgets(linebuf, sizeof(linebuf), p_file) != NULL)
	{
		key = strtok(linebuf, ":");
		value = strtok(NULL, ":");
		if (key != NULL && value != NULL)
		{
			trim(key);
			trim(value);
			if (strcmp(key, "Pid") == 0)
			{
				strcpy(pid, value);
			}
			else if (strcmp(key, "PPid") == 0)
			{
				strcpy(ppid, value);
			}
			else if (strcmp(key, "Name") == 0)
			{
				strcpy(procname, value);
			}
		}
	}
	struct pstree_node *node;
	node = (struct pstree_node *)malloc(sizeof(struct pstree_node));
	if (node == NULL)
	{
		printf("Unable to allocate memory for node\n");
		return 1;
	}
	strcpy(node->name, procname);
	node->pid = atoi(pid);
	node->ppid = atoi(ppid);
	// node->children[0] = NULL;
	// node->parent = NULL;
	node->next = ll_head; /* could be null, that is fine */
	ll_head = node;
	return 0;
}

int get_tgid(char *dirname)
{
	char filename[256];
	char linebuf[256];
	char procname[256];
	char tgid[32];
	char pid[32];
	char *key;
	char *value;
	FILE *p_file;
	strcpy(filename, dirname);
	strcat(filename, "/status");
	p_file = fopen(filename, "r");
	if (p_file == NULL)
	{
		return 1; /* just ignore, this is fine I guess */
	}
	while (fgets(linebuf, sizeof(linebuf), p_file) != NULL)
	{
		key = strtok(linebuf, ":");
		value = strtok(NULL, ":");
		if (key != NULL && value != NULL)
		{
			trim(key);
			trim(value);
			if (strcmp(key, "Tgid") == 0)
			{
				strcpy(tgid, value);
			}
			else if (strcmp(key, "Pid") == 0)
			{
				strcpy(pid, value);
			}
			else if (strcmp(key, "Name") == 0)
			{
				char *new_string;
				strcpy(new_string, "{");
				strcat(new_string, value);
				strcat(new_string, "}");
				strcpy(procname, new_string);
			}
		}
	}
	if (pid != tgid)
	{
		struct pstree_node *node;
		node = (struct pstree_node *)malloc(sizeof(struct pstree_node));
		if (node == NULL)
		{
			printf("Unable to allocate memory for node\n");
			return 1;
		}
		strcpy(node->name, procname);
		node->pid = atoi(pid);
		node->ppid = atoi(tgid);
		// node->children[0] = NULL;
		// node->parent = NULL;
		node->next = ll_head; /* could be null, that is fine */
		ll_head = node;
	}

	return 0;
}

int make_tree(void)
{
	int i;
	struct pstree_node *node, *pnode;

	for (node = ll_head; node != NULL; node = node->next)
	{
		node->visited = 0;
		i = 0;
		pnode = find_node_with_pid(node->ppid);
		if (pnode != NULL)
		{
			node->parent = pnode;
			while (pnode->children[i++] != NULL)
				;
			pnode->children[i - 1] = node;
			pnode->children[i] = NULL;
		}
	}

	return 0;
}

void sort_Childrens_In_Aplhabetical_Order(struct pstree_node *root_node)
{	
	

	for (int i = 0; root_node->children[i] != NULL; i++)
	{
		for (int j = i + 1; root_node->children[j] != NULL; j++)
		{
			if (strcmp(root_node->children[i]->name, root_node->children[j]->name) > 0)
			{
				struct pstree_node *temp;
				temp = root_node->children[i];
				root_node->children[i] = root_node->children[j];
				root_node->children[j] = temp;
			}
		}
	}
	int k;
	while ((root_node = root_node->children[k++]) != NULL)
	{
		sort_Childrens_In_Aplhabetical_Order(root_node);
	}
}

int print_tree(struct pstree_node *root_node)
{
	struct pstree_node *node;
	// printf("├──%s");
	printf("%s", root_node->name);
	/* recurse on children */
	int j = 0;
	if (root_node->children[0] == NULL)
	{
		printf("\n");
	}
	else if (root_node->children[1] == NULL)
	{
		// printf("%s", root_node->name);
		printf("---");
		print_tree(root_node->children[0]);
	}
	else
	{
		while ((node = root_node->children[j++]) != NULL)
		{

			if (j == 1)
			{
				printf("-+-");
				print_tree(node);
			}
			else
			{
				struct pstree_node *parent;
				int name_length = strlen(root_node->name);
				char align[256] = "";
				for (int i = 0; i < name_length; i++)
				{
					strcat(align, " ");
					// printf("%s", align);
				}
				struct pstree_node *current = root_node;
				while (current->pid != 1 && current->parent != NULL)
				{
					parent = current->parent;
					char *tmp = strdup(align);
					if (parent->children[1] == NULL){
						strcpy(align, " ");
					}
					else{
						strcpy(align, "| ");
					}
					strcat(align, tmp); // concatenate previous align
					// strcat(align, "|");
					free(tmp);
					if (parent->pid == 1)
					{
						for (int i = 0; i < strlen(parent->name) + 1; i++)
						{
							char *tmp = strdup(align);
							strcpy(align, " ");
							strcat(align, tmp);
							free(tmp); // concatenate previous align
						}
					}
					else
					{
						for (int i = 0; i < strlen(parent->name) + 2; i++)
						{
							char *tmp = strdup(align);
							strcpy(align, " ");
							strcat(align, tmp);
							free(tmp); // concatenate previous align
						}
					}
					current = current->parent;
				}
				if (root_node->children[j] == NULL){
					strcat(align, " `-");
				}
				else{
					strcat(align, " |-");
				}
				printf("%s", align);
				print_tree(node);
				strcpy(align, "");
			}
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	DIR *dirp;
	struct dirent *directory_entry;
	char dirname[256];

	/* now, walk directories in /proc fs to create nodes */
	if ((dirp = opendir(PROCFS_ROOT)) == NULL)
	{
		perror("Unabled to open /proc");
		return 1;
	}
	do
	{
		if ((directory_entry = readdir(dirp)) != NULL)
		{
			if (directory_entry->d_type == DT_DIR)
			{
				strcpy(&dirname[0], PROCFS_ROOT);
				strcat(&dirname[0], "/");
				strcat(&dirname[0], directory_entry->d_name);
				catalog_process(dirname); // /proc/1

				char dirname1[256];
				strcpy(&dirname1[0], dirname);
				strcat(&dirname1[0], "/task");
				DIR *task_folder;
				if ((task_folder = opendir(dirname1)) != NULL)
				{
					struct dirent *process_under_task;
					int n = 0;
					while ((process_under_task = readdir(task_folder)) != NULL)
					{
						char dirname2[256];
						strcpy(&dirname2[0], dirname1);
						if (n > 2)
						{
							strcat(&dirname2[0], "/");
							strcat(&dirname2[0], process_under_task->d_name);
							get_tgid(dirname2);
						}
						n++;
					}
				}
			}
		}
	} while (directory_entry != NULL);

	/* turn the list into a tree */
	make_tree();

	struct pstree_node *rootnode = find_node_with_pid(1);
	// check_redundency(rootnode);
	sort_Childrens_In_Aplhabetical_Order(rootnode);


	struct string
	{
		char name[256];
	};
	struct dictionary{
		struct string key[32];
		int value[32];
	};
	
	// struct pstree_node *node;
	// for (node = ll_head; node != NULL; node = node->next){
	// 	if (node->children[0] == NULL){
	// 		struct pstree_node *parent = node->parent;

	// 		if (parent->visited == 0){
	// 			struct dictionary *dict;

	// 			int key_num = 0;
	// 			int i = 0;
	// 			while(parent->children[i] != NULL){
					
	// 				int contained = 0;
	// 				int j = 0;
	// 				while(j<32){
	// 					if (strcmp(dict->key[j].name, parent->children[i]->name) == 0){
	// 						dict->value[j]++;
	// 						contained = 1;
	// 						j++;
	// 					} 
	// 				}

	// 				if (contained == 0){
	// 					strcpy(dict->key[key_num].name, parent->children[i]->name);
	// 					dict->value[key_num] = 1;
	// 					key_num++;
	// 				}
	// 				parent->children[i] = NULL;
	// 				i++;
	// 			} 
	// 			parent->visited = 1;


	// 			// struct dictionary *newdict;
	// 			// for (int i = 0; i <32; i++){

	// 			// 	int j = 0;
	// 			// 	if (dict->value[i] > 1){
	// 			// 		char newname[256];
	// 			// 		// strcpy(newname, dict->value[i]);////////////////
	// 			// 		strcat(newname, "*[");
	// 			// 		strcat(newname, dict->key[i].name);/////////////////////////////////
	// 			// 		strcat(newname, "]");
	// 			// 		strcpy(newdict->key[j].name, newname);
	// 			// 		j++;
	// 			// 	}
	// 			// }

			
	// 			// for (int i = 0; newdict->key[i].name != NULL; i++){
	// 			// 	struct pstree_node *new1;
	// 			// 	strcpy(new1->name, newdict->key[i].name);
	// 			// 	new1->parent = parent;
	// 			// 	parent->children[i] = new1;
	// 			// }
	// 		}
	// 	}
	// }
	/* print the tree */
	struct pstree_node *ronode;
	ronode = find_node_with_pid(1);
	print_tree(ronode);
	return 0;
}

