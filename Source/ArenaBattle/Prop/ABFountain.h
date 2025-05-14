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

	// 연관성 판정을 진행할 때 사용하는 함수.
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	// 전송할 액터 목록이 준비되면 실행되는 함수.
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

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

	UFUNCTION()
	void OnRep_ServerLightColor();

	// 리플리케이션 옵션 지정.
	UPROPERTY(ReplicatedUsing = OnRep_ServerRotationYaw)
	float ServerRotationYaw;

	//// 의도적으로 네트워크를 포화(Saturation) 상태로 만들기 위한 변수.
	//UPROPERTY(Replicated)
	//TArray<float> BigData;

	//// 데이터 변경 용 변수.
	//float BigDataElement = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ServerLightColor)
	FLinearColor ServerLightColor;

	// 회전 속도.
	float RotationRate = 30.0f;

	// 서버로부터 패킷(데이터)을 받은 후에
	// 시간이 얼마나 경과했는지를 기록하기 위한 변수.
	float ClientTimeSinceUpdate = 0.0f;

	// 서버로부터 데이터를 받고,
	// 그 다음 데이터를 받았을 때까지 걸린 시간을 기록하기 위한 변수.
	float ClientTimeBetweenLastUpdate = 0.0f;
};