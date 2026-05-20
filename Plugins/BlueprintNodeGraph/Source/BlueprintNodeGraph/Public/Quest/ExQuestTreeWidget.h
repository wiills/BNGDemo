// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ExQuestTypes.h"
#include "ExQuestTreeWidget.generated.h"

class UVerticalBox;
class UTextBlock;
class UScrollBox;
class UExQuestTreeWidget;

/** Forwards expand-button clicks (dynamic delegate requires UFUNCTION) */
UCLASS()
class UExQuestExpandClickHandler : public UObject
{
	GENERATED_BODY()

public:
	void Setup(UExQuestTreeWidget* InOwner, const FString& InTaskId);

	UFUNCTION()
	void OnClicked();

private:
	UPROPERTY()
	TWeakObjectPtr<UExQuestTreeWidget> OwnerWidget;

	FString TaskIdStr;
};

/** Hierarchical quest list UI */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExQuestTreeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Quest UI")
	void RefreshQuestTree();

	UFUNCTION(BlueprintCallable, Category = "Quest UI")
	void SetQuestData(const FExQuestData& QuestData);

protected:
	UFUNCTION()
	void HandleQuestStateChanged(const FExQuestTask& QuestTask);

	UFUNCTION()
	void HandleQuestProgressChanged(const FGameplayTag& TaskId, float CompletionPercent);

	UFUNCTION()
	void HandleQuestObjectiveUpdated(const FExQuestObjective& Objective);

	UFUNCTION()
	void HandleQuestDataLoaded();

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RootQuestContainer;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* QuestScrollBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest UI")
	bool bAutoSyncFromManager = true;

	/** Active tasks auto-expand; Completed tasks collapse when bAutoCollapseOnComplete is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest UI")
	bool bAutoExpandActiveQuests = true;

	/** Remove completed tasks from manual expansion so Standalone and Listen Server match Client. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest UI")
	bool bAutoCollapseOnComplete = true;

	UPROPERTY()
	FExQuestData DisplayedQuestData;

	UPROPERTY()
	TSet<FString> ExpandedTaskIds;

	/** Last-known task states; used to detect Active?Completed on Client replication path. */
	UPROPERTY()
	TMap<FString, EExQuestState> PreviousTaskStates;

	UPROPERTY()
	TArray<TObjectPtr<UExQuestExpandClickHandler>> ExpandClickHandlers;

	void SyncDisplayedDataFromManager();
	void SyncExpansionStateFromQuestData();
	void BuildQuestTree();
	void CreateQuestItem(const FExQuestTask& QuestTask, UVerticalBox* ParentContainer, int32 Depth = 0);

	FSlateColor GetStateColor(EExQuestState State) const;
	FText GetStateText(EExQuestState State) const;

	UFUNCTION()
	void ToggleQuestExpansion(const FString& TaskId);

	friend class UExQuestExpandClickHandler;

	bool IsQuestExpanded(const FString& TaskId) const;
};
