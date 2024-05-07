#pragma once

#include <cstdint>

struct KFile;
struct CKLoadingManager;

// Kal object class
struct KClass {
	virtual void destructor_placeholder() = 0;
	virtual char isSubclass(int cls) = 0;
	virtual void reset() = 0;
#if XXLVER >= 2
	virtual void unknown2() = 0;
#endif
	virtual int getClassGroup() = 0;
	virtual int getClassID() = 0;
	virtual int sendEvent(int event, void *param) = 0;
	virtual char unknown3(void *arg0, void *arg4, void *arg8, void *argC) = 0;
	virtual int unknown4(void *arg0) = 0;
	virtual int serialize(KFile *file, void *arg4) = 0;
	virtual int deserialize(KFile *file, void *arg4) = 0;

	inline char isSubclass(int clgrp, int clid) {
		return isSubclass(clgrp | (clid << 6));
	}
};

#pragma pack(push, 1)
struct ClassContainer {
	void *objects;
	int32_t flags;
	void *u1;
	int16_t u2, count;
	int16_t clSize; char pl, u3;
};
#pragma pack(pop)

struct KclInfos {
	struct Tab {
		void *pnt;
		int32_t allocatedSize;
		int32_t numElements;
	};

	void *vtbl;
	Tab cls[15];
};

// Yellow pages: contains pointers to singleton objects
struct CKYellowPages {
	void *p_00,*p_04,*p_08,*p_0C;
	void *p_10,*p_14,*p_18; KclInfos* kclInfos;
	void *p_20,*p_24,*p_28,*p_2C;
	void *gameLoop,*p_34,*p_38,*p_3C;
	void *p_40; CKLoadingManager* loadingManager; void* savingManager, *p_4C;
	void *srvCollision,*p_54,*p_58,*p_5C;
	void *p_60,*p_64,*p_68,*p_6C;
	void *p_70,*p_74,*srvBeacon,*p_7C;
	void *p_80,*p_84,*p_88,*gameManager;
};

#define k_yellowPages (*(CKYellowPages**)0x6621F4)

KClass* kal_GetLvlObject(int clgrp, int clid, int instid);