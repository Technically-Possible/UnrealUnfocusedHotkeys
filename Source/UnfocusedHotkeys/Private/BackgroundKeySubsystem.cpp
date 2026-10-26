// Made by Jenn.zip

#include "BackgroundKeySubsystem.h"

#include "Async/Async.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include "Windows/HideWindowsPlatformTypes.h"

namespace
{
	struct FNamedVirtualKey
	{
		const TCHAR* Name;
		uint32 VirtualKey;
	};

	bool TryGetVirtualKey(const FKey& Key, uint32& OutVirtualKey)
	{
		const FString Name = Key.GetFName().ToString();

		if (Name.Len() == 1)
		{
			const TCHAR Character = FChar::ToUpper(Name[0]);
			if ((Character >= TEXT('A') && Character <= TEXT('Z')) ||
				(Character >= TEXT('0') && Character <= TEXT('9')))
			{
				OutVirtualKey = static_cast<uint32>(Character);
				return true;
			}
		}

		static const TCHAR* DigitNames[] = {
			TEXT("Zero"), TEXT("One"), TEXT("Two"), TEXT("Three"), TEXT("Four"),
			TEXT("Five"), TEXT("Six"), TEXT("Seven"), TEXT("Eight"), TEXT("Nine")
		};
		for (uint32 Digit = 0; Digit < UE_ARRAY_COUNT(DigitNames); ++Digit)
		{
			if (Name == DigitNames[Digit])
			{
				OutVirtualKey = static_cast<uint32>('0' + Digit);
				return true;
			}
		}

		if (Name.StartsWith(TEXT("F")) && Name.Len() <= 3)
		{
			const int32 FunctionNumber = FCString::Atoi(*Name.RightChop(1));
			if (FunctionNumber >= 1 && FunctionNumber <= 12)
			{
				OutVirtualKey = static_cast<uint32>(VK_F1 + FunctionNumber - 1);
				return true;
			}
		}

		static const FNamedVirtualKey NamedKeys[] = {
			{TEXT("SpaceBar"), VK_SPACE}, {TEXT("Enter"), VK_RETURN},
			{TEXT("Escape"), VK_ESCAPE}, {TEXT("Tab"), VK_TAB},
			{TEXT("BackSpace"), VK_BACK}, {TEXT("Insert"), VK_INSERT},
			{TEXT("Delete"), VK_DELETE}, {TEXT("Home"), VK_HOME},
			{TEXT("End"), VK_END}, {TEXT("PageUp"), VK_PRIOR},
			{TEXT("PageDown"), VK_NEXT}, {TEXT("Left"), VK_LEFT},
			{TEXT("Right"), VK_RIGHT}, {TEXT("Up"), VK_UP},
			{TEXT("Down"), VK_DOWN}, {TEXT("CapsLock"), VK_CAPITAL},
			{TEXT("Comma"), VK_OEM_COMMA}, {TEXT("Period"), VK_OEM_PERIOD},
			{TEXT("Hyphen"), VK_OEM_MINUS}, {TEXT("Equals"), VK_OEM_PLUS},
			{TEXT("LeftBracket"), VK_OEM_4}, {TEXT("RightBracket"), VK_OEM_6},
			{TEXT("Backslash"), VK_OEM_5}, {TEXT("Slash"), VK_OEM_2},
			{TEXT("Semicolon"), VK_OEM_1}, {TEXT("Apostrophe"), VK_OEM_7},
			{TEXT("Tilde"), VK_OEM_3}
		};
		for (const FNamedVirtualKey& NamedKey : NamedKeys)
		{
			if (Name == NamedKey.Name)
			{
				OutVirtualKey = NamedKey.VirtualKey;
				return true;
			}
		}

		return false;
	}
}

class FBackgroundKeyListener final : public FRunnable
{
public:
	FBackgroundKeyListener(uint32 InVirtualKey, uint32 InModifiers, TFunction<void()> InOnPressed,
		TFunction<void(bool)> InOnRegistrationChanged)
		: VirtualKey(InVirtualKey)
		, Modifiers(InModifiers)
		, OnPressed(MoveTemp(InOnPressed))
		, OnRegistrationChanged(MoveTemp(InOnRegistrationChanged))
	{
		ReadyEvent = FPlatformProcess::GetSynchEventFromPool(true);
	}

