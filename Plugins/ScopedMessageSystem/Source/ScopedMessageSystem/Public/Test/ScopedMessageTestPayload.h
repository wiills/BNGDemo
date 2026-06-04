#pragma once

#include "CoreMinimal.h"
#include "ScopedMessageTestPayload.generated.h"

USTRUCT(BlueprintType)
struct FScopedMessageTestPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	int32 Counter = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FVector Location = FVector::ZeroVector;
};
