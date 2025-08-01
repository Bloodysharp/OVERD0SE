#pragma once

//#include "../../sdk/memory/memory.h"
#include "../datatypes/utlbufferstring.h"

struct ResourceBinding_t;

class IResourceSystem {
public:
	void* QueryInterface(const char* szInterfaceName) {
		return MEM::CallVFunc<void*, 2U>(this, szInterfaceName);
	}

	void PreCache(const char* szName) {
		CBufferString pBufferString(szName);
		return MEM::CallVFunc<void, 48>(this, &pBufferString, "");
	}
};

class CResourceHandleUtils {
public:
	void DeleteResource(const ResourceBinding_t* pBinding) {
		MEM::CallVFunc<void, 2U>(this, pBinding);
	}
};