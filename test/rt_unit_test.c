
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
#include "radixtree.h"

#ifdef NDEBUG
#define TEST(x) {tests++;if((x)==PASS){succ++;}}
#else
#define TEST(x) {tests++;if((x)==PASS){succ++;printf("++ PASS: \"%s\" test\n",#x);}else{printf("-- FAIL: \"%s\" test\n",#x);}}
#endif

#define ASSERT(x) {if(!(x)){printf("\t-- line %d: %s\n",__LINE__,#x);ret=FAIL;}}

typedef enum {
	PASS=0,
	FAIL=1,
	ERR=2
} status;

static status test1()
{
	rt_tree *t;
	t = rt_tree_new(0,NULL);
	if(t==NULL) return PASS;
	else {
		rt_tree_free(t);
		return FAIL;
	}
}

static status test2()
{
	rt_tree *t;
	t = rt_tree_new(1,NULL);
	if(t) {
		rt_tree_free(t);
		return PASS;
	} return FAIL;
}

/* rt_tree_set tests */
static status test3()
{
	rt_tree *t;
	char *val = "ABCDEFGH";
	status ret = PASS;
	t = rt_tree_new(16,NULL);
	if(!t) return ERR;

	ASSERT(!rt_tree_set(NULL,"ABC",3,val))
	ASSERT(!rt_tree_set(t,NULL,0,val))
	ASSERT(!rt_tree_set(t,NULL,1,val))
	ASSERT(!rt_tree_set(t,"ABC",3,NULL))

	ASSERT(rt_tree_set(t,"ABC",3,val))
	ASSERT(rt_tree_set(t,"ABC",3,val))
	ASSERT(rt_tree_set(t,"ABC",2,val))

	if(!t) return ERR;
	rt_tree_free(t);
	return ret;
}

/* rt_tree_get tests */
static status test4()
{
	rt_tree *t;
	char *val = "ABCDEFGH",*r;
	status ret = PASS;
	t = rt_tree_new(16,NULL);
	if(!t) return ERR;

	ASSERT(!rt_tree_get(NULL,"ABC",3))
	ASSERT(!rt_tree_get(t,NULL,3))
	ASSERT(!rt_tree_get(t,"ABC",3))
	if(rt_tree_set(t,"abc",3,val)) {
		r = rt_tree_get(t,"abc",3);
		ASSERT(r==val)
		ASSERT(!rt_tree_get(t,"abc",2))
		ASSERT(!rt_tree_get(t,"ABC",3))
	} else ret = FAIL;

	if(!t) return ERR;
	rt_tree_free(t);
	return ret;
}

/* test rt_tree_remove */
static status test5()
{
	rt_tree *t;
	char *val = "ABCDEFGH",*r;
	status ret = PASS;
	t = rt_tree_new(16,NULL);
	if(!t) return ERR;

	ASSERT(!rt_tree_remove(NULL,"ABC",3))
	ASSERT(!rt_tree_remove(t,NULL,2))
	ASSERT(!rt_tree_remove(t,"ABC",3))

	if(rt_tree_set(t,"ABC",3,val)) {
		r = rt_tree_get(t,"ABC",3);
		ASSERT(r==val)
		/* try to remove the key */
		ASSERT(rt_tree_remove(t,"ABC",3))
		/* try to retrive it, should return NULL */
		ASSERT(!rt_tree_get(t,"ABC",3))
		/* try to remove it again, should fail */
		ASSERT(!rt_tree_remove(t,"ABC",3))
	} else ret=FAIL;

	if(!t) return ERR;
	rt_tree_free(t);
	return ret;
}

/* test rt_tree_setdefault */
static status test6()
{
	rt_tree *t;
	char *val = "ABCDEFGH",*v2="012345",*r;
	status ret = PASS;
	t = rt_tree_new(16,NULL);
	if(!t) return ERR;

	r = rt_tree_setdefault(NULL,"ABC",3,val);
	ASSERT(!r)

	r = rt_tree_setdefault(t,NULL,2,val);
	ASSERT(!r)

	r = rt_tree_setdefault(t,"ABC",0,val);
	ASSERT(!r)

	r = rt_tree_setdefault(t,"ABC",3,NULL);
	ASSERT(!r)

	r = rt_tree_setdefault(t,"ABC",3,val);
	ASSERT(r==val)

	/* verify */
	r = NULL;
	r = rt_tree_get(t,"ABC",3);
	ASSERT(r==val)

	/* try to set a different value, this should fail and return the already set val value */
	r = rt_tree_setdefault(t,"ABC",3,v2);
	ASSERT(r==val)
	/* verify */
	r = rt_tree_get(t,"ABC",3);
	ASSERT(r==val)

	r = rt_tree_setdefault(t,"AB",2,v2);
	ASSERT(r==v2)
	r = rt_tree_get(t,"AB",2);
	ASSERT(r==v2)

	if(!t) return ERR;
	rt_tree_free(t);
	return ret;
}

int
main()
{
	unsigned int tests=0, succ=0;

	TEST(test1());
	TEST(test2());
	TEST(test3());
	TEST(test4());
	TEST(test5());
	TEST(test6());

#ifndef NDEBUG
	printf("%s: Passed %u of %u tests\n",tests-succ==0?"SUCCESS":"FAILED",succ,tests);
#endif

	return tests-succ;
}
