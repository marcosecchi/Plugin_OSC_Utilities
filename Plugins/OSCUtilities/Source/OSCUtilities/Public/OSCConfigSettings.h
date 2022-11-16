// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OSCConfigSettings.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct OSCUTILITIES_API FOSCConfigSettings : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Command = "DoSomething";

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString DataMessageAddress = "/default/data";

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString MinValueMessageAddress = "/default/minvalue";

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString MaxValueMessageAddress = "/default/maxvalue";

	// The min value to be set in the Unreal Editor
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinValue = 0;

	// The max value to be set in the Unreal Editor
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxValue = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Notes = "";
};