// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestTreeWidget.h"
#include "Quest/ExQuestManagerSubsystem.h"
#include "Quest/ExQuestBlueprintLibrary.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/ExQuestDefinition.h"

namespace ExQuestTreeWidgetInternal
{
	static FText GetObjectiveDisplayText(const FExQuestObjective& Objective)
	{
		if (!Objective.Description.IsEmpty())
		{
			return Objective.Description;
		}

		if (Objective.ObjectiveTag.IsValid())
		{
			return FText::FromName(Objective.ObjectiveTag.GetTagName());
		}

		return FText::GetEmpty();
	}
}

void UExQuestExpandClickHandler::Setup(UExQuestTreeWidget* InOwner, const FString& InTaskId)
{
	OwnerWidget = InOwner;
	TaskIdStr = InTaskId;
}

void UExQuestExpandClickHandler::OnClicked()
{
	if (UExQuestTreeWidget* Widget = OwnerWidget.Get())
	{
		Widget->ToggleQuestExpansion(TaskIdStr);
	}
}

void UExQuestTreeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UExQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UExQuestManagerSubsystem>())
		{
			QuestManager->OnQuestStateChanged.AddDynamic(this, &UExQuestTreeWidget::HandleQuestStateChanged);
			QuestManager->OnQuestProgressChanged.AddDynamic(this, &UExQuestTreeWidget::HandleQuestProgressChanged);
			QuestManager->OnQuestObjectiveUpdated.AddDynamic(this, &UExQuestTreeWidget::HandleQuestObjectiveUpdated);
			QuestManager->OnQuestDataLoaded.AddDynamic(this, &UExQuestTreeWidget::HandleQuestDataLoaded);
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
			QuestManager->OnQuestObjectiveUpdated.RemoveDynamic(this, &UExQuestTreeWidget::HandleQuestObjectiveUpdated);
			QuestManager->OnQuestDataLoaded.RemoveDynamic(this, &UExQuestTreeWidget::HandleQuestDataLoaded);
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

void UExQuestTreeWidget::HandleQuestObjectiveUpdated(const FExQuestObjective& Objective)
{
	RefreshQuestTree();
}

void UExQuestTreeWidget::HandleQuestDataLoaded()
{
	RefreshQuestTree();
}

void UExQuestTreeWidget::SyncDisplayedDataFromManager()
{
	if (!bAutoSyncFromManager)
	{
		return;
	}

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UExQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UExQuestManagerSubsystem>())
		{
			const FExQuestData& ManagerData = QuestManager->GetQuestData();
			if (ManagerData.AllTasks.Num() > 0)
			{
				DisplayedQuestData = ManagerData;

				if (const UExQuestDataAsset* QuestAsset = QuestManager->GetLoadedQuestAsset())
				{
					DisplayedQuestData.EnrichMetadataFrom(QuestAsset->BuildInitialQuestData());
				}
			}
		}
	}
}

void UExQuestTreeWidget::SyncExpansionStateFromQuestData()
{
	for (const FExQuestTask& Task : DisplayedQuestData.AllTasks)
	{
		if (!Task.TaskId.IsValid())
		{
			continue;
		}

		const FString TaskIdStr = Task.TaskId.ToString();
		const EExQuestState* const PreviousStatePtr = PreviousTaskStates.Find(TaskIdStr);
		const bool bHadPreviousState = PreviousStatePtr != nullptr;
		const EExQuestState PreviousState = bHadPreviousState ? *PreviousStatePtr : EExQuestState::Locked;

		if (Task.State == EExQuestState::Completed)
		{
			if (bAutoCollapseOnComplete)
			{
				ExpandedTaskIds.Remove(TaskIdStr);
			}
			else if (bHadPreviousState && PreviousState == EExQuestState::Active && Task.Objectives.Num() > 0)
			{
				ExpandedTaskIds.Add(TaskIdStr);
			}
		}

		PreviousTaskStates.Add(TaskIdStr, Task.State);
	}
}

void UExQuestTreeWidget::RefreshQuestTree()
{
	SyncDisplayedDataFromManager();
	SyncExpansionStateFromQuestData();

	if (TitleText && !DisplayedQuestData.QuestSetName.IsEmpty())
	{
		TitleText->SetText(DisplayedQuestData.QuestSetName);
	}

	ExpandClickHandlers.Empty();

	if (RootQuestContainer)
	{
		RootQuestContainer->ClearChildren();
		BuildQuestTree();
	}
}

