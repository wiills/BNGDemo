// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/Assets/ExLevelTrigger.h"

#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"


AExLevelTrigger::AExLevelTrigger()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = Root = CreateDefaultSubobject<USceneComponent>("RootComponent");

#if WITH_EDITOR
	Billboard = CreateDefaultSubobject<UBillboardComponent>("BillboardComponent");
	Billboard->SetupAttachment(RootComponent);
	Billboard->bIsEditorOnly = true;
#endif

	// set config
	ShapeConfigs.Add(EExLevelTriggerShapeType::Box, UBoxComponent::StaticClass());
	ShapeConfigs.Add(EExLevelTriggerShapeType::Sphere, USphereComponent::StaticClass());
	ShapeConfigs.Add(EExLevelTriggerShapeType::Capsule, UCapsuleComponent::StaticClass());

	BodyInstance.SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

void AExLevelTrigger::BeginPlay()
{
	Super::BeginPlay();
}

void AExLevelTrigger::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

void AExLevelTrigger::PostLoad()
{
	Super::PostLoad();
	GenerateShape();
}

#if WITH_EDITOR
void AExLevelTrigger::PostActorCreated()
{
	Super::PostActorCreated();
	GenerateShape();
}

void AExLevelTrigger::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const auto PropertyName = PropertyChangedEvent.Property->GetName();
	// shape
	if (PropertyName == "ShapeType")
	{
		GenerateShape();
	}
	if (PropertyName == "CollisionResponses")
	{
		if (ShapeComponent)
			ShapeComponent->SetCollisionProfileName(BodyInstance.GetCollisionProfileName());
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void AExLevelTrigger::GenerateShape()
{
	if (ShapeComponent)
	{
		if (ShapeComponent->GetClass() == ShapeConfigs.FindChecked(ShapeType))
		{
			ShapeComponent->SetCollisionProfileName(BodyInstance.GetCollisionProfileName());
			ShapeComponent->SetCanEverAffectNavigation(false);
			return;
		}
		ShapeComponent->SetVisibility(false, true);
	}
	const auto CompValuePtr = CompsMap.Find(ShapeType);
	if (CompValuePtr)
	{
		const auto ValueComp = *CompValuePtr;
		ShapeComponent = ValueComp;
		ShapeComponent->SetVisibility(true, true);
		return;
	}
	// create new
	if (GetWorld())
	{
		SpawnNewShapeComponent();
	}
}

void AExLevelTrigger::SpawnNewShapeComponent()
{
	const auto ShapeClass = ShapeConfigs.FindChecked(ShapeType);
	const auto TempComp = NewObject<UShapeComponent>(this, ShapeClass, NAME_None, RF_StrongRefOnFrame);
	if (!TempComp)
		return;
	ShapeComponent = TempComp;
	AddOwnedComponent(ShapeComponent);
	ShapeComponent->RegisterComponent();
	ShapeComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	ShapeComponent->SetCollisionProfileName(BodyInstance.GetCollisionProfileName());
	ShapeComponent->SetCanEverAffectNavigation(false);
	CompsMap.Add(ShapeType, ShapeComponent);
}
