#pragma once

#include "CoreMinimal.h"
#include "ScopedMessagePoiDemoTypes.generated.h"

USTRUCT(BlueprintType)
struct FScopedMessageDemoTerminalActivatedPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FName TerminalId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	int32 ActivationCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FVector Location = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FScopedMessageDemoDoorOpenedPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FName DoorId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FName TriggeredByTerminalId = NAME_None;
};
