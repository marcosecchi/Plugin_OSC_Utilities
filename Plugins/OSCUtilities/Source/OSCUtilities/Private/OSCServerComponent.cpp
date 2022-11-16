#include "OSCServerComponent.h"
#include "OSCConfigSettings.h"
#include "OSCUtilitiesLog.h"
#include <SocketSubsystem.h>
#include <IPAddress.h>


// Called when the game starts or when spawned
void UOSCServerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Create Server
	UE_LOG(LogOSCUtilities, Display, TEXT("Creating OSC Server from Component"));
	if(bAutoInitServerIP)
	{
		ServerIP = GetLocalIPAddress();
	}
	Server = UOSCManager::CreateOSCServer(GetLocalIPAddress(), ServerPort, false, false, ServerName);
	Server->OnOscMessageReceived.AddDynamic(this, &UOSCServerComponent::OnReceivedMessageEvent);

	InitConfigData();
	
	if(bAutoplay)
	{
		StartListening();
	}
}

void UOSCServerComponent::InitConfigData()
{
	UE_LOG(LogOSCUtilities, Display, TEXT("Initializing %d datasets"), DataSets.Num());
	for(const auto Handle: DataSets)
	{
		if(Handle.IsNull()) continue;
		const auto RowName = Handle.RowName;
		const auto Context = "";
		if(Handle.DataTable->FindRow<FOSCConfigSettings>(RowName, Context) == nullptr) continue;

		auto Data = *Handle.DataTable->FindRow<FOSCConfigSettings>(RowName, Context);
		ConfigSettingsMap.Add(Data.Command, Data);

		MinValuesMap.Add(Data.Command, 0);
		MaxValuesMap.Add(Data.Command, 4096);

		TArray<int> Samples;
		Samples.Init(0, NumSamples);
		SamplesMap.Add(Data.Command, Samples);
	}
	UE_LOG(LogOSCUtilities, Display, TEXT("Initialized %d datasets"), ConfigSettingsMap.Num());

}

void UOSCServerComponent::StartListening() const
{
	Server->Listen();
	OnServerStart.Broadcast();
	UE_LOG(LogOSCUtilities, Display, TEXT("OSC Server started listening."));
}

void UOSCServerComponent::StopListening() const
{
	Server->Stop();
	OnServerStop.Broadcast();
	UE_LOG(LogOSCUtilities, Display, TEXT("OSC Server stopped listening."));
}

FOSCConfigSettings UOSCServerComponent::GetConfigSettings(const FString Key)
{
	if(!ConfigSettingsMap.Contains(Key))
	{
		//		UE_LOG(LogOSCUtilities, Warning, TEXT("No data with base address '%s' found"), Key);
	//	return nullptr;
	}
	return *ConfigSettingsMap.Find(Key);
}

void UOSCServerComponent::SetMinValue(const FString Key)
{
	const auto Val = GetComputedData(Key);
	MinValuesMap.Add(Key, Val);
	UE_LOG(LogOSCUtilities, Display, TEXT("Setting minimum value to: %f"), Val);
	if(Val >= GetMaxValue(Key))
	{
		UE_LOG(LogOSCUtilities, Warning, TEXT("Minimum value is greater than maximum value."));
	}
}

float UOSCServerComponent::GetMinValue(const FString Key)
{
	if(!MinValuesMap.Contains(Key))
	{
		UE_LOG(LogOSCUtilities, Warning, TEXT("No data with base address '%s' found"), *Key);
		return -1;
	}
	return *MinValuesMap.Find(Key);
}

void UOSCServerComponent::SetMaxValue(const FString Key)
{
	const auto Val = GetComputedData(Key);
	MaxValuesMap.Add(Key, Val);
	UE_LOG(LogOSCUtilities, Display, TEXT("Setting maximum value to: %f"), Val);
	if(Val <= GetMinValue(Key))
	{
		UE_LOG(LogOSCUtilities, Warning, TEXT("Maximum value is less than minimum value."));
	}
}

float UOSCServerComponent::GetMaxValue(const FString Key)
{
	if(!MaxValuesMap.Contains(Key))
	{
		UE_LOG(LogOSCUtilities, Error, TEXT("No data with base address '%s' found"), *Key);
		return -1;
	}
	return *MaxValuesMap.Find(Key);
}

float UOSCServerComponent::GetComputedData(const FString Key)
{
	if(!SamplesMap.Contains(Key)) return 0;
	const auto Samples = *SamplesMap.Find(Key);
	auto Count = 0;
	for (const auto Val: Samples)
	{
		Count += Val;
	}
	
	const auto Result = static_cast<float>(Count) / static_cast<float>(Samples.Num());
	return Result;
}

float UOSCServerComponent::GetNormalizedComputedData(const FString Key)
{
	const auto Data =  GetComputedData(Key);
	const auto MinIncomingValue = MinValuesMap.Find(Key);
	const auto MaxIncomingValue =  MaxValuesMap.Find(Key);
	const auto ConfigSettings = GetConfigSettings(Key);
	const auto Result = FMath::GetMappedRangeValueClamped(
				FVector2D(*MinIncomingValue, *MaxIncomingValue),
				FVector2D(ConfigSettings.MinValue, ConfigSettings.MaxValue),
				Data);

	return Result;
}

void UOSCServerComponent::OnReceivedMessageEvent(const FOSCMessage& Message, const FString& IPAddress, int32 Port)
{
	const auto MessageAddress = Message.GetAddress().GetFullPath();

	for(const auto ConfigSettings: ConfigSettingsMap)
	{
		const auto ConfigKey = ConfigSettings.Key;
		const auto ConfigValue = ConfigSettings.Value;

		if(MessageAddress.Equals(ConfigValue.MinValueMessageAddress, ESearchCase::IgnoreCase))
		{
			SetMinValue(ConfigKey);
			OnMinValueSet.Broadcast(ConfigValue.Command, GetMinValue(ConfigKey));
		}
		else if(MessageAddress.Equals(ConfigValue.MaxValueMessageAddress, ESearchCase::IgnoreCase))
		{
			SetMaxValue(ConfigKey);
			OnMaxValueSet.Broadcast(ConfigValue.Command, GetMaxValue(ConfigKey));
		}
		else if(MessageAddress.Equals(ConfigValue.DataMessageAddress, ESearchCase::IgnoreCase))
		{
			int32 IncomingValue;
			UOSCManager::GetInt32(Message, 0, IncomingValue);
			AddIncomingValue(ConfigKey, IncomingValue);

			OnDataUpdate.Broadcast(ConfigValue.Command, GetNormalizedComputedData(ConfigKey));
		}
//		else
//		{
//			UE_LOG(LogOSCUtilities, Warning, TEXT("Message address '%s' not found"), *MessageAddress);
//		}
	}
}

void UOSCServerComponent::AddIncomingValue(FString Key, int32 Value)
{
	if(!SamplesMap.Contains(Key)) return;
	const auto Samples = SamplesMap.Find(Key);
	Samples->Add(Value);
	if(Samples->Num() > NumSamples)
	{
		Samples->RemoveAt(0);
	}
}

FString UOSCServerComponent::GetLocalIPAddress()
{
	bool CanBind = false;
	const TSharedRef<FInternetAddr> LocalIP = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, CanBind);
	return (LocalIP->IsValid() ? LocalIP->ToString(false) : "");
}

