// Fill out your copyright notice in the Description page of Project Settings.


#include "DataCreator.h"

// Sets default values
ADataCreator::ADataCreator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

}

// Called when the game starts or when spawned
void ADataCreator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADataCreator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

