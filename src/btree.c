#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "btree.h"

//assert for debug use
struct btree_node *btree_alloc() {
	struct btree_node *node = NULL;
//	node = (struct btree_node *)malloc(PAGE_SIZE);
	node = (struct btree_node *)mmap(NULL,PAGE_SIZE,PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE|MAP_LOCKED, -1, 0);
//	memset(node, 0, PAGE_SIZE);
	assert(node);
	assert(node->keynum == 0);
	return node;
}
void btree_free(struct btree_node *p) {
	munmap(p,PAGE_SIZE);
//	free(p);
}
static int __btree_find_half(struct btree_node *node,unsigned long key,unsigned long *pos){
	unsigned long i;
	assert(pos && node);
	for (i = 0; i < node->keynum;i++){
		if (node->child[i].key >= key){
			if (node->child[i].key != key)
				i = (i == 0 ? i : i - 1);
			break;
		}
	}
	*pos = i;
	if (i == node->keynum)
		return 1;
	return 0;
}
/*
static int __btree_find_exact(struct btree_node *head , unsigned long *pos, unsigned long key_to_find) {
	unsigned long i = 0;
	__btree_find_half(head,key_to_find,&i);
	assert(i < head->keynum && head->child[i].key == key_to_find);
	*pos = i;
	return 1;
	
	for (i = 0; head && i < head->keynum; i++) {
		if (head->child[i].key == key_to_find) {
			*pos = i;
			return 1;
		}
	}
	return 0;
	
}
*/
static void __btree_find_locate(struct btree_node *head, unsigned long key_to_find, unsigned long *pos) {
	unsigned long i = 0;
	if (!head) {
//		printf("fuzzy:head null\n");
		return;
	}
	*pos = 0;
	__btree_find_half(head,key_to_find,&i);
//	printf("fuzzy for,key_to_find %ld ",key_to_find);
/*
	for (i = 0; i < head->keynum; i++) {
//		printf(" %ld ",head->child[i].key);
		if (head->child[i].key >= key_to_find) {
			break;
		}
	}
*/
	assert(head->child[i].key == key_to_find);
	*pos = i;
/*
	if (head->child[i].key == key_to_find) {
//		printf("fuzzyp1\n");
		*pos = i;
	}
	else {
		//This couldn't happen..
//		printf("fuzzyp2\n");
		assert(head->child[i].key == key_to_find);
		*pos = i ? i - 1 : i;
	}
*/
}
static struct btree_node *__btree_find(
		struct btree_node *head,
		struct btree_node **pparent,struct btree_node **pnode,
		unsigned long *ppos,unsigned long key){
	struct btree_node *node,*parent = NULL;
	unsigned long i = 0;
	assert(head);
	node = head;
	if (head->keynum == 0)
		goto out;
	do{
		assert(node);
		//if (node->keynum == 0)
		//	i = 0;
		//else {
		//	unsigned long start = 0;
		//	unsigned long end = node->keynum - 1;
		//	unsigned long half;
		//	do {
		//		half = (start + end) >> 1;
		//		i = half;
		//		if (half && node->child[half].key >= key && node->child[half - 1].key <= key) {
		//			i = half - 1;
		//			break;
		//		}
		//		if (half == start) {
		//			if (half < node->keynum - 1 && node->child[half + 1].key <= key)
		//				i = half + 1;
		//			break;
		//		}
		//		else {
		//			if (node->child[half].key >= key) {
		//				end = half;
		//			}
		//			else {
		//				start = half;
		//			}
		//		}
		//	} while (start != end);
		//}
		/*
		for (i = 0; i < node->keynum;i++){
			if (node->child[i].key >= key){
				if (node->child[i].key != key)
					i = (i == 0 ? i : i - 1);
				break;
			}
		}
		*/
		
		unsigned int hret;
		hret = __btree_find_half(node,key,&i);
		if (BTREE_LEAF(node))
			break;
		parent = node;
		if (hret)
			node = node->child[--i].p;
		else 
			node = node->child[i].p;
	} while(true);
	assert(node);
	if (pparent)
		*pparent = parent;
	if (pnode)
		*pnode = node;
	if (ppos)
		*ppos = i;
	if (node->child[i].key == key)
		return node;
	/*
	for (i = 0; node && i < node->keynum;i++){
		if (key == node->child[i].key)
			return node;
	}
	*/
	return NULL;
out:
	if (pparent)
		*pparent = NULL;
	if (pnode)
		*pnode = NULL;
	if (ppos)
		*ppos = 0;
	return NULL;
}
static void __btree_move(struct btree_node *node,unsigned long from,unsigned long num,int left){
	assert(node != NULL);
	unsigned long times = node->keynum - from;
	unsigned long count = 0;
	unsigned long start;

	if (!left) {
		start = node->keynum - 1;
		while (count < times) {
			node->child[start + num] = node->child[start];
			start--;
			count++;
		}
	}
	else {
		start = from;
		while (count < times) {
			node->child[start - num] = node->child[start];
			start++;
			count++;
		}
	}
}
static void __btree_spread_mod(struct btree_node *node,unsigned long key_to_find) {
	struct btree_node *pnode;
	unsigned long pos;
	assert(node && node->parent);
	do {
		__btree_find_locate(node->parent, key_to_find,&pos);
		node->parent->child[pos].key = node->child[0].key;
		node = node->parent;
	} while (node->parent && pos == 0);
}
static void __btree_insert_key(struct btree_node *node,struct btree_node *p,unsigned long key,unsigned long *pos){
	unsigned long i,j;
	if (node == NULL) {
		node = NULL;
	}
	if (!pos){
		for (i = 0; i < node->keynum; i++){
			assert(key != node->child[i].key);
			if (node->child[i].key > key)
			       break;
		}
	}
	else
		i = *pos;
	j = node->keynum;
	if (node->keynum > 0)
		__btree_move(node,i,1,0);
	node->child[i].key = key;
	node->child[i].p = p;
	if (p){
		assert(p->child[0].key == key);
		p->parent = node;
		p->next = NULL;
		if (i < node->keynum)
			node->child[i].p->next = node->child[i+1].p;
		else if(i > 0)
			node->child[i].p->next = node->child[i-1].p->next;
		if (i > 0) {
			node->child[i-1].p->next = p;
		}
	}
	node->keynum++;
	if (i == 0 && node->parent) {
		__btree_spread_mod(node,node->child[1].key);
	}
}

