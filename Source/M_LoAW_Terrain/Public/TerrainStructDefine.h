#pragma once

#include "TerrainStructDefine.generated.h"

USTRUCT(BlueprintType)
struct FStructLoopData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 LoopCountLimit = 3000;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Rate = 0.01f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 LoopDepthLimit = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<int32> IndexSaved = {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool HasInitialized = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "0"))
	int32 Count = 0;

};

USTRUCT(BlueprintType)
struct FStructHeightMapping
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RangeMin = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RangeMax = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float MappingMin = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float MappingMax = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RangeMinOffset = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float RangeMaxOffset = 0.0;

};

USTRUCT(BlueprintType)
struct FStructTerrainPointNeighbors
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Radius = 0;

	UPROPERTY()
	int32 Count = 0;

	UPROPERTY()
	TArray<FIntPoint> Points;
};

USTRUCT(BlueprintType)
struct FStructTerrainPointData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FIntPoint AxialCoord = FIntPoint();

	UPROPERTY(BlueprintReadOnly)
	FVector2D Position2D = FVector2D();

	UPROPERTY(BlueprintReadOnly)
	float PositionZ = 0.0;

	UPROPERTY(BlueprintReadOnly)
	FVector Normal = FVector();

	UPROPERTY(BlueprintReadOnly)
	TArray<FStructTerrainPointNeighbors> Neighbors;

};

