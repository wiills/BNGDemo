// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * Flow graph ËµÑÊ∫êÁö?Asset Type Actions„Ä?
 * FAssetTypeActions_Base Èù?UObjectÔºåÁ¶ÅÊ≠¢‰ΩøÁî?UCLASS / GENERATED_BODY„Ä?
 */
class FExAssetTypeActions_Graph : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
	virtual uint32 GetCategories() override;
	
protected:
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
};
