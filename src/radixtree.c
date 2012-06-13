
/*
 * Copyright 2012 William Heinbockel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define RT_DEBUG

#define NODE_INIT_SIZE 6

#define ALPHABET_SIZE 38
#define NODE_LEAF_MAX  ALPHABET_SIZE

typedef struct _node rt_node;

struct _node {
	uint8_t klen;
	uint8_t lcnt;
	uint8_t lalloc;
	char *key;
	void *value;
	struct _node **leaf;
};

typedef rt_node rt_tree;

static void
rt_node_free(rt_node *n)
{
	uint8_t i;
	rt_node **l;
	if(!n) return;
	for(i=0,l=n->leaf;i<n->lcnt;i++,*l++)
		rt_node_free(*l);
	if(n->key) free(n->key);
	if(n->leaf) free(n->leaf);
	/** @todo add support to free values */
	if(n->value) /*free(n->value)*/;
	free(n);
}

static rt_node *
rt_node_new(uint8_t c, char *key, size_t keylen)
{
	rt_node *n = NULL;
	uint8_t s = c > 0 && c < NODE_LEAF_MAX ? c : NODE_INIT_SIZE;
	size_t sz = sizeof(*n);
	n = calloc(1,sz+1);
	if(!n) return NULL;
	n->lalloc = s;
	n->klen = keylen;
	n->leaf = calloc(s,sizeof(n));
	if(!n->leaf) goto fail;
	if(key && keylen>0) {
		n->key = malloc(keylen);
		if(!n->key) goto fail;
		memcpy(n->key,key,keylen);
	}
	return n;
fail:
	rt_node_free(n);
	return NULL;
}

void
rt_node_print(rt_node *n, int depth)
{
	int i;
	rt_node **l;
	for(i=0;i<depth;i++) printf("\t");
	if(n)
	{
		if(n->key) printf("\"%.*s\"",n->klen,n->key);
		else       printf("NULL");
		if(n->value) printf(" = \"%s\"\n",n->value);
		else         printf(" = NULL\n");
		for(i=0,l=n->leaf;i<n->lcnt;i++,*l++) rt_node_print(*l,depth+1);
	} else printf("NULL\n");
}

static size_t
_maxmatch(const char *s1, const char *s2, size_t len)
{
	register char *m1 = (char *)s1, *m2 = (char *)s2;
	char *me1 = m1+len, *me2 = m2+len;
	size_t ret;
	while(*m1 == *m2 && m1<me1 && m2<me2) {
		*m1++; *m2++;
	}
	ret = m1 - s1;
	return ret;
}

static int
_cmp_nodes(const void *v1, const void *v2)
{
	register const char *n1 = (const char *)v1;
	register const rt_node *n2 = *(const rt_node **)v2;
	return n1[0] - n2->key[0];
}

