#include <time.h>
#include <stdio.h>
#include "btree.h"
int main(){
	struct btree_node *head = NULL;
	clock_t t;
	unsigned long i = 0;
	printf("0-99999999\n");
	printf("insert..\n");
	t = clock();
	while ( i < 25000000) {
		btree_insert(&head,i);
		btree_insert(&head,100000000 - i);
		btree_insert(&head,50000000 - i);
		btree_insert(&head,50000000 + i);
		i++;
	}
	t = clock() - t;
	btree_show(head);
	printf("insert time = %lf s\n",((float)(t) / CLOCKS_PER_SEC));
	printf("lookup..\n");
	t = clock();
	i = 100000000;
	while(i > 50000000){
		struct btree_node *tmp = btree_find(head,i);
		btree_find(head,100000000 - i);
		i--;
	}
	i = 100000000;
	t = clock() - t;
	btree_show(head);
	printf("lookup time = %lf s\n",((float)(t) / CLOCKS_PER_SEC));
	printf("remove..\n");
	t = clock();
	while(i > 50000000){
		btree_remove(head,i);
		btree_remove(head,100000000 - i);
		i--;
	}
	t = clock() - t;
	btree_show(head);
	printf("remove time = %lf s\n",((float)(t) / CLOCKS_PER_SEC));
//	btree_show(head);
	return 0;
}
