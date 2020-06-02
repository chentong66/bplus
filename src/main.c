#include <time.h>
#include <iostream>
#include "btree.h"
using namespace std;
int main(){
	struct btree_node *head = NULL;
	clock_t t;
	unsigned long i = 0;
	cout << BTREE_ORDER << " " << PAGE_SIZE << endl;
	cout << sizeof(*head) << endl;
	t = clock();
	while ( i < 100000000) {
		btree_insert(&head,i);
		btree_insert(&head,100000000 - i);
		i++;
	}
	t = clock() - t;
	printf("insert time = %lf s\n",((float)(t) / CLOCKS_PER_SEC));
//	btree_show(head);
	t = clock();
	while(i > 50000000){
		struct btree_node *tmp = btree_find(head,i);
		tmp = btree_find(head,100000000 - i);
		btree_find(head,i);
		tmp = btree_find(head,100000000 - i);
		btree_find(head,i << 1);
		tmp = btree_find(head,i >> 1);
//		tmp = btree_find(head,i * i);
		/*if (tmp)
			cout << 1 << " ";
		else 
			cout << 0 << " ";
		*/
//		btree_find(head,i >> 3);
//		btree_find(head,i >> 2);
		i--;
	}
	t = clock() - t;
	printf("serach time = %lf s\n",((float)(t) / CLOCKS_PER_SEC));
	btree_show(head);
	printf("head->keynum %ld\n",head->keynum);
	return 0;
}
