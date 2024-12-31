// Fill out your copyright notice in the Description page of Project Settings.


#include "DataCreator.h"

#include <filesystem>
#include <fstream>

DEFINE_LOG_CATEGORY(DataCreator);

// Sets default values
ADataCreator::ADataCreator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

}

bool ADataCreator::CreateFilePath(const FString& RelPath, FString& FullPath)
{
	FullPath = FPaths::ProjectDir().Append(RelPath);
	FString Path = FPaths::GetPath(FullPath);
	if (!FPaths::DirectoryExists(Path)) {
		if (std::filesystem::create_directories(*Path)) {
			UE_LOG(DataCreator, Log, TEXT("Create directory %s success."), *Path);
			return true;
		}
	}
	return false;
}

void ADataCreator::WritePipeDelimiter(std::ofstream& ofs)
{
	ofs << TCHAR_TO_ANSI(*PipeDelim);
}

void ADataCreator::WriteColonDelimiter(std::ofstream& ofs)
{
	ofs << TCHAR_TO_ANSI(*ColonDelim);
}

void ADataCreator::WriteLineEnd(std::ofstream& ofs)
{
	ofs << std::endl;
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

