// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ABCharacterBase.h"
#include "InputActionValue.h"
#include "Interface/ABCharacterHUDInterface.h"
#include "ABCharacterPlayer.generated.h"

/**
 * 
 */
UCLASS()
class ARENABATTLE_API AABCharacterPlayer : public AABCharacterBase, public IABCharacterHUDInterface
{
	GENERATED_BODY()
	
public:
	AABCharacterPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void SetDead() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Owner() override;
	virtual void PostNetInit() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// Character Control Section
protected:
	void ChangeCharacterControl();
	void SetCharacterControl(ECharacterControlType NewCharacterControlType);
	virtual void SetCharacterControlData(const class UABCharacterControlData* CharacterControlData) override;

// Camera Section
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> FollowCamera;

// Input Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ChangeControlAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> ShoulderLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> QuaterMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UInputAction> AttackAction;

	void ShoulderMove(const FInputActionValue& Value);
	void ShoulderLook(const FInputActionValue& Value);

	void QuaterMove(const FInputActionValue& Value);

	ECharacterControlType CurrentCharacterControlType;

	// ������Ƽ ���ø����̼� ����� ���� �Լ� �������̵�.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Attack();

	// ���� �ִϸ��̼��� ����ϴ� �Լ�.
	void PlayAttackAnimation();

	// AttackHitCheck �Լ� �������̵�.
	virtual void AttackHitCheck() override;

	// ���� ���� Ȯ�� �Լ�.
	void AttackHitConfirm(AActor* HitActor);

	// Debug Draw �Լ�.
	void DrawDebugAttackRange(
		const FColor& DrawColor,
		FVector TraceStart,
		FVector TraceEnd,
		FVector Forward
	);

	// ���� ��� ó���� ���� Server RPC ����.
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerRPCAttack(float AttackStartTime);

	// Ŭ���̾�Ʈ�� ���� ��� ������ ���� ��Ƽĳ��Ʈ RPC ����.
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCAttack();

	// ���� �ִϸ��̼��� ��� ��û�� �� ����� Client RPC �Լ�.
	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayAnimation(AABCharacterPlayer* CharacterToPlay);


	// Ŭ���̾�Ʈ���� ���Ͱ� �¾��� �� ������ ������ ������ �Լ�.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyHit(const FHitResult& HitResult, float HitCheckTime);

	// Ŭ���̾�Ʈ���� �浹 ���� �� �̽��� �߻����� �� ������ �Լ�.
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCNotifyMiss(
		FVector_NetQuantize TraceStart, 
		FVector_NetQuantize TraceEnd,
		FVector_NetQuantizeNormal TraceDir,
		float HitCheckTime
	);

	UFUNCTION()
	void OnRep_CanAttack();

	// ���� ���� �������� �Ǵ��� �� ����� ����.
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bCanAttack : 1;

	// ù ��° ���� �ִϸ��̼��� ��� ���� ��.
	// Ÿ�̸Ӹ� ����ؼ� �� �ð��� ������, ������ �����ϵ��� ó��.
	float AttackTime = 1.4667f;

	// �ð� ���� ����.
	// Ŭ���̾�Ʈ�� ������ �ð�(������ ��û��)�� ����ϱ� ���� ����.
	float LastAttackStartTime = 0.0f;

	// Ŭ���̾�Ʈ�� ������ �ð� ���� ����ϱ� ���� ����.
	float AttackTimeDifference = 0.0f;

	// ���� ������ ����� �Ÿ� ��.
	float AttackCheckDistance = 300.0f;

	// ������ ������ �Ŀ� ������ �ð��� ������ ������ �����ϴٰ� �Ǵ��� ���� ��.
	float AcceptMinCheckTime = 0.15f;

// UI Section
protected:
	virtual void SetupHUDWidget(class UABHUDWidget* InHUDWidget) override;
};
