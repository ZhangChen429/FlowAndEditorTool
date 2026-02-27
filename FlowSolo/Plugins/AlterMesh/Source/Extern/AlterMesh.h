// Copyright 2023 Aechmea

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

__declspec(dllexport) void* Init(const wchar_t* Guid, const wchar_t* Guid2);
__declspec(dllexport) void Free(void* Handle);
__declspec(dllexport) bool Read(void* Handle, void** Address, size_t* OutLength);
__declspec(dllexport) void Write(void* Handle, const char* Source, const size_t Length);
__declspec(dllexport) bool ReadLock(void* Handle);
__declspec(dllexport) bool WriteLock(void* Handle);
__declspec(dllexport) void ReadUnlock(void* Handle);
__declspec(dllexport) void WriteUnlock(void* Handle);
#ifdef __cplusplus
}
#endif