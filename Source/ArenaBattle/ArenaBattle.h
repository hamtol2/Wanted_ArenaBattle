// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// �ݸ��(��Ʈ��ũ ���) ����.
#define LOG_NETMODEINFO ( (GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), GPlayInEditorID) : ( ( GetNetMode() == ENetMode::NM_Standalone ) ? TEXT("STANDALONE") : TEXT("SERVER"))  )

// �Լ� ȣ�� ����.
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

// �α� ��ũ��.
#define AB_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogABNetwork, Log, All);