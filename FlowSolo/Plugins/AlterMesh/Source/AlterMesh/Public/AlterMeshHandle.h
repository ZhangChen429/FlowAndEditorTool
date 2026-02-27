#pragma once
#include "Extern/AlterMesh.h"

struct FAlterMeshHandle
{
	void* Get() const
	{
		return Handle;
	}
	
	void Set(void* InPtr)
	{
		Handle = InPtr;
	}
	
	~FAlterMeshHandle()
	{
		Free(Handle);
		Handle = nullptr;
	}
	
private:
	
	void* Handle = nullptr;
};