	virtual ~FBackgroundKeyListener() override
	{
		StopAndJoin();
		FPlatformProcess::ReturnSynchEventToPool(ReadyEvent);
	}

	bool Start()
	{
		Thread = FRunnableThread::Create(this, TEXT("BackgroundKeyListener"));
		if (!Thread)
		{
			return false;
		}

		ReadyEvent->Wait();
		return bRegistered;
	}

	void StopAndJoin()
	{
		if (!Thread)
		{
			return;
		}

		if (ThreadId != 0)
		{
			::PostThreadMessageW(ThreadId, WM_QUIT, 0, 0);
		}

		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}

	virtual uint32 Run() override
	{
		ThreadId = ::GetCurrentThreadId();

		// Creates this thread's message queue so WM_HOTKEY and WM_QUIT can be delivered to it.
		MSG Message;
		::PeekMessageW(&Message, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

		constexpr int32 HotKeyId = 0x4A31;
		if (!::RegisterHotKey(nullptr, HotKeyId, Modifiers | MOD_NOREPEAT, VirtualKey))
		{
			bRegistered = false;
			OnRegistrationChanged(false);
			ReadyEvent->Trigger();
			return 1;
		}

		bRegistered = true;
		OnRegistrationChanged(true);
		ReadyEvent->Trigger();
		while (::GetMessageW(&Message, nullptr, 0, 0) > 0)
		{
			if (Message.message == WM_HOTKEY && static_cast<int32>(Message.wParam) == HotKeyId)
			{
				OnPressed();
			}
		}

		::UnregisterHotKey(nullptr, HotKeyId);
		OnRegistrationChanged(false);
		return 0;
	}

private:
	uint32 VirtualKey;
	uint32 Modifiers;
	TFunction<void()> OnPressed;
	TFunction<void(bool)> OnRegistrationChanged;
	FEvent* ReadyEvent = nullptr;
	FRunnableThread* Thread = nullptr;
	uint32 ThreadId = 0;
	bool bRegistered = false;
};
#endif

bool UBackgroundKeySubsystem::SetGlobalHotKey(FKey Key, bool bControl, bool bAlt, bool bShift, bool bWindows)
{
#if PLATFORM_WINDOWS
	uint32 VirtualKey = 0;
	if (!Key.IsValid() || !TryGetVirtualKey(Key, VirtualKey))
	{
		return false;
	}

	StopListener();
	CurrentKey = Key;

	uint32 Modifiers = 0;
	Modifiers |= bControl ? MOD_CONTROL : 0;
	Modifiers |= bAlt ? MOD_ALT : 0;
	Modifiers |= bShift ? MOD_SHIFT : 0;
	Modifiers |= bWindows ? MOD_WIN : 0;

	const TWeakObjectPtr<UBackgroundKeySubsystem> WeakThis(this);
	Listener = new FBackgroundKeyListener(
		VirtualKey,
		Modifiers,
		[WeakThis, Key]()
		{
			AsyncTask(ENamedThreads::GameThread, [WeakThis, Key]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->OnHotKeyPressed.Broadcast(Key);
				}
			});
		},
		[WeakThis](bool bRegistered)
		{
			AsyncTask(ENamedThreads::GameThread, [WeakThis, bRegistered]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->OnRegistrationChanged.Broadcast(bRegistered);
				}
			});
		});

	if (!Listener->Start())
	{
		StopListener();
		CurrentKey = FKey();
		return false;
	}
	return true;
#else
	return false;
#endif
}

void UBackgroundKeySubsystem::DisableGlobalHotKey()
{
	StopListener();
	CurrentKey = FKey();
}

FKey UBackgroundKeySubsystem::GetGlobalHotKey() const
{
	return CurrentKey;
}

void UBackgroundKeySubsystem::Deinitialize()
{
	StopListener();
	Super::Deinitialize();
}

void UBackgroundKeySubsystem::StopListener()
{
#if PLATFORM_WINDOWS
	if (Listener)
	{
		Listener->StopAndJoin();
		delete Listener;
		Listener = nullptr;
	}
#endif
}
