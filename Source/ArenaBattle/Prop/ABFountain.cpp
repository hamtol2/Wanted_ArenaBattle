// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/ABFountain.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "ArenaBattle.h"

// Sets default values
AABFountain::AABFountain()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Water = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Water"));

	RootComponent = Body;
	Water->SetupAttachment(Body);
	Water->SetRelativeLocation(FVector(0.0f, 0.0f, 132.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Castle_Fountain_01.SM_Plains_Castle_Fountain_01'"));
	if (BodyMeshRef.Object)
	{
		Body->SetStaticMesh(BodyMeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterMeshRef(TEXT("/Script/Engine.StaticMesh'/Game/ArenaBattle/Environment/Props/SM_Plains_Fountain_02.SM_Plains_Fountain_02'"));
	if (WaterMeshRef.Object)
	{
		Water->SetStaticMesh(WaterMeshRef.Object);
	}

	// 리플리케이션 활성화를 위한 옵션 설정.
	bReplicates = true;

	// 네트워크 전송 빈도 설정 (1초에 1번으로 줄이기).
	NetUpdateFrequency = 1.0f;
}

// Called when the game starts or when spawned
void AABFountain::BeginPlay()
{
	Super::BeginPlay();
	
	// 서버 로직.
	//if (HasAuthority())
	//{
	//	FTimerHandle Handle;
	//	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]() 
	//		{
	//			ServerRotationYaw += 1.0f;
	//		}
	//	), 1.0f, true);
	//}
}

void AABFountain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 복제할 속성 정의 추가.
	DOREPLIFETIME(AABFountain, ServerRotationYaw);
}

void AABFountain::OnActorChannelOpen(
	FInBunch& InBunch, 
	UNetConnection* Connection)
{
	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("Begin"));

	Super::OnActorChannelOpen(InBunch, Connection);

	AB_LOG(LogABNetwork, Log, TEXT("%s"), TEXT("End"));
}

// Called every frame
void AABFountain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 서버인지 확인.
	if (HasAuthority())
	{
		// 서버의 액터가 회전.
		AddActorLocalRotation(FRotator(0.0f, RotationRate * DeltaTime, 0.0f));

		// 변경된 회전 값을 프로퍼티에 저장.
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}
	// 클라이언트.
	else
	{
		// 서버로부터 데이터를 받은 후에 경과한 시간 업데이트.
		ClientTimeSinceUpdate += DeltaTime;

		// KINDA_SMALL_NUMBER은 0에 근접한 값.
		// 보간 처리를 하는데 ClientTimeBetweenLastUpdate 값이 매우 작으면,
		// 보간에 의미가 없음.
		if (ClientTimeBetweenLastUpdate < KINDA_SMALL_NUMBER)
		{
			return;
		}

		// 다음 네트워크 패킷 때 수신할 회전 값을 예측.
		const float EstimateRotationYaw
			= ServerRotationYaw + RotationRate * ClientTimeBetweenLastUpdate;

		// 회전 값 구하기.
		FRotator ClientRotator = RootComponent->GetComponentRotation();

		// 비율 (Alpha, t 값).
		const float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;

		// 보간 값 구하기.
		const float ClientNewYaw = FMath::Lerp(
			ServerRotationYaw,
			EstimateRotationYaw,
			LerpRatio
		);

		ClientRotator.Yaw = ClientNewYaw;

		// 컴포넌트 회전에 적용.
		RootComponent->SetWorldRotation(ClientRotator);
	}
}

void AABFountain::OnRep_ServerRotationYaw()
{
	AB_LOG(LogABNetwork, Log, TEXT("Yaw: %f"), ServerRotationYaw);

	// 변경된 회전 값을 반영해 새 회전 값 생성.
	FRotator NewRotator = RootComponent->GetComponentRotation();
	NewRotator.Yaw = ServerRotationYaw;

	// 루트 컴포넌트 회전에 설정.
	RootComponent->SetWorldRotation(NewRotator);

	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;

	// 서버로부터 데이터를 받았기 때문에 0으로 초기화.
	ClientTimeSinceUpdate = 0.0f;
}