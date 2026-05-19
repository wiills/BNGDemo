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

/** UButton::OnClicked 为动态委托，需通过 UFUNCTION 转发点击 */
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

/**
 * @class UExQuestTreeWidget
 * @brief 任务树 UI 控件
 *
 * 显示层级任务系统的 UI 组件，支持展开/收起任务，显示任务进度等
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExQuestTreeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** 刷新任务树显示 */
	UFUNCTION(BlueprintCallable, Category = "Quest UI")
	void RefreshQuestTree();

	/** 设置要显示的任务数据 */
	UFUNCTION(BlueprintCallable, Category = "Quest UI")
	void SetQuestData(const FExQuestData& QuestData);

protected:
	/** 任务状态变化时刷新（与 FOnQuestStateChanged 签名一致，供 AddDynamic 绑定） */
	UFUNCTION()
	void HandleQuestStateChanged(const FExQuestTask& QuestTask);

	/** 任务进度变化时刷新（与 FOnQuestProgressChanged 签名一致，供 AddDynamic 绑定） */
	UFUNCTION()
	void HandleQuestProgressChanged(const FGameplayTag& TaskId, float CompletionPercent);

	UFUNCTION()
	void HandleQuestObjectiveUpdated(const FExQuestObjective& Objective);

	UFUNCTION()
	void HandleQuestDataLoaded();

	/** 根任务容器 */
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RootQuestContainer;

	/** 滚动框 */
	UPROPERTY(meta = (BindWidget))
	UScrollBox* QuestScrollBox;

	/** 标题文本 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;

	/** 绑定到 QuestManager 后，刷新时自动从 Subsystem 同步数据 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest UI")
	bool bAutoSyncFromManager = true;

	/** 当前显示的任务数据（本地缓存，Refresh 时可能被 Manager 覆盖） */
	UPROPERTY()
	FExQuestData DisplayedQuestData;

	/** 展开的任务ID集合 */
	UPROPERTY()
	TSet<FString> ExpandedTaskIds;

	/** 保持展开按钮点击处理器存活（Refresh 时重建） */
	UPROPERTY()
	TArray<TObjectPtr<UExQuestExpandClickHandler>> ExpandClickHandlers;

	/**
	 * 从 QuestManager Subsystem 同步 DisplayedQuestData
	 */
	void SyncDisplayedDataFromManager();

	/**
	 * 构建任务树
	 */
	void BuildQuestTree();

	/**
	 * 为单个任务创建UI元素
	 * @param QuestTask 任务数据
	 * @param ParentContainer 父容器
	 * @param Depth 层级深度（用于缩进）
	 */
	void CreateQuestItem(const FExQuestTask& QuestTask, UVerticalBox* ParentContainer, int32 Depth = 0);

	/**
	 * 获取任务状态颜色
	 */
	FSlateColor GetStateColor(EExQuestState State) const;

	/**
	 * 获取任务状态文本
	 */
	FText GetStateText(EExQuestState State) const;

	/** 切换任务展开状态（供 UExQuestExpandClickHandler 调用） */
	UFUNCTION()
	void ToggleQuestExpansion(const FString& TaskId);

	friend class UExQuestExpandClickHandler;

	/**
	 * 检查任务是否展开
	 */
	bool IsQuestExpanded(const FString& TaskId) const;
};