void UExQuestTreeWidget::SetQuestData(const FExQuestData& QuestData)
{
	DisplayedQuestData = QuestData;

	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UExQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UExQuestManagerSubsystem>())
		{
			QuestManager->LoadQuestData(QuestData);
		}
	}

	RefreshQuestTree();
}

void UExQuestTreeWidget::BuildQuestTree()
{
	if (!RootQuestContainer)
	{
		return;
	}

	const TArray<FExQuestTask> RootTasks = DisplayedQuestData.GetRootTasks();
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

	const FString TaskIdStr = QuestTask.TaskId.ToString();
	const bool bHasChildren = DisplayedQuestData.GetSubTasks(QuestTask.TaskId).Num() > 0 || QuestTask.Objectives.Num() > 0;
	const bool bExpanded = (bAutoExpandActiveQuests && QuestTask.State == EExQuestState::Active)
		|| IsQuestExpanded(TaskIdStr);

	UHorizontalBox* RowBox = NewObject<UHorizontalBox>(ParentContainer);
	if (!RowBox)
	{
		return;
	}

	if (bHasChildren)
	{
		if (UButton* ExpandButton = NewObject<UButton>(ParentContainer))
		{
			if (UTextBlock* ExpandLabel = NewObject<UTextBlock>(ExpandButton))
			{
				ExpandLabel->SetText(FText::FromString(bExpanded ? TEXT("-") : TEXT("+")));
				ExpandButton->AddChild(ExpandLabel);
			}

			UExQuestExpandClickHandler* ClickHandler = NewObject<UExQuestExpandClickHandler>(this);
			ClickHandler->Setup(this, TaskIdStr);
			ExpandButton->OnClicked.AddDynamic(ClickHandler, &UExQuestExpandClickHandler::OnClicked);
			ExpandClickHandlers.Add(ClickHandler);

			if (UHorizontalBoxSlot* ButtonSlot = RowBox->AddChildToHorizontalBox(ExpandButton))
			{
				ButtonSlot->SetPadding(FMargin(0.0f, 2.0f, 4.0f, 2.0f));
				ButtonSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			}
		}
	}
	else
	{
		if (USpacer* Spacer = NewObject<USpacer>(ParentContainer))
		{
			Spacer->SetSize(FVector2D(24.0f, 1.0f));
			RowBox->AddChildToHorizontalBox(Spacer);
		}
	}

	if (UTextBlock* TaskNameText = NewObject<UTextBlock>(ParentContainer))
	{
		const FText TaskDisplayText = FText::Format(
			NSLOCTEXT("QuestUI", "TaskNameFormat", "{0} ({1})"),
			QuestTask.TaskName,
			UExQuestBlueprintLibrary::GetQuestStateText(QuestTask.State));
		TaskNameText->SetText(TaskDisplayText);
		TaskNameText->SetColorAndOpacity(UExQuestBlueprintLibrary::GetQuestStateColor(QuestTask.State));
		RowBox->AddChildToHorizontalBox(TaskNameText);
	}

	const FMargin IndentMargin(Depth * 20.0f, 2.0f, 0.0f, 2.0f);
	if (UVerticalBoxSlot* RowSlot = ParentContainer->AddChildToVerticalBox(RowBox))
	{
		RowSlot->SetPadding(IndentMargin);
	}

	if (bExpanded)
	{
		for (const FExQuestObjective& Objective : QuestTask.Objectives)
		{
			if (UTextBlock* ObjectiveText = NewObject<UTextBlock>(ParentContainer))
			{
				const FText ObjectiveDisplayText = FText::Format(
					NSLOCTEXT("QuestUI", "ObjectiveFormat", "  - {0} ({1}/{2})"),
					ExQuestTreeWidgetInternal::GetObjectiveDisplayText(Objective),
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

		const TArray<FExQuestTask> SubTasks = DisplayedQuestData.GetSubTasks(QuestTask.TaskId);
		for (const FExQuestTask& SubTask : SubTasks)
		{
			CreateQuestItem(SubTask, ParentContainer, Depth + 1);
		}
	}
}

FSlateColor UExQuestTreeWidget::GetStateColor(EExQuestState State) const
{
	return FSlateColor(UExQuestBlueprintLibrary::GetQuestStateColor(State));
}

FText UExQuestTreeWidget::GetStateText(EExQuestState State) const
{
	return UExQuestBlueprintLibrary::GetQuestStateText(State);
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
