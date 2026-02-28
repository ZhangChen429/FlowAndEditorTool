// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IPropertyHandle;

/**
 * Details panel customization for UWorkspotTree
 * Provides custom UI for editing WorkspotTree properties
 */
class FWorkspotTreeDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void AddEntryActions(IDetailLayoutBuilder& DetailBuilder);
	void AddValidationSection(IDetailLayoutBuilder& DetailBuilder);

	FReply OnRegenerateEntryIds();
	FReply OnValidateTree();

	TWeakObjectPtr<class UWorkspotTree> CachedWorkspotTree;
};
