// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TerrainStructDefine.h"
#include "HexGridStructDefine.h"
#include "CoreMinimal.h"
#include "DataCreator.h"
#include "HexGridCreator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(HexGridCreator, Log, All);

UENUM(BlueprintType)
enum class Enum_HexGridCreatorWorkflowState : uint8
{
	InitWorkflow,
	SpiralCreateCenter,
	SpiralCreateNeighbors,
	WriteTiles,
	WriteTilesNeighbor,
	WriteTileIndices,
	WriteParams,
	Done,
	Error
};

#define RING_START_DIRECTION_INDEX	4

UCLASS(MinimalAPI)
class AHexGridCreator : public ADataCreator
{
	GENERATED_BODY()

public:
	AHexGridCreator();

	M_LOAW_HEXGRID_API virtual void CreateData() override;

private:
	FVector QDirection;
	FVector RDirection;
	FVector SDirection;

	TArray<FVector2D> NeighborDirVectors;

	float TileWidth;
	float TileHeight;

	//delegate
	FTimerDynamicDelegate WorkflowDelegate;

	TArray<FStructHexTileData> Tiles;
	TMap<FIntPoint, int32> TileIndices;

	//Flag for spiral ring
	bool RingInitFlag = false;

	TArray<FIntPoint> AxialDirectionVectors;

	//Save temp data for SpiralCreateCenter and SpiralCreateNeighbors
	FVector2D TmpPosition2D;
	FIntPoint TmpHex;

	//Temp data for create vertices
	TArray<FVector> OuterVectors;
	TArray<FVector> InnerVectors;

	//Temp data for create triangles
	TArray<int32> TriArr0, TriArr1, TriArr2, TriArr3;

protected:
	//Params
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params", meta = (ClampMin = "0.0"))
	float TileSize = 500.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params", meta = (ClampMin = "1"))
	int32 GridRange = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Params", meta = (ClampMin = "1"))
	int32 NeighborRange = 4;

	//Path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	//FString DataFileRelPath = FString(TEXT("Plugins/LoAW_Asset/Content/Data/HexGrid/"));
	FString DataFileRelPath = FString(TEXT("Data/HexGrid/"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	FString TilesDataFileName = FString(TEXT("Tiles.data"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	FString TileIndicesDataFileName = FString(TEXT("TileIndices.data"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Path")
	FString ParamsDataFileName = FString(TEXT("Params.data"));

	//Timer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Timer")
	float DefaultTimerRate = 0.01f;

	//Loop BP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData SpiralCreateCenterLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData SpiralCreateNeighborsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData WriteTilesLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData WriteNeighborsLoopData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom|Loop")
	struct FStructLoopData WriteTileIndicesLoopData;

	//Workflow
	UPROPERTY(BlueprintReadWrite)
	Enum_HexGridCreatorWorkflowState WorkflowState = Enum_HexGridCreatorWorkflowState::Done;

	//Progress
	UPROPERTY(BlueprintReadOnly)
	int32 ProgressTarget = 0;
	UPROPERTY(BlueprintReadOnly)
	int32 ProgressCurrent = 0;

private:
	//Timer delegate
	void BindDelegate();

	//Workflow
	UFUNCTION(CallInEditor)
	void CreateHexGridFlow();

	//Init workflow
	void InitWorkflow();
	void InitDirection();
	void InitTileParams();
	void InitLoopData();
	void InitAxialDirections();

	//Hex axial coordinate function
	FIntPoint AxialAdd(const FIntPoint& Hex, const FIntPoint& Vec);
	FIntPoint AxialDirection(const int32 Direction);
	FIntPoint AxialNeighbor(const FIntPoint& Hex, const int32 Direction);
	FIntPoint AxialScale(const FIntPoint& Hex, const int32 Factor);

	void ResetProgress();

	//Create center
	void SpiralCreateCenter();
	void InitGridCenter();
	void AddRingTileAndIndex();
	void FindNeighborTileOfRing(int32 DirIndex);

	//Create neighbors
	void SpiralCreateNeighbors();
	void AddTileNeighbor(int32 TileIndex, int32 Radius);
	void SetTileNeighbor(int32 TileIndex, int32 Radius, int32 DirIndex);

	//Write hex tiles data to file
	void WriteTilesToFile();
	void WriteTiles(std::ofstream& ofs);
	void WriteTileLine(std::ofstream& ofs, int32 Index);
	void WriteIndices(std::ofstream& ofs, int32 Index);
	void WriteAxialCoord(std::ofstream& ofs, const FStructHexTileData& Data);
	void WritePosition2D(std::ofstream& ofs, const FStructHexTileData& Data);

	//Write neighbors to file
	void WriteNeighborsToFile();
	void CreateNeighborPath(FString& NeighborPath, int32 Radius);
	int32 CalNeighborsWeight(int32 Range);
	bool WriteNeighbors(std::ofstream& ofs, int32 Radius);
	void WriteNeighborLine(std::ofstream& ofs, int32 Index, int32 Radius);

	//Write tile indices data to file
	void WriteTileIndicesToFile();
	void WriteTileIndices(std::ofstream& ofs);
	void WriteTileIndicesLine(std::ofstream& ofs, int32 Index);
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
		return WorkflowState < Enum_HexGridCreatorWorkflowState::Done;
	}

	M_LOAW_HEXGRID_API FString FloatToString(double value, int32 MaximumFractionalDigits);

};
