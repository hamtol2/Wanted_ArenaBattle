// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ABCharacterMovementComponent.generated.h"

// ������ ������ ������ ���� Ŭ���� Ȯ��.
class FABSavedMove_Character : public FSavedMove_Character
{
	// ���� ������ ���� Super Ű���� ����.
	using Super = FSavedMove_Character;

public:

	/** Clear saved move properties, so it can be re-used. */
	virtual void Clear() override;

	/** Set the properties describing the position, etc. of the moved pawn at the start of the move. */
	virtual void SetInitialPosition(ACharacter* Character) override;

	/** Returns a byte containing encoded special movement information (jumping, crouching, etc.)	 */
	virtual uint8 GetCompressedFlags() const override;

	// �ڷ���Ʈ �Է��� ���ȴ����� Ȯ���ϴ� ����.
	uint8 bPressedTeleport : 1;

	// �ڷ���Ʈ ��Ÿ�� ��� ����� ����.
	uint8 bDidTeleport : 1;
};

class FABNetworkPredictionData_Client_Character
	: public FNetworkPredictionData_Client_Character
{
	// ���� ������ ���� Super Ű���� ����.
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

	// ĳ���� �ʿ��� �ڷ���Ʈ �Է��� ������, ������ �Լ�.
	void SetTeleportCommand();

protected:

	// �ڷ���Ʈ ó�� �Լ�.
	virtual void ABTeleport();

	// PerformMovement �Լ� ���ο��� ȣ��Ǵ� �Լ� �������̵�.
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Ŭ���̾�Ʈ���� ������ �ڷ���Ʈ ����� ���� �� ����� ����.
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	// Ŭ���̾�Ʈ���� �����ؼ� ������ �÷��׸� �������� ���� �� ����Ǵ� �Լ�.
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

public:

	// �ڷ���Ʈ �Է��� ���ȴ����� Ȯ���ϴ� ����.
	uint8 bPressedTeleport : 1;

	// �ڷ���Ʈ ��Ÿ�� ��� ����� ����.
	uint8 bDidTeleport : 1;

protected:

	// �ڷ���Ʈ �̵� �Ÿ�.
	UPROPERTY()
	float TeleportOffset;

	// �ڷ���Ʈ ��Ÿ��.
	UPROPERTY()
	float TeleportCooltime;
};