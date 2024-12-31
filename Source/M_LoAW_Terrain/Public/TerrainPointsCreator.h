// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "TerrainStructDefine.h"
#include "Quad.h"
#include "CoreMinimal.h"
#include "DataCreator.h"
#include "TerrainPointsCreator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(TerrainPointsCreator, Log, All);

UENUM(BlueprintType)
enum class Enum_TerrainPointsCreatorWorkflowState : uint8
{
	InitWorkflow,
	SpiralCreateCenter,
	SpiralCreateNeighbors,
	WritePoints,
	WritePointsNeighbor,
	WritePointIndices,
	WriteParams,
	Done,
	Error
};

#define TERRAIN_POINTS_RING_START_DIRECTION_INDEX	0

UCLASS(MinimalAPI)
class ATerrainPointsCreator : public ADataCreator
{
	GENERATED_BODY()

public:
	ATerrainPointsCreator();

	M_LOAW_TERRAIN_API virtual void CreateData() override;

private:
	//delegate
	FTimerDynamicDelegate WorkflowDelegate;

	TArray<FStructTerrainPointData> Points;
	TMap<FIntPoint, int32> PointIndices;

	//Flag for spiral ring
	bool RingInitFlag = false;

	int32 NeighborStep = 4;

	Quad TmpQuad;

protected:
	//Params
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params", meta = (ClampMin = "1"))
	int32 GridRange = 505;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params", meta = (ClampMin = "1"))
	int32 NeighborRange = 3;

	//Timer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Timer")
	float DefaultTimerRate = 0.01f;

	//Loop BP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData SpiralCreateCenterLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData SpiralCreateNeighborsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData WritePointsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData WriteNeighborsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData WritePointIndicesLoopData;

	//Workflow
	UPROPERTY(BlueprintReadWrite)
	Enum_TerrainPointsCreatorWorkflowState WorkflowState = Enum_TerrainPointsCreatorWorkflowState::Done;

	//Progress
	UPROPERTY(BlueprintReadOnly)
	int32 ProgressTarget = 0;
	UPROPERTY(BlueprintReadOnly)
	int32 ProgressCurrent = 0;

	//Path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	//FString DataFileRelPath = FString(TEXT("Plugins/LoAW_Asset/Content/Data/Terrain/"));
	FString DataFileRelPath = FString(TEXT("Data/Terrain/"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	FString PointsDataFileName = FString(TEXT("Points.data"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	FString PointIndicesDataFileName = FString(TEXT("PointIndices.data"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	FString ParamsDataFileName = FString(TEXT("Params.data"));


private:
	//Timer delegate
	void BindDelegate();

	//Workflow
	UFUNCTION(CallInEditor)
	void CreateTerrainPointsFlow();

	//Init workflow
	void InitWorkflow();
	void InitLoopData();

	void ResetProgress();

	//Create center
	void SpiralCreateCenter();
	void InitGridCenter();
	void AddRingPointAndIndex();
	void FindNeighborPointOfRing(int32 DirIndex);

	//Create neighbors
	void SpiralCreateNeighbors();
	void AddPointNeighbor(int32 PointIndex, int32 Radius);
	void SetPointNeighbor(int32 PointIndex, int32 Radius, int32 DirIndex);

	//Write points data to file
	void WritePointsToFile();
	void WritePoints(std::ofstream& ofs);
	void WritePointLine(std::ofstream& ofs, int32 Index);
	void WriteAxialCoord(std::ofstream& ofs, const FStructTerrainPointData& Data);

	//Write neighbors to file
	void WriteNeighborsToFile();
	int32 CalNeighborsWeight(int32 Range);
	void CreateNeighborPath(FString& NeighborPath, int32 Radius);
	bool WriteNeighbors(std::ofstream& ofs, int32 Radius);
	void WriteNeighborLine(std::ofstream& ofs, int32 Index, int32 Radius);

	//Write point indices data to file
	void WritePointIndicesToFile();
	void WritePointIndices(std::ofstream& ofs);
	void WritePointIndicesLine(std::ofstream& ofs, int32 Index);
	void WriteIndicesKey(std::ofstream& ofs, const FIntPoint& key);
	void WriteIndicesValue(std::ofstream& ofs, int32 Index);

	//Write info data to file
	void WriteParamsToFile();
	void WriteParams(std::ofstream& ofs);
	void WriteParamsContent(std::ofstream& ofs);
	
public:
	UFUNCTION(BlueprintCallable)
	void GetProgress(float& Out_Progress);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsInWorkingState()
	{
		return WorkflowState < Enum_TerrainPointsCreatorWorkflowState::Done;
	}

};
