// Made by Jenn.zip

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BackgroundKeySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBackgroundHotKeyPressed, FKey, Key);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBackgroundHotKeyRegistrationChanged, bool, bRegistered);

class FBackgroundKeyListener;

UCLASS(BlueprintType)
class UNFOCUSEDHOTKEYS_API UBackgroundKeySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Background Keys")
	FBackgroundHotKeyPressed OnHotKeyPressed;

	UPROPERTY(BlueprintAssignable, Category = "Background Keys")
	FBackgroundHotKeyRegistrationChanged OnRegistrationChanged;

	UFUNCTION(BlueprintCallable, Category = "Background Keys")
	bool SetGlobalHotKey(FKey Key, bool bControl, bool bAlt, bool bShift, bool bWindows);

	UFUNCTION(BlueprintCallable, Category = "Background Keys")
	void DisableGlobalHotKey();

	UFUNCTION(BlueprintPure, Category = "Background Keys")
	FKey GetGlobalHotKey() const;

	virtual void Deinitialize() override;

private:
	void StopListener();

	FBackgroundKeyListener* Listener = nullptr;
	FKey CurrentKey;
};