static rt_node *
rt_node_find(rt_node *n,char *key, size_t lkey, int add)
{
	rt_node *node = NULL;
	/* printf("LOG: rt_node_find(n,\"%.*s\",%d,%d)\n",lkey,key,lkey,add); */
	if(!n || lkey < 1 || !key) return NULL;
	/* if we are only adding, we can do a more efficient binary search */
	if(!add) {
		size_t mm,mlen;
		if(n->lcnt == 0 || !n->leaf) return NULL;
		node = *(rt_node **)bsearch(key,n->leaf,n->lcnt,sizeof(void*),_cmp_nodes);
		if(node==NULL) return NULL;
		mlen = node->klen < lkey ? node->klen : lkey;
		mm = _maxmatch(node->key,key,mlen);
		if(mm < node->klen) return NULL;
		else if(mm==mlen && node->klen==lkey) return node;
		else return rt_node_find(node,key+mm,lkey-mm,add);
	}

	/* otherwise, do a tail insertion sort */
	if(n->lcnt == 0)
	{
		node = rt_node_new(0,key,lkey);
		if(!node) return NULL;
		n->leaf[0] = node;
		n->lcnt++;
	} else {
		/* Search for the key
		 * If not found, return the location for it to be added */
		int i = n->lcnt-1, diff;
		rt_node **index;
		if(n->lcnt >= n->lalloc) {
			size_t ns = n->lalloc;
			rt_node **rt;
			ns *= 2;
			if(ns>ALPHABET_SIZE) ns = ALPHABET_SIZE;
			rt = realloc(n->leaf,ns*sizeof(rt));
			if(!rt) return NULL;
			n->lalloc = ns;
			n->leaf = rt;
		}
		index = &n->leaf[i];
		while(i >= 0) {
			/* Only need to compare first char */
			diff = key[0] - (*index)->key[0];
			if(diff > 0) {
				node = rt_node_new(0,key,lkey);
				if(!node) return NULL;
				n->leaf[i+1] = node;
				n->lcnt++;
				break;
			} else if(diff < 0) {
				n->leaf[i+1] = *index;
				if(i==0) {
					node = rt_node_new(0,key,lkey);
					if(!node) return NULL;
					n->leaf[i] = node;
					n->lcnt++;
					break;
				}
			} else {
				/*
				 * if matches, recurse to next node level
				 * otherwise,  update this node to have the shared string; then add children for different suffixes
				 */
				rt_node *p = *index, *child;
				size_t mm = _maxmatch(p->key, key, p->klen < lkey ? p->klen : lkey);
				if(mm < p->klen) {
					rt_node **tmp;
					/* otherwise, split
					 * add child and update this node */
					child = rt_node_new(0,p->key+mm,p->klen-mm);
					if(!child) {
						/* failed to split and add child node */
						return NULL;
					}
					tmp = calloc(NODE_INIT_SIZE,sizeof(rt_node *));
					if(!tmp) return NULL;
					child->lalloc=p->lalloc;
					child->lcnt = p->lcnt;
					child->leaf = p->leaf;
					child->value = p->value;
					p->klen = mm;
					p->key[mm] = 0;
					p->value = NULL;
					p->leaf = tmp;
					p->lalloc = NODE_INIT_SIZE;
					p->lcnt = 1;
					p->leaf[0] = child;
				}
				if(mm==lkey && p->klen==lkey) return p;
				else return rt_node_find(p,key+mm,lkey-mm,add);
			}
			i--;*index--;
		}
	}
	return node;
}

rt_tree *
rt_tree_new(void)
{
	rt_tree *t = NULL;
	t = (rt_tree *)rt_node_new(0,NULL,0);
	return t;
}

void
rt_tree_free(rt_tree *t)
{
	rt_node_free((rt_node *)t);
}

int
rt_tree_find(rt_tree *t, char *key, uint8_t lkey, void ** value)
{
	rt_node *n = rt_node_find((rt_node*)t,key,lkey,0);
	if(n) {
		*value = n->value;
		return 1;
	} else {
		*value = NULL;
		return 0;
	}
}

int
rt_tree_add(rt_tree *t, char *key, uint8_t lkey, void *value)
{
	rt_node *n = rt_node_find((rt_node*)t,key,lkey,1);
	if(n) {
		n->value = value;
		return 1;
	} else {
		return 0;
	}
}

void
rt_tree_print(rt_tree *t)
{
	int i;
	rt_node **l;
	if(!t) printf("NULL");
	for(i=0,l=t->leaf;i<t->lcnt;i++,*l++) rt_node_print(*l,0);
}

#ifdef RT_DEBUG
int
main(int argc, char **argv)
{
	rt_tree *t;
	char **arg;
	int i, succ=0;

	t = rt_tree_new();
	if(!t) {
		printf("ERROR: Could not create rt_tree... Exiting\n");
		return 1;
	}
	for(i=1,arg=++argv;i<argc;i++,arg++)
	{
		if(rt_tree_add(t, *arg, strlen(*arg), *arg))
			succ++;
		else printf("!!! Adding arg[%d] = %s... FAILED\n",i,*arg);
	}
	printf("ADD Passed: %d of %d\n",succ,argc-1);
	rt_tree_print(t);
	succ = 0;
	for(i=argc-1,arg--;i>0;i--,arg--)
	{
		char *val;
		if(rt_tree_find(t, *arg, strlen(*arg), (void **)&val)) {
			if(!strcmp(val,*arg)) succ++;
			else printf("!!! Value mismatch (%s != %s)\n",val,*arg);
		} else printf("!!! Searching for \"%s\"(arg[%d])... FAILED\n",*arg,i);
	}
	printf("SEARCH Passed: %d of %d\n",succ,argc-1);

	rt_tree_free(t);
	return 0;
}
#endif

