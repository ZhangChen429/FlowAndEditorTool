// Copyright 2023 Aechmea

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"
#include "AlterMeshAssetFactory.generated.h"

#define LOCTEXT_NAMESPACE "AlterMeshActorBlueprintFactory"

class UAlterMeshAsset;
DECLARE_LOG_CATEGORY_EXTERN(LogAlterMeshAssetFactory, Log, All);

/**
*
*/
UCLASS()
class UAlterMeshAssetFactory : public UFactory
{
	GENERATED_BODY()

	public:
	UAlterMeshAssetFactory();

	void ShowImportDialog();
	TArray<TSharedPtr<FString>> ListViewObjects;
	TArray<FString> SelectedObjects;
	bool bImport;

	bool bReimporting;
	UAlterMeshAsset* ReimportedObject;

	TSharedPtr<SListView<TSharedPtr<FString>>> ListView;
	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

	// Begin UFactory Interface
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	// End UFactory Interface

};

class FAlterMeshActorBlueprint_AssetTypeActions : public FAssetTypeActions_Base {
public:

	FAlterMeshActorBlueprint_AssetTypeActions(EAssetTypeCategories::Type InAssetCategoryBit)
		: AssetCategoryBit(InAssetCategoryBit)
	{}

	virtual FText GetName() const override { return LOCTEXT("AlterMeshActor","AlterMesh Instance"); }
	virtual FColor GetTypeColor() const override { return FColor(255,127,255); }
	virtual FText GetAssetDescription(const FAssetData &AssetData) const override { return LOCTEXT("AlterMeshActorDesc","todo description"); }
	virtual UClass* GetSupportedClass() const override { return UBlueprint::StaticClass(); }
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override { return AssetCategoryBit; }

private:
	EAssetTypeCategories::Type AssetCategoryBit;
};
#undef LOCTEXT_NAMESPACE