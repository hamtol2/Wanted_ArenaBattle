// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABFountain.generated.h"

UCLASS()
class ARENABATTLE_API AABFountain : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AABFountain();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ��Ʈ��ũ�� ������ �Ӽ��� �߰��ϱ� ���� �Լ� �������̵�.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ���� ä���� ���� �� ȣ��Ǵ� �Լ�.
	virtual void OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection) override;

	// ������ ������ ������ �� ����ϴ� �Լ�.
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	// ������ ���� ����� �غ�Ǹ� ����Ǵ� �Լ�.
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Water;

	// ServerRotationYaw �Ӽ��� ������� �� ȣ���� �ݹ� �Լ�.
	UFUNCTION()
	void OnRep_ServerRotationYaw();

	UFUNCTION()
	void OnRep_ServerLightColor();

	// ���ø����̼� �ɼ� ����.
	UPROPERTY(ReplicatedUsing = OnRep_ServerRotationYaw)
	float ServerRotationYaw;

	//// �ǵ������� ��Ʈ��ũ�� ��ȭ(Saturation) ���·� ����� ���� ����.
	//UPROPERTY(Replicated)
	//TArray<float> BigData;

	//// ������ ���� �� ����.
	//float BigDataElement = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ServerLightColor)
	FLinearColor ServerLightColor;

	// ȸ�� �ӵ�.
	float RotationRate = 30.0f;

	// �����κ��� ��Ŷ(������)�� ���� �Ŀ�
	// �ð��� �󸶳� ����ߴ����� ����ϱ� ���� ����.
	float ClientTimeSinceUpdate = 0.0f;

	// �����κ��� �����͸� �ް�,
	// �� ���� �����͸� �޾��� ������ �ɸ� �ð��� ����ϱ� ���� ����.
	float ClientTimeBetweenLastUpdate = 0.0f;
};