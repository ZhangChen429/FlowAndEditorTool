#pragma once

#include "CoreMinimal.h"
#include "AlterMeshExport.h"
#include "AlterMeshImport.h"
#include "Input/DragAndDrop.h"
#include "AlterMeshGeometryBase.generated.h"


// Base class for geometry params
// Subclass to add your own custom geometry interfaces
// must be matched with a geometry importer in /ThirdParty/GeometryTypes folder
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, BlueprintType, Blueprintable)
class ALTERMESH_API UAlterMeshGeometryBase : public UObject
{
	GENERATED_BODY()
public:

	UAlterMeshGeometryBase ();

	// Override to decide if it should even try to export, because of missing parameters etc.
	virtual bool ShouldExport() { return true; };

	// Called when input params are updated
	virtual void Initialize(AActor* OwnerActor);
	
	// Do cleanup if this param is not used anymore
	virtual void Cleanup();

	// Override to support replacing output instances with Unreal assets
	// If this returns an asset, it will be placed instead of creating a new mesh
	// See UAlterMeshGeometryAsset for example
	virtual UObject* GetAsset() { return nullptr; };

	// Before exporting for blender
	virtual void PreExport(FAlterMeshExport& Exporter) {};
	
	// Does actual exporting for blender
	virtual void Export(FAlterMeshExport& Exporter) {};

	// Implement if your type accepts default values
	// must be paired with get_defaults() on /ThirdParty/GeometryTypes
	virtual void ImportDefaults(FAlterMeshImport& Importer) {};

	// If returns true, this class accepts a drop operation
	// Eg. dragging an actor or asset will create an instance of this class
	// @Param OperationIndex: If it was set from a collection, indicates which asset is being processed
	virtual bool CanDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex) { return false; };

	// Create a instance of this class and use the dropped asset
	// @Param OperationIndex: If it was set from a collection, indicates which asset is being processed
	virtual void OnDropOperation(TSharedPtr<FDragDropOperation> DragDropOperation, int32 OperationIndex) {};

	// Called when child objects need to be duplicated for saving or initializing
	virtual void OnDuplicate(UObject* NewOuter);

	void ReattachComponent(USceneComponent* Component, UObject* NewOuter);
};
