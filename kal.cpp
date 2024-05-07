#include "kal.hpp"
#include <cassert>

KClass* kal_GetLvlObject(int clgrp, int clid, int instid)
{
	void *kcls = *(void**)((char*)k_yellowPages->loadingManager + 4);
	void *grp = *(void**)((char*)kcls + 12*clgrp);
	ClassContainer *cl = (ClassContainer*)grp + clid;
	char pl = cl->pl;
	int32_t flg = cl->flags;
	int16_t cnt = cl->count;
	int16_t csz = cl->clSize;
	char *fstobj = (char*)cl->objects;
	if(!cnt) return 0;
	assert(instid < cnt);
	if(pl)
		return (KClass*)(fstobj + csz*instid);
	else if(flg >> 17 == 1)
		return (KClass*)fstobj;
	else
		return (KClass*)(((void**)fstobj)[instid]);
}