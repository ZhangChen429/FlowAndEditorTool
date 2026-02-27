// Copyright 2023 Aechmea

#include "AlterMeshParamBaseCustomization.h"

TSharedRef<IPropertyTypeCustomization> FAlterMeshParamBaseCustomization::MakeInstance()
{
	return MakeShareable(new FAlterMeshParamBaseCustomization);
}
