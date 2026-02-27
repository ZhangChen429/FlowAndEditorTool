// Copyright 2023 Aechmea

#include "AlterMeshParam.h"

#include "CommonParams/AlterMeshCollectionParam.h"
#include "CommonParams/AlterMeshGeometryParam.h"
#include "GeometryParams/AlterMeshGeometryAsset.h"

/*
FAlterMeshInputParams& FAlterMeshInputParams::operator=(const FAlterMeshInputParams& Other)
{
	Frame = Other.Frame;
	Params = Other.Params;
	return *this;
}*/

FAlterMeshInputParams& FAlterMeshInputParams::Copy(const FAlterMeshInputParams& Other, UObject* NewOuter)
{
	Frame = Other.Frame;
	Params = Other.Params;

	for (FAlterMeshGeometryParam* Param : GetTyped<FAlterMeshGeometryParam>())
	{
		if (Param->Data.InputType)
		{			
			Param->Data.InputType = DuplicateObject<UAlterMeshGeometryBase>(Param->Data.InputType, NewOuter);
			Param->Data.InputType->SetFlags(RF_Transactional);
			Param->Data.InputType->OnDuplicate(NewOuter);
		}
	}

	for (FAlterMeshCollectionParam* Param : GetTyped<FAlterMeshCollectionParam>())
	{
		for (FAlterMeshGeometryParam& GeometryParam : Param->Data.Geometries)
		{
			if (GeometryParam.Data.InputType)
			{
				GeometryParam.Data.InputType = DuplicateObject<UAlterMeshGeometryBase>(GeometryParam.Data.InputType, NewOuter);
				GeometryParam.Data.InputType->SetFlags(RF_Transactional);
				GeometryParam.Data.InputType->OnDuplicate(NewOuter);
			}
		}
	}

	return *this;
}

void FAlterMeshInputParams::Clear()
{
	
}
