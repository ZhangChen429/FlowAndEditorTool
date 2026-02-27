// Fill out your copyright notice in the Description page of Project Settings.

#include "Graph/SFlowGraphNode_FactNode.h"

#include "SGraphPanel.h"
#include "Graph/FlowGraphNode_FactNode.h"


void SFlowGraphNode_FactNode::Construct(const FArguments& InArgs, UFlowGraphNode* InNode)
{
	// Call parent construct
	SFlowGraphNode::Construct(SFlowGraphNode::FArguments(), Cast<UFlowGraphNode>(InNode));
}

void SFlowGraphNode_FactNode::UpdateGraphNode()
{
	// 清除现有内容
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	// 六边形节点布局
	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SOverlay)

			// 主体内容 - 使用固定尺寸确保节点大小稳定
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SBox)
				.WidthOverride(140.0f)    // 使用固定宽度
				.HeightOverride(140.0f)   // 使用固定高度
				[
					SNew(SVerticalBox)

					// 中心内容区域
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.FillHeight(1.0f)
					[
						SNew(SVerticalBox)

						// 标题
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(5.0f, 10.0f, 5.0f, 5.0f)
						[
							SNew(STextBlock)
							.Text(this, &SFlowGraphNode_FactNode::GetTitle)
							.TextStyle(FAppStyle::Get(), "Graph.StateNode.NodeTitle")
							.Justification(ETextJustify::Center)
						]

						// 内容区域
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(5.0f)
						[
							CreateNodeContentArea()
						]
					]
				]
			]

			// 左侧引脚（输入）
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(-10.0f, 0.0f, 0.0f, 0.0f))
			[
				SAssignNew(LeftNodeBox, SVerticalBox)
			]

			// 右侧引脚（输出）
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(FMargin(0.0f, 0.0f, -10.0f, 0.0f))
			[
				SAssignNew(RightNodeBox, SVerticalBox)
			]
		];

	// 创建引脚
	CreatePinWidgets();
}

const FSlateBrush* SFlowGraphNode_FactNode::GetNodeBodyBrush() const
{

    return FAppStyle::GetBrush("Graph.StateNode.Body");

}

TSharedRef<SWidget> SFlowGraphNode_FactNode::CreateNodeContentArea()
{
    return SFlowGraphNode::CreateNodeContentArea();
}

FSlateColor SFlowGraphNode_FactNode::GetBorderBackgroundColor() const
{
	// 自定义节点背景颜色 (深紫色)
	return FLinearColor(0.1f, 0.05f, 0.15f, 1.0f);
	
}

FText SFlowGraphNode_FactNode::GetTitle() const
{
	// 可以自定义标题显示
	return SFlowGraphNode::GetTitle();
}

const FSlateBrush* SFlowGraphNode_FactNode::GetNameIcon() const
{
	// 返回节点图标
	return SFlowGraphNode::GetNameIcon();
}

FVector2D SFlowGraphNode_FactNode::ComputeDesiredSize(float) const
{
	// 返回固定的节点尺寸
	return FVector2D(140.0f, 140.0f);
}

TArray<FVector2D> SFlowGraphNode_FactNode::GenerateHexagonPoints(const FVector2D& Center, float Radius)
{
	TArray<FVector2D> Points;
	Points.Reserve(6);

	// 生成正六边形的6个顶点（从顶部开始，顺时针）
	for (int32 i = 0; i < 6; i++)
	{
		// 从-90度开始（顶部），每60度一个顶点
		float Angle = PI / 180.0f * (60.0f * i - 90.0f);
		Points.Add(Center + FVector2D(
			FMath::Cos(Angle) * Radius,
			FMath::Sin(Angle) * Radius
		));
	}

	return Points;
}

int32 SFlowGraphNode_FactNode::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 先调用父类绘制，这会完成布局并绘制引脚、文本等元素
	int32 MaxLayerId = SFlowGraphNode::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// 现在获取节点大小（父类已经完成布局）
	FVector2D NodeSize = AllottedGeometry.GetLocalSize();

	// 添加安全检查
	if (NodeSize.X <= 0.0f || NodeSize.Y <= 0.0f)
	{
		// 使用固定尺寸作为后备
		NodeSize = FVector2D(140.0f, 140.0f);
	}

	const FVector2D Center = NodeSize * 0.5f;

	// 六边形半径（基于节点较小的维度）
	const float Radius = FMath::Min(NodeSize.X, NodeSize.Y) * 0.45f;

	// 生成六边形顶点
	TArray<FVector2D> HexPoints = GenerateHexagonPoints(Center, Radius);

	// 在最底层绘制六边形填充（在所有内容下方）
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		HexPoints,
		ESlateDrawEffect::None,
		GetBorderBackgroundColor().GetSpecifiedColor(),
		true,  // bAntialias
		0.0f   // Thickness - 0 means filled polygon
	);

	// 绘制六边形边框（在填充上方）
	TArray<FVector2D> BorderPoints = HexPoints;
	BorderPoints.Add(HexPoints[0]); // 闭合路径

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		MaxLayerId + 1,
		AllottedGeometry.ToPaintGeometry(),
		BorderPoints,
		ESlateDrawEffect::None,
		FLinearColor(0.2f, 0.15f, 0.25f, 1.0f), // 边框颜色
		true,  // bAntialias
		3.0f   // 边框粗细
	);

	// 如果节点被选中，绘制高亮边框（在最上层）
	if (GetOwnerPanel().IsValid() && GetOwnerPanel()->SelectionManager.SelectedNodes.Contains(GraphNode))
	{
		TArray<FVector2D> SelectionPoints = GenerateHexagonPoints(Center, Radius + 5.0f);
		if (!SelectionPoints.Emplace())
		{
			SelectionPoints.Add(SelectionPoints[0]);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				MaxLayerId + 2,
				AllottedGeometry.ToPaintGeometry(),
				SelectionPoints,
				ESlateDrawEffect::None,
				FLinearColor(1.0f, 0.5f, 0.0f, 1.0f), // 橙色选中高亮
				true,
				4.0f
			);

			return MaxLayerId + 3;
		}
		
	}

	return MaxLayerId + 2;
}
