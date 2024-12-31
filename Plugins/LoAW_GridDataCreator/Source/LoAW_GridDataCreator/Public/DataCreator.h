// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataCreator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(DataCreator, Log, All);

UCLASS()
class LOAW_GRIDDATACREATOR_API ADataCreator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADataCreator();

	virtual void CreateData()
	{
	}

protected:
	//Delimiter
	FString PipeDelim = FString(TEXT("|"));
	FString CommaDelim = FString(TEXT(","));
	FString SpaceDelim = FString(TEXT(" "));
	FString ColonDelim = FString(TEXT(":"));

protected:
	bool CreateFilePath(const FString& RelPath, FString& FullPath);
	void WritePipeDelimiter(std::ofstream& ofs);
	void WriteColonDelimiter(std::ofstream& ofs);
	void WriteLineEnd(std::ofstream& ofs);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
