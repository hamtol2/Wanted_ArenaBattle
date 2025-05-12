// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// ��Ʈ��ũ ���� ���� ����.
#define LOG_LOCALROLLINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole()))

// ��Ʈ��ũ ����Ʈ ���� ����.
#define LOG_REMOTEROLLINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole()))

// �ݸ��(��Ʈ��ũ ���) ����.
#define LOG_NETMODEINFO ( (GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT%d"), (int32)GPlayInEditorID) : ( GetNetMode() == ENetMode::NM_Standalone ) ? TEXT("STANDALONE") : TEXT("SERVER"))

// �Լ� ȣ�� ����.
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

// �α� ��ũ��.
#define AB_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s][%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLLINFO, LOG_REMOTEROLLINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogABNetwork, Log, All);