static void __btree_insert_key_leaf(struct btree_node *node, unsigned long key) {
	return __btree_insert_key(node, NULL, key, NULL);
}
static struct btree_node *__btree_find_avaliable_silbing(
		struct btree_node *parent,
		unsigned long *_pos){
	struct btree_node *ret = NULL;
	unsigned long pos = *_pos;
	assert(parent != NULL);
	if (pos == 0){
		struct btree_node *tmp;
		tmp = parent->child[1].p;
		if (!BTREE_FULL(tmp)) {
			ret = tmp;
			pos = 1;
		}
	}
	else {
		struct btree_node *left = NULL,*right = NULL;
		left = parent->child[pos-1].p;
		if (pos < parent->keynum - 1)
			right = parent->child[pos+1].p;
		if (left->keynum > BTREE_ORDER || right && right->keynum > BTREE_ORDER) {
			assert(false);
		}
		if (!BTREE_FULL(left)) {
			ret = left;
			pos--;
		}
		else if (right && !BTREE_FULL(right)) {
			ret = right;
			pos++;
		}

	}
	*_pos = pos;
	return ret;
}
static void __btree_copy(struct btree_node *dst, struct btree_node *src, unsigned long dst_start, unsigned long src_start, unsigned long num) {
	assert(dst != NULL && src != NULL);
	unsigned long count = 0;
	while (count < num) {
		dst->child[dst_start] = src->child[src_start];
		if (dst->child[dst_start].p)
			dst->child[dst_start].p->parent = dst;
		dst_start++;
		src_start++;
		count++;
	}
}
static void __btree_move_element(
		struct btree_node *dst,
		struct btree_node *src,
		unsigned long num){
	unsigned long src_start;
	unsigned long dst_start;
	assert(dst != NULL && src != NULL && !BTREE_FULL(dst));
	assert(num <= BTREE_ORDER && num + dst->keynum <= BTREE_ORDER);
	if (!(src->keynum > 0 && src->keynum >= num))
		src = src;
	if (dst->keynum == 0) {
		src_start = src->keynum - num;
		dst_start = 0;
		__btree_copy(dst, src, dst_start, src_start, num);
	}
	else if (src->child[0].key > dst->child[0].key) {
		src_start = 0;
		dst_start = dst->keynum;
		__btree_copy(dst, src, dst_start, src_start, num);
		__btree_move(src, num, num, 1);
	}
	else {
		src_start = src->keynum - num;
		dst_start = 0;
		__btree_move(dst, 0, num, 0);
		__btree_copy(dst, src, dst_start, src_start, num);
	}
	src->keynum -= num;
	dst->keynum += num;
	
}
static int __btree_sibling_balance(
		struct btree_node *parent,
		struct btree_node *node,
		unsigned long pos){
	struct btree_node *silb;
	assert(node != NULL);
	if (!(!parent || parent->child[pos].key <= node->child[0].key)){
		if (parent){
			printf("%ld ,%ld , %ld\n",pos,parent->child[pos].key,node->child[0].key);
			assert(false);
		}
		
	}
	if (pos == parent->keynum)
		return 0;
	silb = __btree_find_avaliable_silbing(parent,&pos);
	if (!silb)
		return 0;
//	printf("balance:silb child[0] %ld\n",silb->child[0].key);
//	btree_show(node);
//	btree_show(silb);
	__btree_move_element(silb,node,1);
	if (silb->child[0].key < node->child[0].key) {
//		printf("balcance p1\n");
		__btree_spread_mod(node, silb->child[silb->keynum - 1].key);
	}
	else {
//		printf("balcance p2\n");
		__btree_spread_mod(silb, silb->child[1].key);
	}
//	btree_show(node);
//	btree_show(silb);
	return 1;
}
static struct btree_node *__btree_rebalance(
		struct btree_node *parent,
		struct btree_node *node){
	assert(node != NULL && BTREE_OVERFLOW(node));
	struct btree_node *tmp;
	tmp = btree_alloc();
	if (!tmp)
		goto out;
	if (parent == NULL){
		struct btree_node *nhead;
		nhead = btree_alloc();
		assert(nhead);
		if (!nhead)
			goto out_free_tmp;
		__btree_move_element(tmp,node,node->keynum / 2);
		__btree_insert_key(nhead, node, node->child[0].key, NULL);
		__btree_insert_key(nhead,tmp,tmp->child[0].key,NULL);
		parent = nhead;
	}
	else {
		__btree_move_element(tmp, node, node->keynum / 2);
		__btree_insert_key(parent,tmp,tmp->child[0].key,NULL);
	}
	return parent;
out_free_tmp:
	btree_free(tmp);
out:
	return NULL;
}
static void __btree_erase(struct btree_node *head, unsigned long key) {
	assert(head != NULL);
}

