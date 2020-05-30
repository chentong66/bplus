#undef PAGE_SIZE
#define PAGE_SIZE 8192
#include <iostream>
#include <assert.h>
using namespace std;
struct btree_inode {
	unsigned long key;
	struct btree_node *p;
};
struct btree_node {
	unsigned long keynum;
	struct btree_node *next;
	struct btree_node *parent;
	struct btree_inode child[5];
}__attribute__((aligned(sizeof(unsigned long))));
//#define BTREE_ORDER ((((PAGE_SIZE) - sizeof(struct btree_node)) \
		/ sizeof(struct btree_inode)) - 1)
#define BTREE_ORDER 4
#define BTREE_ORDER_HALF (BTREE_ORDER % 2 ? (BTREE_ORDER + 1) / 2 \
		: BTREE_ORDER / 2)
#define BTREE_FULL(pointer) ((pointer)->keynum >= BTREE_ORDER)
#define BTREE_OVERFLOW(pointer) ((pointer)->keynum >= BTREE_ORDER + 1)
#define BTREE_LEAF(pointer) ((pointer)->child[0].p == NULL)
extern int btree_insert(struct btree_node **_head, unsigned long key);
extern struct btree_node *btree_find(struct btree_node *head,unsigned long key);
inline void btree_show(struct btree_node *head) {
	struct btree_node *node = head;
	unsigned long tmp;
	while (node->child[0].p)
		node = node->child[0].p;
	while (node) {
		for (int i = 0; i < node->keynum; i++) {
			if (node->child[i].key && node->child[i].key <= tmp){
				cout << "Ouch!" << endl;
				assert(false);
			}
			cout << node->child[i].key << " ";
			tmp = node->child[i].key;
		}
		node = node->next;
	}
	cout << endl;
}
