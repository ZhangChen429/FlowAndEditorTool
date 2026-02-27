// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "StructView.h"
#include "StructUtils/StructArrayView.h"
#include "StructUtils/StructView.h"

// Custom types to support versioning
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
typedef FStructView FAlterMeshStructView;
typedef FStructArrayView FAlterMeshStructArrayView;
#endif

typedef FConstStructView FAlterMeshConstStructView;


#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
template <typename T>
struct FAlterMeshConstStructArrayView : FConstStructArrayView
{
	explicit FAlterMeshConstStructArrayView(const TArray<T>& InArray)
		: FConstStructArrayView(const_cast<TArray<T>&>(InArray))
	{}
	
};
#endif

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION <= 2
struct FAlterMeshStructView : FStructView
{
	FAlterMeshStructView(const UScriptStruct* InScriptStruct, uint8* InStructMemory = nullptr)
	: FStructView(InScriptStruct, InStructMemory)
	{
	
	}
public:
	template <typename InnerType>
	InnerType& Get()
	{
		return GetMutable<InnerType>();
	}
	
	template <typename InnerType>
	InnerType* GetPtr()
	{
		return GetMutablePtr<InnerType>();
	}
};

template <typename T>
struct FAlterMeshStructArrayView : FStructArrayView
{
	FAlterMeshStructArrayView(TArray<T>& InArray)
		:FStructArrayView(InArray)
	{
		
	}
	
	FAlterMeshStructView operator[](const int32 Index) const
	{
		return FAlterMeshStructView(&GetFragmentType(), (uint8*)GetDataAt(Index)); // GetDataAt() calls RangeCheck().
	}
	
	struct FIterator
	{
		FIterator(const FAlterMeshStructArrayView& InOwner, int32 InIndex)
			: Owner(&InOwner)
			, Index(InIndex)
		{}

		FIterator& operator++()
		{
			++Index;
			return *this;
		}

		FAlterMeshStructView operator*() const
		{
			return (*Owner)[Index];
		}

		FORCEINLINE bool operator == (const FIterator& Other) const
		{
			return Owner == Other.Owner
				&& Index == Other.Index;
		}

		FORCEINLINE bool operator != (const FIterator& Other) const
		{
			return !(*this == Other);
		}

	private:
		const FAlterMeshStructArrayView* Owner = nullptr;
		int32 Index = INDEX_NONE;
	};
	
	/** Ranged iteration support. DO NOT USE DIRECTLY. */
	FORCEINLINE FIterator begin() const { return FIterator(*this, 0); }
	FORCEINLINE FIterator end() const { return FIterator(*this, Num()); }
	
};

#endif