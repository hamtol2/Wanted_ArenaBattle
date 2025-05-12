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

	// 네트워크로 복제될 속성을 추가하기 위한 함수 오버라이딩.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 액터 채널이 열릴 때 호출되는 함수.
	virtual void OnActorChannelOpen(class FInBunch& InBunch, class UNetConnection* Connection) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<class UStaticMeshComponent> Water;

	// ServerRotationYaw 속성이 변경됐을 때 호출할 콜백 함수.
	UFUNCTION()
	void OnRep_ServerRotationYaw();

	// 리플리케이션 옵션 지정.
	UPROPERTY(ReplicatedUsing = OnRep_ServerRotationYaw)
	float ServerRotationYaw;

	// 회전 속도.
	float RotationRate = 30.0f;
};