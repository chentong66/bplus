#include <time.h>
#include <iostream>
#include "btree.h"
using namespace std;
int main(){
	struct btree_node *head = NULL;
	clock_t t;
	unsigned long i = 0;
	cout << BTREE_ORDER << " " << PAGE_SIZE << endl;
	t = clock();
	while ( i < 1000) {
		if (i == 16)
			i = 16;
		cout << i << endl;
		btree_insert(&head,i);
		btree_insert(&head,100000 - i);
//		btree_insert(&head,100000 - i);
		i++;
	}
	t = clock() - t;
	printf("time = %lf s\n",((float)(t) / CLOCKS_PER_SEC));
	printf("head->keynum %ld\n",head->keynum);
	btree_show(head);
	return 0;
}
