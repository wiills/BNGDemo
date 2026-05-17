// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "Factories/Factory.h"
#include "ExAssetTypeActions_FlowGraph.generated.h"

class FExAssetTypeActions_FlowGraph : public FAssetTypeActions_Base
{
public:
	FExAssetTypeActions_FlowGraph(EAssetTypeCategories::Type InCategory = EAssetTypeCategories::Blueprint);

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;
	virtual uint32 GetCategories() override;
	virtual const TArray<FText>& GetSubMenus() const override;

protected:
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;

private:
	EAssetTypeCategories::Type Category;
};

UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UFactory_ExFlowGraph : public UFactory
{
	GENERATED_BODY()

public:
	UFactory_ExFlowGraph(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;
	virtual FName GetNewAssetThumbnailOverride() const override;
	virtual bool ShouldShowInNewMenu() const override;
};
