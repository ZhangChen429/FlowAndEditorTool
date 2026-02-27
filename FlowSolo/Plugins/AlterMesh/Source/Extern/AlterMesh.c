// Copyright 2023 Aechmea

#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <stdbool.h>
#include "AlterMesh.h"

struct BufferView
{
	size_t Offset;
	char* Address;
	size_t MaxSize;
	void* Empty;
	void* Full;
	void* Handle;
	size_t AllocatedSize;
};

#define EXPORT_BUFFER Handle[0]
#define IMPORT_BUFFER Handle[1]

#define GIGABYTE 1024*1024*1024
#define MEGABYTE 1024*1024

const wchar_t* CreateGuid(wchar_t* Name, const wchar_t* Guid)
{
	wchar_t* MutexName = malloc(50 * sizeof(wchar_t));
	MutexName[0] = L'\0';
	wcsncat_s(MutexName, 50, Name, wcslen(Name));	 
	wcsncat_s(MutexName, 50, Guid, wcslen(Guid));
	return MutexName;
}

void* Init(const wchar_t* Guid, const wchar_t* Guid2)
{
	struct BufferView ImportBuffer = { .Offset = 0, .Address = 0, .MaxSize = 1ull * GIGABYTE, .Full = 0, .Empty = 0, .Handle = 0, .AllocatedSize = 0 };
	struct BufferView ExportBuffer = { .Offset = 0, .Address = 0, .MaxSize = 1ull * GIGABYTE, .Full = 0, .Empty = 0, .Handle = 0, .AllocatedSize = 0 };

	struct BufferView* AlterMeshHandle = malloc(2 * sizeof(struct BufferView));

	ExportBuffer.Handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_RESERVE,
									ExportBuffer.MaxSize >> 32, ExportBuffer.MaxSize & 0xFFFFFFFF, Guid);
	ImportBuffer.Handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_RESERVE,
									ImportBuffer.MaxSize >> 32, ImportBuffer.MaxSize & 0xFFFFFFFF, Guid2);

	if (ExportBuffer.Handle)
	{
		ExportBuffer.Address = MapViewOfFile(ExportBuffer.Handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		// Semaphores
		ExportBuffer.Full = CreateSemaphoreW(0, 0, 1, CreateGuid(L"F-", Guid));
		ExportBuffer.Empty = CreateSemaphoreW(0, 1, 1, CreateGuid(L"E-", Guid));

    	// Initial allocation
		VirtualAlloc(ExportBuffer.Address, MEGABYTE, MEM_COMMIT, PAGE_READWRITE);
		ExportBuffer.AllocatedSize = MEGABYTE;
	}

    if (ImportBuffer.Handle)
    {
        ImportBuffer.Address = MapViewOfFile(ImportBuffer.Handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		// Semaphores
    	ImportBuffer.Full = CreateSemaphoreW(0, 0, 1, CreateGuid(L"F-", Guid2));
    	ImportBuffer.Empty = CreateSemaphoreW(0, 1, 1, CreateGuid(L"E-", Guid2));

    	// Initial allocation
    	VirtualAlloc(ImportBuffer.Address, MEGABYTE, MEM_COMMIT, PAGE_READWRITE);
    	ImportBuffer.AllocatedSize = MEGABYTE;
    }

	AlterMeshHandle[0] = ExportBuffer;
	AlterMeshHandle[1] = ImportBuffer;

	return AlterMeshHandle;
}

void Free(struct BufferView* Handle)
{
	if (EXPORT_BUFFER.Address)
		UnmapViewOfFile(EXPORT_BUFFER.Address);
	if (IMPORT_BUFFER.Address)
		UnmapViewOfFile(IMPORT_BUFFER.Address);
	if (EXPORT_BUFFER.Handle)
		CloseHandle(EXPORT_BUFFER.Handle);
	if (IMPORT_BUFFER.Handle)
		CloseHandle(IMPORT_BUFFER.Handle);

	free(Handle);
}

bool ReadLock(struct BufferView* Handle)
{
	bool bLocked = WaitForSingleObject(IMPORT_BUFFER.Full, -1) == WAIT_OBJECT_0;
	if (bLocked)
	{
		IMPORT_BUFFER.Offset = 0;
	}
	
	return bLocked;
}

bool WriteLock(struct BufferView* Handle)
{
	bool bLocked = WaitForSingleObject(EXPORT_BUFFER.Empty, -1) == WAIT_OBJECT_0;
	if (bLocked)
	{
		EXPORT_BUFFER.Offset = 0;
	}
	
	return bLocked;
}

void Allocate(struct BufferView* Handle)
{
	VirtualAlloc(Handle->Address + Handle->AllocatedSize, Handle->AllocatedSize, MEM_COMMIT, PAGE_READWRITE);
	Handle->AllocatedSize *= 2;
}

void Write(struct BufferView* Handle, const char* Source, const size_t Length)
{
	// Allocate enough to write
	while (EXPORT_BUFFER.AllocatedSize < EXPORT_BUFFER.Offset + Length)
	{
		Allocate(&EXPORT_BUFFER);
	}

	const int HeaderSize = sizeof(int);
	const int ChunkSize = (int)Length + HeaderSize;
    memcpy(EXPORT_BUFFER.Address + EXPORT_BUFFER.Offset, &ChunkSize, HeaderSize);
    EXPORT_BUFFER.Offset += HeaderSize;

    memcpy(EXPORT_BUFFER.Address + EXPORT_BUFFER.Offset, Source, Length);
    EXPORT_BUFFER.Offset += Length;
}
  
bool Read(struct BufferView* Handle, void** Address, size_t* OutLength)
{
    int ChunkSize;
    memcpy(&ChunkSize, IMPORT_BUFFER.Address + IMPORT_BUFFER.Offset, sizeof(int));

	// Allocated memory must be at least the whole chunk plus next chunk's header
	while (IMPORT_BUFFER.AllocatedSize < IMPORT_BUFFER.Offset + ChunkSize + sizeof(int))
	{
		Allocate(&IMPORT_BUFFER);
	}

	if (ChunkSize == 0)
	{
		if (OutLength)
			*OutLength = 0;

		return false;
	}
	else
	{
		IMPORT_BUFFER.Offset += sizeof(int);

		*Address = IMPORT_BUFFER.Address + IMPORT_BUFFER.Offset;

		IMPORT_BUFFER.Offset += ChunkSize - sizeof(int);

		if (OutLength)
			*OutLength = ChunkSize - sizeof(int);

		return true;
	}
}
  
void ReadUnlock(struct BufferView* Handle)
{
    memset(IMPORT_BUFFER.Address, 0, 4);
    ReleaseSemaphore(IMPORT_BUFFER.Empty, 1, NULL);
}

void WriteUnlock(struct BufferView* Handle)
{
    memset(EXPORT_BUFFER.Address + EXPORT_BUFFER.Offset, 0, 4);
    ReleaseSemaphore(EXPORT_BUFFER.Full, 1, NULL);
}
