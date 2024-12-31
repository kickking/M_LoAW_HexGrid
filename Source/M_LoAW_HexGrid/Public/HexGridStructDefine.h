#pragma once

#include "HexGridStructDefine.generated.h"

USTRUCT(BlueprintType)
struct FStructHexTileNeighbors
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Radius = 0;

	UPROPERTY()
	int32 Count = 0;

	UPROPERTY()
	TArray<FIntPoint> Tiles;
};

USTRUCT(BlueprintType)
struct FStructHexTileData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FIntPoint AxialCoord = FIntPoint();

	UPROPERTY(BlueprintReadOnly)
	FVector2D Position2D = FVector2D();

	UPROPERTY(BlueprintReadOnly)
	TArray<FVector2D> VerticesPostion2D = {};

	UPROPERTY(BlueprintReadOnly)
	float PositionZ = 0.0;

	UPROPERTY(BlueprintReadOnly)
	TArray<float> VerticesPositionZ;

	UPROPERTY(BlueprintReadOnly)
	float AvgPositionZ = 0.0;

	UPROPERTY(BlueprintReadOnly)
	FVector Normal = FVector();

	UPROPERTY(BlueprintReadOnly)
	float AngleToUp = 0.0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FStructHexTileNeighbors> Neighbors;

	UPROPERTY(BlueprintReadOnly)
	int32 TerrainAreaBlockLevel = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TerrainFlyingBlockLevel = 0;

	UPROPERTY(BlueprintReadOnly)
	bool TerrainIsLand = false;

	UPROPERTY(BlueprintReadOnly)
	int32 TerrainBuildingBlockLevel = 0;

	UPROPERTY(BlueprintReadOnly)
	bool TerrainAreaConnection = true;
};

