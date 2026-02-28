// Workspot调试指南
// 按执行顺序检查每个环节

//=============================================================================
// 1. SmartObject集成检查
//=============================================================================

// 检查点1.1: WorkspotBehaviorDefinition配置
// 位置: SmartObject Definition asset
✓ WorkspotTree是否已赋值？
✓ PreferredEntryPoint是否正确？
✓ BehaviorClass是否返回UGameplayBehavior_Workspot？

// 调试代码：在WorkspotBehaviorDefinition.cpp添加日志
TSubclassOf<UGameplayBehavior> UWorkspotBehaviorDefinition::GetBehaviorClass() const
{
    UE_LOG(LogWorkspot, Warning, TEXT("GetBehaviorClass called, returning UGameplayBehavior_Workspot"));
    return UGameplayBehavior_Workspot::StaticClass();
}

//=============================================================================
// 2. GameplayBehavior触发检查
//=============================================================================

// 检查点2.1: Trigger是否被调用
// 位置: WorkspotGameplayBehavior.cpp:19
bool UGameplayBehavior_Workspot::Trigger(...)
{
    UE_LOG(LogWorkspot, Error, TEXT("🔥 TRIGGER CALLED on Avatar: %s"), *Avatar.GetName());

    // 检查Definition
    const UWorkspotBehaviorDefinition* WorkspotDefinition = Cast<UWorkspotBehaviorDefinition>(Config);
    if (!WorkspotDefinition)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ Config is NOT WorkspotBehaviorDefinition!"));
        return false;
    }

    UE_LOG(LogWorkspot, Error, TEXT("✓ WorkspotDefinition valid"));

    // 检查WorkspotTree
    if (!WorkspotDefinition->WorkspotTree)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ WorkspotTree is NULL!"));
        return false;
    }

    UE_LOG(LogWorkspot, Error, TEXT("✓ WorkspotTree: %s"), *WorkspotDefinition->WorkspotTree->GetName());
    ...
}

//=============================================================================
// 3. WorkspotSubsystem检查
//=============================================================================

// 检查点3.1: Subsystem是否初始化
// 添加到WorkspotSubsystem.cpp:15
void UWorkspotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogWorkspot, Error, TEXT("🌍 WorkspotSubsystem::Initialize - Subsystem is READY"));
}

// 检查点3.2: StartWorkspot是否被调用
// 添加到WorkspotSubsystem.cpp:47
UWorkspotInstance* UWorkspotSubsystem::StartWorkspot(AActor* Actor, UWorkspotTree* WorkspotTree, FName EntryPointTag)
{
    UE_LOG(LogWorkspot, Error, TEXT("🎬 StartWorkspot called - Actor: %s, Tree: %s"),
        *Actor->GetName(), *WorkspotTree->GetName());

    if (!Actor)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ Actor is NULL!"));
        return nullptr;
    }

    if (!WorkspotTree)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ WorkspotTree is NULL!"));
        return nullptr;
    }

    // 检查WorkspotTree是否有效
    if (!WorkspotTree->IsValid())
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ WorkspotTree is INVALID! RootEntry: %s, Skeleton: %s"),
            WorkspotTree->RootEntry ? TEXT("OK") : TEXT("NULL"),
            WorkspotTree->WorkspotSkeleton ? TEXT("OK") : TEXT("NULL"));
        return nullptr;
    }

    UE_LOG(LogWorkspot, Error, TEXT("✓ WorkspotTree is valid"));
    ...
}

//=============================================================================
// 4. WorkspotInstance创建检查
//=============================================================================

// 检查点4.1: Instance Setup
// 添加到WorkspotInstance.cpp:9
bool UWorkspotInstance::Setup(AActor* InActor, UWorkspotTree* InTree, FName EntryPointTag)
{
    UE_LOG(LogWorkspot, Error, TEXT("🎭 WorkspotInstance::Setup - Actor: %s, Tree: %s"),
        *InActor->GetName(), *InTree->GetName());

    // 检查SkeletalMeshComponent
    USkeletalMeshComponent* MeshComp = GetSkeletalMeshComponent();
    if (!MeshComp)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ No SkeletalMeshComponent on actor!"));
        return false;
    }

    UE_LOG(LogWorkspot, Error, TEXT("✓ SkeletalMeshComponent: %s"), *MeshComp->GetName());

    // 检查AnimInstance
    UAnimInstance* AnimInst = GetAnimInstance();
    if (!AnimInst)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ No AnimInstance on SkeletalMeshComponent!"));
        return false;
    }

    UE_LOG(LogWorkspot, Error, TEXT("✓ AnimInstance: %s"), *AnimInst->GetClass()->GetName());

    // 检查RootEntry
    if (!InTree->RootEntry)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ WorkspotTree has no RootEntry!"));
        return false;
    }

    UE_LOG(LogWorkspot, Error, TEXT("✓ RootEntry: %s"), *InTree->RootEntry->GetClass()->GetName());
    ...
}

