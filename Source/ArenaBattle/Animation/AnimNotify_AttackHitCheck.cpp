// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify_AttackHitCheck.h"
#include "Interface/ABAnimationAttackInterface.h"
#include "ArenaBattle.h"

void UAnimNotify_AttackHitCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		IABAnimationAttackInterface* AttackPawn = Cast<IABAnimationAttackInterface>(MeshComp->GetOwner());
		if (AttackPawn)
		{
			UE_LOG(LogABNetwork, Log, TEXT("[%s]"), LOG_CALLINFO);

			AttackPawn->AttackHitCheck();
		}
	}
}
