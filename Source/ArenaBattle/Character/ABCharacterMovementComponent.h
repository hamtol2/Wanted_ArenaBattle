// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ABCharacterMovementComponent.generated.h"

// 움직임 데이터 변경을 위한 클래스 확장.
class FABSavedMove_Character : public FSavedMove_Character
{
	// 편의 목적을 위해 Super 키워드 지정.
	using Super = FSavedMove_Character;

public:

	/** Clear saved move properties, so it can be re-used. */
	virtual void Clear() override;

	/** Set the properties describing the position, etc. of the moved pawn at the start of the move. */
	virtual void SetInitialPosition(ACharacter* Character) override;

	/** Returns a byte containing encoded special movement information (jumping, crouching, etc.)	 */
	virtual uint8 GetCompressedFlags() const override;

	// 텔레포트 입력이 눌렸는지를 확인하는 변수.
	uint8 bPressedTeleport : 1;

	// 텔레포트 쿨타임 제어에 사용할 변수.
	uint8 bDidTeleport : 1;
};

class FABNetworkPredictionData_Client_Character
	: public FNetworkPredictionData_Client_Character
{
	// 편의 목적을 위해 Super 키워드 지정.
	//typedef FNetworkPredictionData_Client_Character Super;
	using Super = FNetworkPredictionData_Client_Character;

public:
	FABNetworkPredictionData_Client_Character(
		const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};

/**
 * 
 */
UCLASS()
class ARENABATTLE_API UABCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UABCharacterMovementComponent();

	// 캐릭터 쪽에서 텔레포트 입력이 눌리면, 실행할 함수.
	void SetTeleportCommand();

protected:

	// 텔레포트 처리 함수.
	virtual void ABTeleport();

	// PerformMovement 함수 내부에서 호출되는 함수 오버라이드.
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	// 클라이언트에서 서버로 텔레포트 명령을 보낼 때 사용할 예정.
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	// 클라이언트에서 압축해서 전송한 플래그를 서버에서 받을 때 실행되는 함수.
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

public:

	// 텔레포트 입력이 눌렸는지를 확인하는 변수.
	uint8 bPressedTeleport : 1;

	// 텔레포트 쿨타임 제어에 사용할 변수.
	uint8 bDidTeleport : 1;

protected:

	// 텔레포트 이동 거리.
	UPROPERTY()
	float TeleportOffset;

	// 텔레포트 쿨타임.
	UPROPERTY()
	float TeleportCooltime;
};