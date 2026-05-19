// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "ExLevelTrigger.generated.h"


UENUM(BlueprintType)
enum class EExLevelTriggerShapeType : uint8
{
	Box,
	Sphere,
	Capsule
};

/**
 * Actor With a Shape
 */
UCLASS(Abstract)
class BLUEPRINTNODEGRAPH_API AExLevelTrigger : public AActor
{
	GENERATED_BODY()

	TMap<EExLevelTriggerShapeType,TSubclassOf<UShapeComponent>> ShapeConfigs;
	
	UPROPERTY()
	TMap<EExLevelTriggerShapeType,UShapeComponent*> CompsMap;
	
public:
	AExLevelTrigger();
	
	UPROPERTY(VisibleDefaultsOnly)
	USceneComponent* Root;
	UPROPERTY(VisibleInstanceOnly,Category="Shape")
	UShapeComponent* ShapeComponent;
	
#if WITH_EDITORONLY_DATA
	/** Billboard used to see the trigger in the editor */
	UPROPERTY(VisibleDefaultsOnly)
	UBillboardComponent* Billboard;
#endif
	
	/******* Shape *******/
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Shape")
	EExLevelTriggerShapeType ShapeType=EExLevelTriggerShapeType::Box;
	/******* Shape *******/
	
	UPROPERTY(EditAnywhere,Category="Shape",meta=(EditCondition=bUseCustomCollision))
	FBodyInstance BodyInstance;

protected:
	virtual void BeginPlay() override;

	// for child-class Register
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
	virtual void RegisterDelegates() {}
	
#if WITH_EDITOR
	virtual void PostActorCreated() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	void GenerateShape();
	void SpawnNewShapeComponent();
	virtual void OnGenerateShape() { ReceiveOnGenerateShape(); }
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveOnGenerateShape();
};