//=============================================================================
// 5. Iterator创建检查
//=============================================================================

// 检查点5.1: Iterator创建
// 在WorkspotInstance.cpp:Setup中添加
Iterator = InTree->RootEntry->CreateIterator(Context);
if (!Iterator.IsValid())
{
    UE_LOG(LogWorkspot, Error, TEXT("❌ Failed to create Iterator!"));
    State = EWorkspotState::Inactive;
    return false;
}

UE_LOG(LogWorkspot, Error, TEXT("✓ Iterator created successfully"));

// 检查点5.2: 第一个Entry
if (Iterator->Next(Context))
{
    UE_LOG(LogWorkspot, Error, TEXT("✓ Iterator->Next() returned TRUE"));

    if (Iterator->GetData(CurrentEntryData))
    {
        UE_LOG(LogWorkspot, Error, TEXT("✓ GetData succeeded - AnimMontage: %s, IdleAnim: %s"),
            CurrentEntryData.AnimMontage ? *CurrentEntryData.AnimMontage->GetName() : TEXT("NULL"),
            *CurrentEntryData.IdleAnim.ToString());

        if (!CurrentEntryData.AnimMontage)
        {
            UE_LOG(LogWorkspot, Error, TEXT("❌ AnimMontage is NULL!"));
            return false;
        }
        ...
    }
    else
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ Iterator->GetData() returned FALSE"));
        return false;
    }
}
else
{
    UE_LOG(LogWorkspot, Error, TEXT("❌ Iterator->Next() returned FALSE - No entries to play!"));
    return false;
}

//=============================================================================
// 6. 动画播放检查
//=============================================================================

// 检查点6.1: PlayCurrentAnimation
// 在WorkspotInstance.cpp:213添加
void UWorkspotInstance::PlayCurrentAnimation()
{
    UAnimInstance* AnimInst = GetAnimInstance();
    if (!AnimInst)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ PlayCurrentAnimation - No AnimInstance!"));
        return;
    }

    if (!CurrentEntryData.AnimMontage)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ PlayCurrentAnimation - No AnimMontage!"));
        return;
    }

    UE_LOG(LogWorkspot, Error, TEXT("🎵 Playing animation: %s"), *CurrentEntryData.AnimMontage->GetName());

    // 检查Skeleton匹配
    if (AnimInst->CurrentSkeleton != CurrentEntryData.AnimMontage->GetSkeleton())
    {
        UE_LOG(LogWorkspot, Error, TEXT("⚠️ Skeleton mismatch! AnimInstance: %s, Montage: %s"),
            AnimInst->CurrentSkeleton ? *AnimInst->CurrentSkeleton->GetName() : TEXT("NULL"),
            CurrentEntryData.AnimMontage->GetSkeleton() ? *CurrentEntryData.AnimMontage->GetSkeleton()->GetName() : TEXT("NULL"));
    }

    // Play montage
    float PlayLength = AnimInst->Montage_Play(
        CurrentEntryData.AnimMontage,
        1.0f,
        EMontagePlayReturnType::MontageLength,
        0.0f,
        true
    );

    if (PlayLength <= 0.0f)
    {
        UE_LOG(LogWorkspot, Error, TEXT("❌ Montage_Play returned invalid length: %.2f"), PlayLength);
    }
    else
    {
        UE_LOG(LogWorkspot, Error, TEXT("✅ Animation playing! Length: %.2f seconds"), PlayLength);
    }
    ...
}

//=============================================================================
// 7. Tick检查
//=============================================================================

// 检查点7.1: Instance是否被Tick
// 在WorkspotSubsystem.cpp:29添加
void UWorkspotSubsystem::Tick(float DeltaTime)
{
    static int TickCount = 0;
    if (++TickCount % 60 == 0) // 每60帧打印一次
    {
        UE_LOG(LogWorkspot, Warning, TEXT("🔄 Subsystem Tick - Active instances: %d"), ActiveInstances.Num());
    }

    // Tick all active instances
    for (auto& Pair : ActiveInstances)
    {
        if (UWorkspotInstance* Instance = Pair.Value)
        {
            Instance->Tick(DeltaTime);
        }
    }
    ...
}

// 检查点7.2: Instance Tick
// 在WorkspotInstance.cpp:76添加
void UWorkspotInstance::Tick(float DeltaTime)
{
    static int TickCount = 0;
    if (++TickCount % 60 == 0)
    {
        UE_LOG(LogWorkspot, Warning, TEXT("🎭 Instance Tick - State: %d, IsFinished: %s"),
            (int)State, IsCurrentAnimationFinished() ? TEXT("YES") : TEXT("NO"));
    }
    ...
}
