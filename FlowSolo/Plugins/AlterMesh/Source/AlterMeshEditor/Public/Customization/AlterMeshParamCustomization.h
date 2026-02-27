// Copyright 2023 Aechmea

#pragma once
#include "AlterMeshAsset.h"

#include "IPropertyTypeCustomization.h"
#include "Templates/SharedPointer.h"
#include "PropertyHandle.h"

class UAlterMeshAsset;

struct ALTERMESHEDITOR_API FAlterMeshParamCustomizationUtils
{
	template<typename T>
	static void SetMinMaxValues(TSharedRef<IPropertyHandle> PropertyHandle);

	static void RenameProperty(TSharedRef<IPropertyHandle> ParamDataHandle, FName PropertyToRename = FName("Value"));
	
	template <typename T>
	static T* GetAssetParamFromHandle(TSharedPtr<IPropertyHandle> PropertyHandle);

private:
	
	static UAlterMeshAssetInterface* GetAssetFromProperty(TSharedPtr<IPropertyHandle> PropertyHandle);
};

template <typename T>
T* FAlterMeshParamCustomizationUtils::GetAssetParamFromHandle(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	TSharedPtr<IPropertyHandle> StructProperty = PropertyHandle->GetParentHandle()->GetParentHandle();
	void* Data = nullptr;
	StructProperty->GetChildHandle(FName("BaseData"))->GetChildHandle(FName("Id"))->GetValueData(Data);
	FAlterMeshParamId* ParamId = reinterpret_cast<FAlterMeshParamId*>(Data);
	
	UAlterMeshAssetInterface* Asset = GetAssetFromProperty(PropertyHandle);
	if (Asset && ParamId)
	{
		if (T* Param = Asset->GetParams().FindParamById<T>(*ParamId))
		{
			return Param;
		}
	}

	return nullptr;
}
