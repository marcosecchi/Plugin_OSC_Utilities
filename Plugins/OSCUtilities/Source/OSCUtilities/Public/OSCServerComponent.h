// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OSCConfigSettings.h"
#include "Engine/DataTable.h"
#include "OSC/public/OSCServer.h"
#include "OSC/public/OSCManager.h"
#include "OSCServerComponent.generated.h"

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOSCServerStartEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOSCServerStopEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOSCDataUpdateEvent, const FString, Command, const float, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOSCMinValueSetEvent, const FString, Command, const int32, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOSCMaxValueSetEvent, const FString, Command, const int32, Data);

UCLASS(Blueprintable, BlueprintType, ClassGroup=("OSC-Utilities"), meta=(BlueprintSpawnableComponent))
class OSCUTILITIES_API UOSCServerComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OSC|Server Settings")
	bool bAutoInitServerIP = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OSC|Server Settings")
	FString ServerIP = "127.0.0.1";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OSC|Server Settings")
	int32 ServerPort = 3333;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OSC|Server Settings")
	FString ServerName = "Default Server";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OSC|Server Settings")
	int32 NumSamples = 10;

	/** Should the server automatically start at Begin Play? */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OSC|Server Settings")
	bool bAutoplay = true;

	/** The configuration data to be used for message parsing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="OSC|Data Config")
	TArray<FDataTableRowHandle> DataSets;

	/** Starts the server */
	UFUNCTION(BlueprintCallable)
	void StartListening() const;

	/** Stops the server */
	UFUNCTION(BlueprintCallable)
	void StopListening() const;

	/** The OSC server */
	UPROPERTY(BlueprintReadOnly)
	UOSCServer* Server;

	/** Returns the computed data from the sampled collection */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetComputedData(FString Key);

	/** Returns the computed data from the sampled collection (normalized) */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetNormalizedComputedData(FString Key);
	
	// Events
	
	/** Event that gets called when an OSC message is received. */
	UPROPERTY(BlueprintAssignable)
	FOSCDataUpdateEvent OnDataUpdate;

	/** Event that gets called when the OSC server is started. */
	UPROPERTY(BlueprintAssignable)
	FOSCServerStartEvent OnServerStart;

	/** Event that gets called when the OSC server is stopped. */
	UPROPERTY(BlueprintAssignable)
	FOSCServerStopEvent OnServerStop;

	/** Event that gets called when the minimum value is set. */
	UPROPERTY(BlueprintAssignable)
	FOSCMinValueSetEvent OnMinValueSet;

	/** Event that gets called when the maximum value is set. */
	UPROPERTY(BlueprintAssignable)
	FOSCMaxValueSetEvent OnMaxValueSet;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	/** Returns the local IP Address */
	static FString GetLocalIPAddress();

protected:
	virtual void BeginPlay() override;

	TMap<FString, FOSCConfigSettings> ConfigSettingsMap;

	TMap<FString, float> MinValuesMap;

	TMap<FString, float> MaxValuesMap;

	TMap<FString, TArray<int>>  SamplesMap; 

	/** Handles the incoming data from an OSC client */
	UFUNCTION()
	void InitConfigData();

	UFUNCTION()
	FOSCConfigSettings GetConfigSettings(FString Key);

	/** Sets the min value using the actual computed data */
	UFUNCTION()
	void SetMinValue(FString Key);

	UFUNCTION()
	float GetMinValue(FString Key);

	/** Sets the max value using the actual computed data */
	UFUNCTION()
	void SetMaxValue(FString Key);

	UFUNCTION()
	float GetMaxValue(FString Key);

	/** Handles the incoming data from an OSC client */
	UFUNCTION()
	void OnReceivedMessageEvent(const FOSCMessage& Message, const FString& IPAddress, int32 Port);

	/** Sets the max value using the actual computed data */
	UFUNCTION()
	void AddIncomingValue(FString Key, int32 Value);
	
};
