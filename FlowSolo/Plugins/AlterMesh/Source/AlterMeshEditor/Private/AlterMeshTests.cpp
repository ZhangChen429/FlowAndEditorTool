// Copyright 2023 Aechmea

#include "AlterMeshAsset.h"
#include "AlterMeshComponent.h"
#include "AlterMeshInstance.h"
#include "AlterMeshLibrary.h"
#include "AlterMeshSettings.h"
#include "AlterMeshTransformComponent.h"
#include "CoreTypes.h"
#include "Landscape.h"
#include "NiagaraComponent.h"
#include "CommonParams/AlterMeshBoolParam.h"
#include "CommonParams/AlterMeshCollectionParam.h"
#include "CommonParams/AlterMeshColorParam.h"
#include "CommonParams/AlterMeshFloatParam.h"
#include "CommonParams/AlterMeshIntParam.h"
#include "CommonParams/AlterMeshStringParam.h"
#include "CommonParams/AlterMeshTextureParam.h"
#include "CommonParams/AlterMeshVectorParam.h"
#include "Components/SplineComponent.h"
#include "Containers/UnrealString.h"
#include "Engine/StaticMeshActor.h"
#include "GeometryParams/AlterMeshGeometryActor.h"
#include "GeometryParams/AlterMeshGeometryLandscape.h"
#include "GeometryParams/AlterMeshGeometryParticleData.h"
#include "GeometryParams/AlterMeshGeometryTransform.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"


// Tests that import a mesh into unreal using an existing asset
class FAlterMeshImportMeshTestBase : public FAutomationTestBase
{
public:
	FAlterMeshImportMeshTestBase(const FString& InName, bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask)
	{
	}

	TStrongObjectPtr<AAlterMeshActor> CreatedActor = nullptr;
	TStrongObjectPtr<UWorld> CreatedWorld = nullptr;
	
	virtual void CleanUpWorld()
	{
		CreatedActor->Destroy();
		CreatedActor.Reset();
		CreatedWorld.Reset();
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}
	
	virtual void ImportMeshFromUnitTestAsset(int32 AssetNum)
	{		
		const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();
		CreatedWorld = TStrongObjectPtr(FAutomationEditorCommonUtils::CreateNewMap());

		FActorSpawnParameters Params;
		Params.bNoFail = true;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CreatedActor = TStrongObjectPtr(CreatedWorld->SpawnActor<AAlterMeshActor>(Params));
		CreatedActor->Asset = Settings->UnitTestAssets[AssetNum].LoadSynchronous()->Get();
		CreatedActor->InitializeParamsFromAsset();
		CreatedActor->RefreshSync();
	}

	bool RunTestOnFile();
};

