// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestTreeWidget.h"
#include "Quest/ExQuestManagerSubsystem.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"

void UExQuestTreeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UExQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UExQuestManagerSubsystem>())
		{
			QuestManager->OnQuestStateChanged.AddDynamic(this, &UExQuestTreeWidget::HandleQuestStateChanged);
			QuestManager->OnQuestProgressChanged.AddDynamic(this, &UExQuestTreeWidget::HandleQuestProgressChanged);
		}
	}

	RefreshQuestTree();
}

void UExQuestTreeWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UExQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UExQuestManagerSubsystem>())
		{
			QuestManager->OnQuestStateChanged.RemoveDynamic(this, &UExQuestTreeWidget::HandleQuestStateChanged);
			QuestManager->OnQuestProgressChanged.RemoveDynamic(this, &UExQuestTreeWidget::HandleQuestProgressChanged);
		}
	}

	Super::NativeDestruct();
}

void UExQuestTreeWidget::HandleQuestStateChanged(const FExQuestTask& QuestTask)
{
	RefreshQuestTree();
}

void UExQuestTreeWidget::HandleQuestProgressChanged(const FGameplayTag& TaskId, float CompletionPercent)
{
	RefreshQuestTree();
}

void UExQuestTreeWidget::RefreshQuestTree()
{
	if (RootQuestContainer)
	{
		RootQuestContainer->ClearChildren();
	}

	BuildQuestTree();
}

void UExQuestTreeWidget::SetQuestData(const FExQuestData& QuestData)
{
	DisplayedQuestData = QuestData;
	RefreshQuestTree();
}

void UExQuestTreeWidget::BuildQuestTree()
{
	if (!RootQuestContainer)
	{
		return;
	}

	TArray<FExQuestTask> RootTasks = DisplayedQuestData.GetRootTasks();
	for (const FExQuestTask& RootTask : RootTasks)
	{
		CreateQuestItem(RootTask, RootQuestContainer, 0);
	}
}

void UExQuestTreeWidget::CreateQuestItem(const FExQuestTask& QuestTask, UVerticalBox* ParentContainer, int32 Depth)
{
	if (!ParentContainer)
	{
		return;
	}

	UTextBlock* TaskNameText = NewObject<UTextBlock>(ParentContainer);
	if (TaskNameText)
	{
		FText TaskDisplayText = FText::Format(NSLOCTEXT("QuestUI", "TaskNameFormat", "{0} ({1})"),
			QuestTask.TaskName, GetStateText(QuestTask.State));
		TaskNameText->SetText(TaskDisplayText);
		TaskNameText->SetColorAndOpacity(GetStateColor(QuestTask.State));

		const FMargin IndentMargin(Depth * 20.0f, 2.0f, 0.0f, 2.0f);
		if (UVerticalBoxSlot* NameSlot = ParentContainer->AddChildToVerticalBox(TaskNameText))
		{
			NameSlot->SetPadding(IndentMargin);
		}
	}

	if (QuestTask.State == EExQuestState::Active || IsQuestExpanded(QuestTask.TaskId.ToString()))
	{
		for (const FExQuestObjective& Objective : QuestTask.Objectives)
		{
			UTextBlock* ObjectiveText = NewObject<UTextBlock>(ParentContainer);
			if (ObjectiveText)
			{
				FText ObjectiveDisplayText = FText::Format(NSLOCTEXT("QuestUI", "ObjectiveFormat", "  - {0} ({1}/{2})"),
					Objective.Description,
					FText::AsNumber(Objective.CurrentProgress),
					FText::AsNumber(Objective.TargetProgress));
				ObjectiveText->SetText(ObjectiveDisplayText);

				const FMargin ObjectiveIndent((Depth + 1) * 20.0f, 2.0f, 0.0f, 2.0f);
				if (UVerticalBoxSlot* ObjSlot = ParentContainer->AddChildToVerticalBox(ObjectiveText))
				{
					ObjSlot->SetPadding(ObjectiveIndent);
				}

				if (Objective.bIsCompleted)
				{
					ObjectiveText->SetColorAndOpacity(FLinearColor::Green);
				}
			}
		}

		TArray<FExQuestTask> SubTasks = DisplayedQuestData.GetSubTasks(QuestTask.TaskId);
		for (const FExQuestTask& SubTask : SubTasks)
		{
			CreateQuestItem(SubTask, ParentContainer, Depth + 1);
		}
	}
}

FSlateColor UExQuestTreeWidget::GetStateColor(EExQuestState State) const
{
	switch (State)
	{
	case EExQuestState::Active:
		return FLinearColor::Yellow;
	case EExQuestState::Completed:
		return FLinearColor::Green;
	case EExQuestState::Failed:
		return FLinearColor::Red;
	case EExQuestState::Locked:
		return FLinearColor::Gray;
	default:
		return FLinearColor::White;
	}
}

FText UExQuestTreeWidget::GetStateText(EExQuestState State) const
{
	switch (State)
	{
	case EExQuestState::Inactive:
		return NSLOCTEXT("QuestUI", "StateInactive", "未激活");
	case EExQuestState::Active:
		return NSLOCTEXT("QuestUI", "StateActive", "进行中");
	case EExQuestState::Completed:
		return NSLOCTEXT("QuestUI", "StateCompleted", "已完成");
	case EExQuestState::Failed:
		return NSLOCTEXT("QuestUI", "StateFailed", "失败");
	case EExQuestState::Locked:
		return NSLOCTEXT("QuestUI", "StateLocked", "已锁定");
	default:
		return NSLOCTEXT("QuestUI", "StateUnknown", "未知");
	}
}

void UExQuestTreeWidget::ToggleQuestExpansion(const FString& TaskId)
{
	if (ExpandedTaskIds.Contains(TaskId))
	{
		ExpandedTaskIds.Remove(TaskId);
	}
	else
	{
		ExpandedTaskIds.Add(TaskId);
	}
	RefreshQuestTree();
}

bool UExQuestTreeWidget::IsQuestExpanded(const FString& TaskId) const
{
	return ExpandedTaskIds.Contains(TaskId);
}
