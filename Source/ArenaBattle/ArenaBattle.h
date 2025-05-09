// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// 넷모드(네트워크 모드) 정보.
#define LOG_NETMODEINFO ( (GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ( ( GetNetMode() == ENetMode::NM_Standalone ) ? TEXT("STANDALONE") : TEXT("SERVER"))  )

// 함수 호출 정보.
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

// 로그 매크로.
#define AB_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogABNetwork, Log, All);