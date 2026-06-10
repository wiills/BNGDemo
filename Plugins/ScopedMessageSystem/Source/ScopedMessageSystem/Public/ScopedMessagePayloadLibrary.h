#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ScopedMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "ScopedMessagePayloadLibrary.generated.h"

UCLASS()
class SCOPEDMESSAGESYSTEM_API UScopedMessagePayloadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Scoped Message|Payload")
	static bool IsPayloadValid(const FScopedMessagePayload& Payload);

	UFUNCTION(BlueprintPure, Category = "Scoped Message|Payload")
	static bool IsPayloadOfType(const FScopedMessagePayload& Payload, const UScriptStruct* PayloadType);

	UFUNCTION(BlueprintPure, Category = "Scoped Message|Payload")
	static FString GetPayloadStructPath(const FScopedMessagePayload& Payload);

	UFUNCTION(BlueprintPure, Category = "Scoped Message|Payload")
	static FName GetPayloadStructName(const FScopedMessagePayload& Payload);

	UFUNCTION(BlueprintPure, Category = "Scoped Message|Payload")
	static int32 GetPayloadSize(const FScopedMessagePayload& Payload);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Payload")
	static bool MakePayloadFromInstancedStruct(const FInstancedStruct& InstancedStruct, FScopedMessagePayload& OutPayload);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Payload")
	static bool MakeInstancedStructFromPayload(const FScopedMessagePayload& Payload, FInstancedStruct& OutInstancedStruct);
};
