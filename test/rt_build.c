
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

#define ALPHABET "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"
#define ALSIZE (strlen(ALPHABET))

int
main(int argc, char **argv)
{
	rt_tree *t;
	char **arg;
	int i, succ=0;

	t = rt_tree_new(ALSIZE,NULL);
	if(!t) {
#ifdef DEBUG
		printf("ERROR: Could not create rt_tree... Exiting\n");
#endif
		return (-1);
	}
	for(i=1,arg=argv+1;i<argc;i++,arg++)
	{
		if(rt_tree_set(t, *arg, strlen(*arg), *arg))
			succ++;
#ifdef DEBUG
		else printf("!!! Adding arg[%d] = %s... FAILED\n",i,*arg);
#endif
	}
#ifdef DEBUG
	printf("ADD Passed: %d of %d\n",succ,argc-1);
	rt_tree_print(t);
#endif
	if(succ!=argc-1) {
		rt_tree_free(t);
		return (-1);
	}
	succ = 0;
	for(i=argc-1,arg--;i>0;i--,arg--)
	{
		char *val;
		if(rt_tree_get(t, *arg, strlen(*arg), (void **)&val)) {
			if(!strcmp(val,*arg)) succ++;
#ifdef DEBUG
			else printf("!!! Value mismatch (%s != %s)\n",val,*arg);
#endif
		}
#ifdef DEBUG
		else printf("!!! Searching for \"%s\"(arg[%d])... FAILED\n",*arg,i);
#endif
	}
#ifdef DEBUG
	printf("SEARCH Passed: %d of %d\n",succ,argc-1);
#endif

	rt_tree_free(t);
	return succ==argc-1;
}
