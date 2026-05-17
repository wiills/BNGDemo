// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExK2Node_GameplayTag.h"
#include "BlueprintTool/ExGameplayTagProxy.h"

#define LOCTEXT_NAMESPACE "K2Node_GameplayTag"

UExK2Node_GameplayTagListener::UExK2Node_GameplayTagListener(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExGameplayTagListenerProxy, CreateProxy_ListenTag);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExGameplayTagListenerProxy, Activate);
	ProxyFactoryClass = UExGameplayTagListenerProxy::StaticClass();
	ProxyClass = UExGameplayTagListenerProxy::StaticClass();
}

void UExK2Node_GameplayTagListener::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
}

FText UExK2Node_GameplayTagListener::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "GameplayTagListener_Tooltip", "Listen for Gameplay Tag changes");
}

FText UExK2Node_GameplayTagListener::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "GameplayTagListener_Title", "Listen Gameplay Tag");
}

FSlateIcon UExK2Node_GameplayTagListener::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FColor(100, 200, 100);
	static FSlateIcon Icon("GameplayTagsEditor", "GameplayTags.TabIcon");
	return Icon;
}

UExK2Node_GameplayTagQuery::UExK2Node_GameplayTagQuery(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExGameplayTagQueryProxy, CreateProxy_QueryTags);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExGameplayTagQueryProxy, Activate);
	ProxyFactoryClass = UExGameplayTagQueryProxy::StaticClass();
	ProxyClass = UExGameplayTagQueryProxy::StaticClass();
}

void UExK2Node_GameplayTagQuery::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
}

FText UExK2Node_GameplayTagQuery::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "GameplayTagQuery_Tooltip", "Query and iterate Gameplay Tags");
}

FText UExK2Node_GameplayTagQuery::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "GameplayTagQuery_Title", "Query Gameplay Tags");
}

FSlateIcon UExK2Node_GameplayTagQuery::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FColor(100, 200, 100);
	static FSlateIcon Icon("GameplayTagsEditor", "GameplayTags.TabIcon");
	return Icon;
}

UExK2Node_GameplayTagModifier::UExK2Node_GameplayTagModifier(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExGameplayTagModifierProxy, CreateProxy_AddTags);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExGameplayTagModifierProxy, Activate);
	ProxyFactoryClass = UExGameplayTagModifierProxy::StaticClass();
	ProxyClass = UExGameplayTagModifierProxy::StaticClass();
}

void UExK2Node_GameplayTagModifier::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
}

FText UExK2Node_GameplayTagModifier::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "GameplayTagModifier_Tooltip", "Add or Remove Gameplay Tags");
}

FText UExK2Node_GameplayTagModifier::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "GameplayTagModifier_Title", "Modify Gameplay Tags");
}

FSlateIcon UExK2Node_GameplayTagModifier::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FColor(100, 200, 100);
	static FSlateIcon Icon("GameplayTagsEditor", "GameplayTags.TabIcon");
	return Icon;
}

#undef LOCTEXT_NAMESPACE
