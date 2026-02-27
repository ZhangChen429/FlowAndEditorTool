// Copyright 2023 Aechmea

#pragma once
#include "AlterMeshAsset.h"
#include "AlterMeshSection.h"
#include "AlterMeshConverter.generated.h"

class AAlterMeshActor;

// Base class for converters
UCLASS(Abstract, BlueprintType, Blueprintable)
class ALTERMESHEDITOR_API UAlterMeshConverterBase : public UObject
{
	GENERATED_BODY()

public:
	
	// Helper function to iterate sections
	void ForEachSection(AAlterMeshActor* InActor, TFunctionRef<void(FAlterMeshSection& Section)> Function, bool bIncludeInstances = false);

	// Display preview of the conversion, called at every import
	//virtual void Preview(AAlterMeshActor* InActor) { };
	
	// Do actual conversion, create asset here
	virtual void Convert(AAlterMeshActor* InActor);

	// Do conversion, create asset here, parent class native code is always called before
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="Convert"))
	void K2_Convert(AAlterMeshActor* InActor);

	// Called when going into preview mode, actor deleted or converter class changed
	UFUNCTION()
	virtual void CleanUp();
};