struct btree_node *btree_find(struct btree_node *head,unsigned long key){
	return __btree_find(head,NULL,NULL,NULL,key);
}
int btree_insert(struct btree_node **_head,unsigned long key){
	struct btree_node *head,*nhead = NULL;
	struct btree_node *parent;
	struct btree_node *node;
	unsigned long ppos;
	if (!_head)
		return -1;
	head = *_head;
	if (head == NULL)
		head = btree_alloc();
	if(__btree_find(head,&parent,&node,&ppos,key))
		return 1;
	nhead = head;
	if (BTREE_LEAF(head)){
		__btree_insert_key_leaf(head,key);
		if (BTREE_OVERFLOW(head))
			nhead = __btree_rebalance(NULL,head);
	}
	else {
		__btree_insert_key_leaf(node,key);
		assert(node->parent == parent);
		while(BTREE_OVERFLOW(node)){
			__btree_find_locate(node->parent, node->child[0].key,&ppos);
//			printf("loop %d,key %ld,num %ld ,ppos %ld\n",++i,key,parent ? parent->keynum : 0,ppos);
//			printf("pos %ld ,pkey %ld ,node key %ld\n",ppos,parent ? parent->child[ppos].key : 0,node->child[0].key);
			if (parent && __btree_sibling_balance(parent,node,ppos)){
//				__btree_find_fuzzy(node->parent, &ppos, node->child[0].key);
//				printf("parent %p\n",parent);
//				printf("pos %ld ,pkey %ld ,node key %ld\n",ppos,parent ? parent->child[ppos].key : 0,node->child[0].key);
				assert((!parent || parent->child[ppos].key <= node->child[0].key));
				break;
			}
			parent = __btree_rebalance(parent,node);
			assert(parent == node->parent);
			if (!parent){
				assert(false);
				break;
			}
			node = parent;
			parent = node->parent;
		}
		if (!node->parent || !parent)
			nhead = node;
	}

	if (!nhead) {
		assert(false);
		__btree_erase(head,key);
		return -1;
	}
	*_head = nhead;
	return 0;

}