#define IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(TClass, PrettyName) \
	IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(TClass, FAlterMeshImportMeshTestBase, PrettyName, EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Tests that import an asset from a .blend file
class FAlterMeshImportAssetTestBase : public FAutomationTestBase
{
public:
	FAlterMeshImportAssetTestBase(const FString& InName, bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask)
	{
	}

	TStrongObjectPtr<UAlterMeshAsset> CreatedAsset;
};

#define IMPLEMENT_ALTERMESH_IMPORT_ASSET_TEST(TClass, PrettyName) \
	IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(TClass, FAlterMeshImportAssetTestBase, PrettyName, EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshDefaultPlane, "AlterMesh.ImportMesh.DefaultPlane")

bool FAlterMeshImportMeshDefaultPlane::RunTest(const FString& Parameters)
{	
	ImportMeshFromUnitTestAsset(0);

	TestEqual(TEXT("Section0 VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 4);

	TestEqual(TEXT("Section0 Vert0 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[0]), 0.0), FVector(0.0, 0.0, 0.0));
	TestEqual(TEXT("Section0 Vert1 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[1]), 0.0), FVector(1.0, 0.0, 0.0));
	TestEqual(TEXT("Section0 Vert2 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[2]), 0.0), FVector(1.0, 1.0, 0.0));
	TestEqual(TEXT("Section0 Vert3 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[3]), 0.0), FVector(0.0, 1.0, 0.0));

	TestEqual(TEXT("Section0 Colors0"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[0], FColor(0.0, 0.0, 0.0, 0.0));
	TestEqual(TEXT("Section0 Colors1"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[1], FColor(0.0, 0.0, 0.0, 0.0));
	TestEqual(TEXT("Section0 Colors2"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[2], FColor(0.0, 0.0, 0.0, 0.0));
	TestEqual(TEXT("Section0 Colors3"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[3], FColor(0.0, 0.0, 0.0, 0.0));	

	TestEqual(TEXT("Section0 Vert0 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[0]), FVector(0.0,0.0,1.0));
	TestEqual(TEXT("Section0 Vert1 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[1]), FVector(0.0,0.0,1.0));
	TestEqual(TEXT("Section0 Vert2 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[2]), FVector(0.0,0.0,1.0));
	TestEqual(TEXT("Section0 Vert3 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[3]), FVector(0.0,0.0,1.0));	

	TestEqual(TEXT("Section0 Vert0 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), FVector(100.0,100.0,0.0));
	TestEqual(TEXT("Section0 Vert1 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), FVector(100.0,-100.0,0.0));
	TestEqual(TEXT("Section0 Vert2 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), FVector(-100.0,-100.0,0.0));
	TestEqual(TEXT("Section0 Vert3 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), FVector(-100.0,100.0,0.0));

	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshColorTest, "AlterMesh.ImportMesh.Colors")

bool FAlterMeshImportMeshColorTest::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(4);
	
	TestEqual(TEXT("Section0 Vert0 Colors"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[0], FColor(0, 255, 0));
	TestEqual(TEXT("Section0 Vert1 Colors"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[1], FColor(0, 54, 0));
	TestEqual(TEXT("Section0 Vert2 Colors"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[2], FColor(54, 0, 0));
	TestEqual(TEXT("Section0 Vert3 Colors"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[3], FColor(255, 0, 0));
	TestEqual(TEXT("Section0 Vert4 Colors"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[4], FColor(255, 255, 0));
	TestEqual(TEXT("Section0 Vert5 Colors"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[5], FColor(255, 0, 0));
	TestEqual(TEXT("Section0 Vert6 Colors"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[6], FColor(255, 255, 255));
	TestEqual(TEXT("Section0 Vert7 Colors"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[7], FColor(0, 0, 255));
	TestEqual(TEXT("Section0 Vert8 Colors"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors[8], FColor(0, 0, 54));		

	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshMeshUVTest, "AlterMesh.ImportMesh.MeshUVs")

bool FAlterMeshImportMeshMeshUVTest::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(1);
	
	TestEqual(TEXT("Section0 Vert0 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[0]), 0.0), FVector(0.5f, 0.5f, 0.0));
	TestEqual(TEXT("Section0 Vert1 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[1]), 0.0), FVector(1.0, 0.5f, 0.0));
	TestEqual(TEXT("Section0 Vert2 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[2]), 0.0), FVector(2.0, 1.0, 0.0));
	TestEqual(TEXT("Section0 Vert3 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[3]), 0.0), FVector(0.5f, 1.0, 0.0));
	TestEqual(TEXT("Section0 Vert4 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[4]), 0.0), FVector(0.0, 0.5f, 0.0));
	TestEqual(TEXT("Section0 Vert5 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[5]), 0.0), FVector(-1.0, 1.0, 0.0));
	TestEqual(TEXT("Section0 Vert6 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[6]), 0.0), FVector(0.0, -1.0, 0.0));
	TestEqual(TEXT("Section0 Vert7 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[7]), 0.0), FVector(0.5f, 0.0, 0.0));
	TestEqual(TEXT("Section0 Vert8 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[8]), 0.0), FVector(1.0, 2.0, 0.0));

	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshAttributeUVTest, "AlterMesh.ImportMesh.AttributeUVs")

bool FAlterMeshImportMeshAttributeUVTest::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(2);
	
	TestEqual(TEXT("Section0 Vert0 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[0]), 0.0), FVector(0.5f, 0.5f, 0.0));
	TestEqual(TEXT("Section0 Vert1 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[1]), 0.0), FVector(1.0, 0.5f, 0.0));
	TestEqual(TEXT("Section0 Vert2 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[2]), 0.0), FVector(1.0, 1.0, 0.0));
	TestEqual(TEXT("Section0 Vert3 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[3]), 0.0), FVector(0.5f, 1.0, 0.0));
	TestEqual(TEXT("Section0 Vert4 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[4]), 0.0), FVector(0.0, 0.5f, 0.0));
	TestEqual(TEXT("Section0 Vert5 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[5]), 0.0), FVector(0.0, 1.0, 0.0));
	TestEqual(TEXT("Section0 Vert6 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[6]), 0.0), FVector(0.0, 0.0, 0.0));
	TestEqual(TEXT("Section0 Vert7 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[7]), 0.0), FVector(0.5f, 0.0, 0.0));
	TestEqual(TEXT("Section0 Vert8 UVs"), FVector(FVector2d(CreatedActor->AlterMeshComponents[0]->Sections[0].UV0[8]), 0.0), FVector(1.0, 0.0, 0.0));

	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshVerticesTest, "AlterMesh.ImportMesh.Vertices")

bool FAlterMeshImportMeshVerticesTest::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(3);

	TestEqual(TEXT("Section0 Vert0 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), FVector(0.0,0.0,0.0));
	TestEqual(TEXT("Section0 Vert1 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), FVector(0.0,-100.0,0.0));
	TestEqual(TEXT("Section0 Vert2 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), FVector(-100.0,-100.0,200.0));
	TestEqual(TEXT("Section0 Vert3 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), FVector(-200.0,0.0,0.0));
	TestEqual(TEXT("Section0 Vert4 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[4]), FVector(-0.0,100.0,0.0));
	TestEqual(TEXT("Section0 Vert5 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[5]), FVector(0.0,0.0,0.0));
	TestEqual(TEXT("Section0 Vert6 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[6]), FVector(-200.0,0.0,0.0));
	TestEqual(TEXT("Section0 Vert7 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[7]), FVector(-100.0,100.0,0.0));
	TestEqual(TEXT("Section0 Vert8 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[8]), FVector(100.0,0.0,-100.0));

	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshStressTest, "AlterMesh.ImportMesh.Stress")

bool FAlterMeshImportMeshStressTest::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(11);

	TestEqual(TEXT("Section0 VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 1050625);
	TestEqual(TEXT("Section0 UVs Num"), CreatedActor->AlterMeshComponents[0]->Sections[0].UV0.Num(), 1050625);
	TestEqual(TEXT("Section0 Colors Num"), CreatedActor->AlterMeshComponents[0]->Sections[0].Colors.Num(), 1050625);

	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshSections, "AlterMesh.ImportMesh.Sections")

bool FAlterMeshImportMeshSections::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(6);
	
	TestEqual(TEXT("Section0 SectionNum"), CreatedActor->AlterMeshComponents[0]->Sections.Num(), 2);
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshWeldVerts, "AlterMesh.ImportMesh.WeldVerts")

bool FAlterMeshImportMeshWeldVerts::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(9);

	TestEqual(TEXT("Section0 VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 9);
	
	CleanUpWorld();
	return !HasAnyErrors();
}


IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshSplitUVs, "AlterMesh.ImportMesh.SplitUVs")

bool FAlterMeshImportMeshSplitUVs::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(8);

	TestEqual(TEXT("Section0 VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 12);
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshSplitColors, "AlterMesh.ImportMesh.SplitColors")

bool FAlterMeshImportMeshSplitColors::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(5);

	TestEqual(TEXT("Section0 VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 8);
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshSplitNormals, "AlterMesh.ImportMesh.SplitNormals")

bool FAlterMeshImportMeshSplitNormals::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(7);
	
	TestEqual(TEXT("Section0 VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 8);
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshImportMeshCube, "AlterMesh.ImportMesh.DefaultCube")

bool FAlterMeshImportMeshCube::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(12);
	
	TestEqual(TEXT("Section0 VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 24);
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshGeometrySpline, "AlterMesh.ImportMesh.GeometrySpline")

bool FAlterMeshGeometrySpline::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(21);

	CreatedActor->InitializeAllGeometryParams();
	CreatedActor->RefreshSync();

	TArray<USplineComponent*> OutComponents;	
	CreatedActor->GetComponents<USplineComponent>(OutComponents);
	TestEqual(TEXT("Spawned Spline Components"), OutComponents.Num(), 1);

	
	TestEqual(TEXT("Spline points Num"), OutComponents[0]->GetNumberOfSplinePoints(), 3);

	FVector P0 = FVector(100,0,0); 
	FVector P1 = FVector(0,100,0); 
	FVector P2 = FVector(0,0,100);
	
	FSplinePoint ExpectedPoint0(0,  P0, ESplinePointType::Curve, FRotationMatrix::MakeFromX(P1 - P0).Rotator(), FVector(1,1,1));
	FSplinePoint ExpectedPoint1(0, P1, ESplinePointType::Curve, FRotationMatrix::MakeFromX((P1 - P0) + (P2 - P1)).Rotator(), FVector(1,1,1));
	FSplinePoint ExpectedPoint2(0, P2, ESplinePointType::Curve, FRotationMatrix::MakeFromX(P2 - P1).Rotator(), FVector(1,1,1));
	
	TestEqual(TEXT("SplinePoint Pos0"), OutComponents[0]->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local), ExpectedPoint0.Position);
	TestEqual(TEXT("SplinePoint Pos1"), OutComponents[0]->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::Local), ExpectedPoint1.Position);
	TestEqual(TEXT("SplinePoint Pos2"), OutComponents[0]->GetLocationAtSplinePoint(2, ESplineCoordinateSpace::Local), ExpectedPoint2.Position);

	TestEqual(TEXT("SplinePoint Rot0"), OutComponents[0]->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::Local), ExpectedPoint0.Rotation);
	TestEqual(TEXT("SplinePoint Rot1"), OutComponents[0]->GetRotationAtSplinePoint(1, ESplineCoordinateSpace::Local), ExpectedPoint1.Rotation);
	TestEqual(TEXT("SplinePoint Rot2"), OutComponents[0]->GetRotationAtSplinePoint(2, ESplineCoordinateSpace::Local), ExpectedPoint2.Rotation);
	
	TestEqual(TEXT("SplinePoint Scale0"), OutComponents[0]->GetScaleAtSplinePoint(0), ExpectedPoint0.Scale);
	TestEqual(TEXT("SplinePoint Scale1"), OutComponents[0]->GetScaleAtSplinePoint(1), ExpectedPoint1.Scale);
	TestEqual(TEXT("SplinePoint Scale2"), OutComponents[0]->GetScaleAtSplinePoint(2), ExpectedPoint2.Scale);
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshGeometryTransform, "AlterMesh.ImportMesh.GeometryTransform")

bool FAlterMeshGeometryTransform::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(22);
	
	CreatedActor->InitializeAllGeometryParams();
	CreatedActor->RefreshSync();
	
	TArray<UAlterMeshTransformComponent*> OutComponents;	
	CreatedActor->GetComponents<UAlterMeshTransformComponent>(OutComponents);
	TestEqual(TEXT("Spawned Transform Components"), OutComponents.Num(), 1);

	check(OutComponents.Num())	
	FTransform AddedTransform = FTransform(FRotator(10,20,30), FVector(10.0, 15.0, 20.0), FVector(1,2,3));
	OutComponents[0]->AddLocalTransform(AddedTransform);
	CreatedActor->RefreshSync();
	
	TestEqual(TEXT("TransformComponent transform"), OutComponents[0]->GetComponentTransform(),  AddedTransform);
	TestEqual(TEXT("Instance transform"), FTransform(FTransform3f(CreatedActor->AlterMeshComponents[0]->Sections[0].Instances[0])),  AddedTransform);
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshGeometryParticleData, "AlterMesh.ImportMesh.GeometryParticleData")

bool FAlterMeshGeometryParticleData::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(20);
		
	const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();
	auto* ParticleParam = Cast<UAlterMeshGeometryParticleData>(UAlterMeshLibrary::GetGeometryParam(CreatedActor.Get(), FString("Object")));	
	ParticleParam->ParticleSystem = Cast<UNiagaraSystem>(Settings->UnitTestHelperAssets[1].LoadSynchronous());
	
	CreatedActor->InitializeAllGeometryParams();
	CreatedActor->RefreshSync();

	// todo particle data is async by nature, so we need to wait for the data to be loaded
	// this is a temporary solution
	FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
	
	CreatedActor->RefreshSync();

	TArray<UNiagaraComponent*> OutComponents;	
	CreatedActor->GetComponents<UNiagaraComponent>(OutComponents);
	TestEqual(TEXT("Spawned Particle Components"), OutComponents.Num(), 1);
	
	TestEqual(TEXT("Particle Sections"), CreatedActor->AlterMeshComponents[0]->Sections.Num(), 1);
	TestTrue(TEXT("Particle Verts"), !!CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num());
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshGeometryAssetRelative, "AlterMesh.ImportMesh.GeometryAssetRelative")

bool FAlterMeshGeometryAssetRelative::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(16);
	
	const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();	
	
	auto* AssetParam = Cast<UAlterMeshGeometryAsset>(UAlterMeshLibrary::GetGeometryParam(CreatedActor.Get(), FString("Object")));
	check(AssetParam)
	
	AssetParam->Object = Cast<UStaticMesh>(Settings->UnitTestHelperAssets[0].LoadSynchronous());
	CreatedActor->RefreshSync();

	// asset without transform
	check(CreatedActor->AlterMeshComponents.Num() && CreatedActor->AlterMeshComponents[0]->Sections.Num())
	
	TestEqual(TEXT("NumSections"), CreatedActor->AlterMeshComponents[0]->Sections.Num(), 1);	
	TestEqual(TEXT("VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 4);
	TestEqual(TEXT("Section0 Vert0 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), FVector(0.0, -100.0, 100.0));
	TestEqual(TEXT("Section0 Vert1 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), FVector(100.0, 100.0, 100.0));
	TestEqual(TEXT("Section0 Vert2 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), FVector(100.0, 100.0, -100.0));
	TestEqual(TEXT("Section0 Vert3 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), FVector(0.0, -100.0, -100.0));

	FVector ExpectedNormal = FVector(0.894f,-0.447f,0.0);
	TestEqual(TEXT("Section0 Vert0 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[0]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert1 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[1]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert2 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[2]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert3 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[3]), ExpectedNormal, 0.001f);
	
	AssetParam->TransformOverride = FTransform(FRotator(10, 20, 30), FVector(10, 20, 30), FVector(10,20,30));
	CreatedActor->RefreshSync();

	// Asset with override transform
	check(CreatedActor->AlterMeshComponents.Num() && CreatedActor->AlterMeshComponents[0]->Sections.Num())
	
	TestEqual(TEXT("VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 4);
	const FVector ExpectedPos0 =  AssetParam->TransformOverride.TransformPosition(FVector(0.0, -100.0, 100.0));
	const FVector ExpectedPos1 =  AssetParam->TransformOverride.TransformPosition(FVector(100.0, 100.0, 100.0));
	const FVector ExpectedPos2 =  AssetParam->TransformOverride.TransformPosition(FVector(100.0, 100.0, -100.0)); 
	const FVector ExpectedPos3 =  AssetParam->TransformOverride.TransformPosition(FVector(0.0, -100.0, -100.0)); 
	TestEqual(TEXT("Section0 Vert0 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), ExpectedPos0, 0.001f);
	TestEqual(TEXT("Section0 Vert1 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), ExpectedPos1, 0.001f);
	TestEqual(TEXT("Section0 Vert2 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), ExpectedPos2, 0.001f);
	TestEqual(TEXT("Section0 Vert3 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), ExpectedPos3, 0.001f);

	// Calculate the inverse transpose of the matrix for correct normal transformation.
	FMatrix InverseTranspose = AssetParam->TransformOverride.ToMatrixWithScale().InverseFast().GetTransposed();
	FVector ExpectedNormalTransformed = InverseTranspose.TransformVector(FVector(0.894f, -0.447f, 0.0)).GetSafeNormal();
	
	TestEqual(TEXT("Section0 Vert0 normal after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[0]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert1 normal after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[1]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert2 normal after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[2]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert3 normal after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[3]), ExpectedNormalTransformed, 0.001f);

	CleanUpWorld();
	return !HasAnyErrors();	
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshGeometryAssetOriginal, "AlterMesh.ImportMesh.GeometryAssetOriginal")

bool FAlterMeshGeometryAssetOriginal::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(17);
	
	const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();	
	
	auto* AssetParam = Cast<UAlterMeshGeometryAsset>(UAlterMeshLibrary::GetGeometryParam(CreatedActor.Get(), FString("Object")));
	check(AssetParam)
	
	AssetParam->Object = Cast<UStaticMesh>(Settings->UnitTestHelperAssets[0].LoadSynchronous());
	CreatedActor->RefreshSync();

	// asset without transform
	check(CreatedActor->AlterMeshComponents.Num() && CreatedActor->AlterMeshComponents[0]->Sections.Num())
	
	TestEqual(TEXT("NumSections"), CreatedActor->AlterMeshComponents[0]->Sections.Num(), 1);	
	TestEqual(TEXT("VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 4);
	TestEqual(TEXT("Section0 Vert0 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), FVector(0.0, -100.0, 100.0));
	TestEqual(TEXT("Section0 Vert1 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), FVector(100.0, 100.0, 100.0));
	TestEqual(TEXT("Section0 Vert2 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), FVector(100.0, 100.0, -100.0));
	TestEqual(TEXT("Section0 Vert3 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), FVector(0.0, -100.0, -100.0));

	FVector ExpectedNormal = FVector(0.894f,-0.447f,0.0);
	TestEqual(TEXT("Section0 Vert0 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[0]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert1 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[1]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert2 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[2]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert3 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[3]), ExpectedNormal, 0.001f);

	// Asset with override transform
	AssetParam->TransformOverride = FTransform(FRotator(10, 20, 30), FVector(10, 20, 30), FVector(10,20,30));
	CreatedActor->RefreshSync();

	check(CreatedActor->AlterMeshComponents.Num() && CreatedActor->AlterMeshComponents[0]->Sections.Num())
	
	TestEqual(TEXT("VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 4);
	TestEqual(TEXT("Section0 Vert0 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), FVector(0.0, -100.0, 100.0));
	TestEqual(TEXT("Section0 Vert1 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), FVector(100.0, 100.0, 100.0));
	TestEqual(TEXT("Section0 Vert2 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), FVector(100.0, 100.0, -100.0));
	TestEqual(TEXT("Section0 Vert3 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), FVector(0.0, -100.0, -100.0));

	FVector ExpectedNormalTransformed = FVector(0.894f,-0.447f,0.0);
	TestEqual(TEXT("Section0 Vert0 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[0]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert1 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[1]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert2 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[2]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert3 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[3]), ExpectedNormalTransformed, 0.001f);

	CleanUpWorld();
	return !HasAnyErrors();	
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshGeometryActorRelative, "AlterMesh.ImportMesh.GeometryActorRelative")

bool FAlterMeshGeometryActorRelative::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(15);
	
	const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();
	
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AStaticMeshActor* StaticMeshActor = CreatedWorld->SpawnActor<AStaticMeshActor>();
	check(StaticMeshActor);

	StaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(Cast<UStaticMesh>(Settings->UnitTestHelperAssets[0].LoadSynchronous()));
	UAlterMeshGeometryActor* Param = Cast<UAlterMeshGeometryActor>(UAlterMeshLibrary::GetGeometryParam(CreatedActor.Get(), FString("Object")));
	check(Param);
	
	Param->Actor = StaticMeshActor;
	CreatedActor->RefreshSync();

	check(CreatedActor->AlterMeshComponents.Num() && CreatedActor->AlterMeshComponents[0]->Sections.Num());
	
	// Test without actor being transformed
	TestEqual(TEXT("NumSections"), CreatedActor->AlterMeshComponents[0]->Sections.Num(), 1);	
	TestEqual(TEXT("VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 4);
	TestEqual(TEXT("Section0 Vert0 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), FVector(0.0, -100.0, 100.0));
	TestEqual(TEXT("Section0 Vert1 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), FVector(100.0, 100.0, 100.0));
	TestEqual(TEXT("Section0 Vert2 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), FVector(100.0, 100.0, -100.0));
	TestEqual(TEXT("Section0 Vert3 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), FVector(0.0, -100.0, -100.0));

	FVector ExpectedNormal = FVector(0.894f,-0.447f,0.0);
	TestEqual(TEXT("Section0 Vert0 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[0]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert1 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[1]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert2 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[2]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert3 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[3]), ExpectedNormal, 0.001f);
	
	StaticMeshActor->AddActorLocalTransform(FTransform(FRotator(10, 20, 30), FVector(10, 20, 30), FVector(10,20,30)));
	CreatedActor->RefreshSync();

	// Test with actor being transformed
	check(CreatedActor->AlterMeshComponents.Num() && CreatedActor->AlterMeshComponents[0]->Sections.Num())
	
	TestEqual(TEXT("VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 4);
	const FVector ExpectedPos0 =  StaticMeshActor->GetActorTransform().TransformPosition(FVector(0.0, -100.0, 100.0));
	const FVector ExpectedPos1 =  StaticMeshActor->GetActorTransform().TransformPosition(FVector(100.0, 100.0, 100.0));
	const FVector ExpectedPos2 =  StaticMeshActor->GetActorTransform().TransformPosition(FVector(100.0, 100.0, -100.0)); 
	const FVector ExpectedPos3 =  StaticMeshActor->GetActorTransform().TransformPosition(FVector(0.0, -100.0, -100.0)); 
	TestEqual(TEXT("Section0 Vert0 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), ExpectedPos0, 0.001f);
	TestEqual(TEXT("Section0 Vert1 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), ExpectedPos1, 0.001f);
	TestEqual(TEXT("Section0 Vert2 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), ExpectedPos2, 0.001f);
	TestEqual(TEXT("Section0 Vert3 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), ExpectedPos3, 0.001f);

	// Calculate the inverse transpose of the matrix for correct normal transformation.
	FMatrix InverseTranspose = StaticMeshActor->GetActorTransform().ToMatrixWithScale().InverseFast().GetTransposed();
	FVector ExpectedNormalTransformed = InverseTranspose.TransformVector(FVector(0.894f, -0.447f, 0.0)).GetSafeNormal();
	
	TestEqual(TEXT("Section0 Vert0 normal after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[0]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert1 normal after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[1]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert2 normal after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[2]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert3 normal after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[3]), ExpectedNormalTransformed, 0.001f);
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshGeometryActorOriginal, "AlterMesh.ImportMesh.GeometryActorOriginal")

bool FAlterMeshGeometryActorOriginal::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(14);
	
	const UAlterMeshEditorSettings* Settings = GetDefault<UAlterMeshEditorSettings>();
	
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AStaticMeshActor* StaticMeshActor = CreatedWorld->SpawnActor<AStaticMeshActor>();
	check(StaticMeshActor);
	
	StaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(Cast<UStaticMesh>(Settings->UnitTestHelperAssets[0].LoadSynchronous()));
	UAlterMeshGeometryActor* Param = Cast<UAlterMeshGeometryActor>(UAlterMeshLibrary::GetGeometryParam(CreatedActor.Get(), FString("Object")));
	check(Param);
	
	Param->Actor = StaticMeshActor;
	CreatedActor->RefreshSync();
	check(CreatedActor->AlterMeshComponents.Num() && CreatedActor->AlterMeshComponents[0]->Sections.Num());
	
	// Test without actor being transformed
	TestEqual(TEXT("NumSections"), CreatedActor->AlterMeshComponents[0]->Sections.Num(), 1);	
	TestEqual(TEXT("VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 4);
	TestEqual(TEXT("Section0 Vert0 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), FVector(0.0, -100.0, 100.0));
	TestEqual(TEXT("Section0 Vert1 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), FVector(100.0, 100.0, 100.0));
	TestEqual(TEXT("Section0 Vert2 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), FVector(100.0, 100.0, -100.0));
	TestEqual(TEXT("Section0 Vert3 pos"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), FVector(0.0, -100.0, -100.0));

	FVector ExpectedNormal = FVector(0.894f,-0.447f,0.0);
	TestEqual(TEXT("Section0 Vert0 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[0]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert1 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[1]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert2 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[2]), ExpectedNormal, 0.001f);
	TestEqual(TEXT("Section0 Vert3 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[3]), ExpectedNormal, 0.001f);

	// Test with actor being transformed
	StaticMeshActor->AddActorLocalTransform(FTransform(FRotator(10, 20, 30), FVector(10, 20, 30), FVector(10,20,30)));
	CreatedActor->RefreshSync();
	check(CreatedActor->AlterMeshComponents.Num() && CreatedActor->AlterMeshComponents[0]->Sections.Num())
	
	TestEqual(TEXT("VertNum"), CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num(), 4);
	TestEqual(TEXT("Section0 Vert0 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[0]), FVector(0.0, -100.0, 100.0));
	TestEqual(TEXT("Section0 Vert1 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[1]), FVector(100.0, 100.0, 100.0));
	TestEqual(TEXT("Section0 Vert2 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[2]), FVector(100.0, 100.0, -100.0));
	TestEqual(TEXT("Section0 Vert3 pos after transformation"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices[3]), FVector(0.0, -100.0, -100.0));

	FVector ExpectedNormalTransformed = FVector(0.894f,-0.447f,0.0);
	TestEqual(TEXT("Section0 Vert0 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[0]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert1 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[1]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert2 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[2]), ExpectedNormalTransformed, 0.001f);
	TestEqual(TEXT("Section0 Vert3 normal"), FVector(CreatedActor->AlterMeshComponents[0]->Sections[0].Normals[3]), ExpectedNormalTransformed, 0.001f);
	
	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshGeometryLandscapeRelative, "AlterMesh.ImportMesh.GeometryLandscapeRelative")

bool FAlterMeshGeometryLandscapeRelative::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(19);
	
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ALandscape* LandscapeActor = CreatedWorld->SpawnActor<ALandscape>();
	LandscapeActor->SetLandscapeGuid(FGuid::NewGuid());
	
	int32 NumComponentsX = 1;
	int32 NumComponentsY = 1;
	int32 NumSectionsPerComponent = 1;
	int32 NumQuadsPerSection = 7;

	int32 SizeX = NumComponentsX * NumSectionsPerComponent * (NumQuadsPerSection + 1);
	int32 SizeY = NumComponentsY * NumSectionsPerComponent * (NumQuadsPerSection + 1);

	TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
	TArray<uint16> HeightData;
	HeightData.Init(0, SizeX * SizeY);

	HeightDataPerLayers.Add(FGuid(), HeightData);
	
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> Layers;
	Layers.Add(FGuid(), {});
	TArray<FLandscapeLayer> ImportLayers;
	TArrayView<const FLandscapeLayer> LayersView(ImportLayers);

	LandscapeActor->CreateLandscapeInfo();
	LandscapeActor->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1, NumSectionsPerComponent, NumQuadsPerSection, HeightDataPerLayers, nullptr, Layers, ELandscapeImportAlphamapType::Additive, LayersView);
	
	auto* LandscapeParam = Cast<UAlterMeshGeometryLandscape>(UAlterMeshLibrary::GetGeometryParam(CreatedActor.Get(), FString("Object")));
	LandscapeParam->MaxDistance = 0;
	CreatedActor->RefreshSync();
	
	TestEqual(TEXT("Landscape Sections"), CreatedActor->AlterMeshComponents[0]->Sections.Num(), 1);
	TestTrue(TEXT("Landscape Verts"), !!CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num());

	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_MESH_TEST(FAlterMeshGeometryLandscapeOriginal, "AlterMesh.ImportMesh.GeometryLandscapeOriginal")

bool FAlterMeshGeometryLandscapeOriginal::RunTest(const FString& Parameters)
{
	ImportMeshFromUnitTestAsset(18);
	
	FActorSpawnParameters Params;
	Params.bNoFail = true;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ALandscape* LandscapeActor = CreatedWorld->SpawnActor<ALandscape>();
	LandscapeActor->SetLandscapeGuid(FGuid::NewGuid());
	
	int32 NumComponentsX = 1;
	int32 NumComponentsY = 1;
	int32 NumSectionsPerComponent = 1;
	int32 NumQuadsPerSection = 7;

	int32 SizeX = NumComponentsX * NumSectionsPerComponent * (NumQuadsPerSection + 1);
	int32 SizeY = NumComponentsY * NumSectionsPerComponent * (NumQuadsPerSection + 1);

	TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
	TArray<uint16> HeightData;
	HeightData.Init(0, SizeX * SizeY);

	HeightDataPerLayers.Add(FGuid(), HeightData);
	
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> Layers;
	Layers.Add(FGuid(), {});
	TArray<FLandscapeLayer> ImportLayers;
	TArrayView<const FLandscapeLayer> LayersView(ImportLayers);

	LandscapeActor->CreateLandscapeInfo();
	LandscapeActor->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1, NumSectionsPerComponent, NumQuadsPerSection, HeightDataPerLayers, nullptr, Layers, ELandscapeImportAlphamapType::Additive, LayersView);
	
	auto* LandscapeParam = Cast<UAlterMeshGeometryLandscape>(UAlterMeshLibrary::GetGeometryParam(CreatedActor.Get(), FString("Object")));
	LandscapeParam->MaxDistance = 0;
	CreatedActor->RefreshSync();
	
	TestEqual(TEXT("Landscape Sections"), CreatedActor->AlterMeshComponents[0]->Sections.Num(), 1);
	TestTrue(TEXT("Landscape Verts"), !!CreatedActor->AlterMeshComponents[0]->Sections[0].Vertices.Num());

	CleanUpWorld();
	return !HasAnyErrors();
}

IMPLEMENT_ALTERMESH_IMPORT_ASSET_TEST(FAlterMeshImportAssetParamTypes, "AlterMesh.ImportAsset.ParamTypes")

bool FAlterMeshImportAssetParamTypes::RunTest(const FString& Parameters)
{
	//return true;
	// todo implement through FEditorImportExportTestDefinition
	
	int32 NumParamTypes = 0;
	for (TObjectIterator<UScriptStruct> StructIterator; StructIterator; ++StructIterator)
	{
		if (StructIterator->IsChildOf(FAlterMeshParamBase::StaticStruct())
			&& *StructIterator != FAlterMeshParamBase::StaticStruct())
		{
			NumParamTypes++;
		}
	}
 
	TestEqual(TEXT("Num existing Params (Update Test File)"), CreatedAsset->GetParams().Params.Num(), NumParamTypes);
	
	int32 NumTestedParams = 0;
	for (FAlterMeshParamBase* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshParamBase>())
	{
		TestTrue(TEXT("Param name imported correctly"), !Param->BaseData.Name.IsEmpty());
		TestTrue(TEXT("Param inputname imported correctly"), Param->BaseData.Id.Input.IsValid());
		TestTrue(TEXT("Param panel imported correctly"), !Param->BaseData.Panel.IsEmpty());
		TestTrue(TEXT("Param tooltip imported correctly"), !Param->BaseData.Tooltip.IsEmpty());
	}
	
	for (auto* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshBoolParam>())
	{
		TestEqual(TEXT("BoolParam Value"), Param->Data.Value, true);
		TestEqual(TEXT("BoolParam Default"), Param->Data.DefaultValue, true);
		NumTestedParams++;
	}
	
	for (auto* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshFloatParam>())
	{
		TestEqual(TEXT("FloatParam Value"), Param->Data.Value, 2.0f);
		TestEqual(TEXT("FloatParam Default"), Param->Data.DefaultValue, 5.0f);
		TestEqual(TEXT("FloatParam MinValue"), Param->Data.MinValue, 1.0f);
		TestEqual(TEXT("FloatParam MaxValue"), Param->Data.MaxValue, 10.0f);
		NumTestedParams++;
	}
	
	for (auto* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshIntParam>())
	{
		TestEqual(TEXT("IntParam Value"), Param->Data.Value, 2);
		TestEqual(TEXT("IntParam Default"), Param->Data.DefaultValue, 5);
		TestEqual(TEXT("IntParam MinValue"), Param->Data.MinValue, 1);
		TestEqual(TEXT("IntParam MaxValue"), Param->Data.MaxValue, 10);
		NumTestedParams++;
	}
	
	for (auto* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshStringParam>())
	{
		TestEqual(TEXT("StringParam Value"), Param->Data.Value, FString("Abc"));
		TestEqual(TEXT("StringParam Default"), Param->Data.DefaultValue, FString("Xyz"));
		NumTestedParams++;
	}
	
	for (auto* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshGeometryParam>())
	{
		// Already tested individually
		NumTestedParams++;
	}
	
	for (auto* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshTextureParam>())
	{
		// Cannot have imported values
		NumTestedParams++;
	}
	
	for (auto* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshCollectionParam>())
	{
		// Already tested individually
		NumTestedParams++;
	}
	
	for (auto* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshColorParam>())
	{
		TestEqual(TEXT("ColorParam Value"), Param->Data.Value, FColor(0, 255,255));
		TestEqual(TEXT("ColorParam Default"), Param->Data.DefaultValue, FColor(0,255,255));
		NumTestedParams++;
	}
	
	for (auto* Param : CreatedAsset->GetParams().GetTyped<FAlterMeshVectorParam>())
	{
		TestEqual(TEXT("VectorParam Value"), Param->Data.Value, FVector::OneVector);
		TestEqual(TEXT("VectorParam Default"), Param->Data.DefaultValue, FVector::OneVector);
		NumTestedParams++;
	}

	TestEqual(TEXT("Num tested Params"), NumTestedParams, NumParamTypes);	
	
	return !HasAnyErrors();
}