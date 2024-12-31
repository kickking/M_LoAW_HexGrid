// Fill out your copyright notice in the Description page of Project Settings.

#include "HexGridCreator.h"
#include "FlowControlUtility.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <Kismet/KismetTextLibrary.h>

DEFINE_LOG_CATEGORY(HexGridCreator);

AHexGridCreator::AHexGridCreator()
{
}

void AHexGridCreator::CreateData()
{
	if (IsInWorkingState()) return;

	UE_LOG(HexGridCreator, Log, TEXT("AHexGridCreator::CreateData()."));
	BindDelegate();
	WorkflowState = Enum_HexGridCreatorWorkflowState::InitWorkflow;
	CreateHexGridFlow();
}

void AHexGridCreator::BindDelegate()
{
	WorkflowDelegate.BindUFunction(Cast<UObject>(this), TEXT("CreateHexGridFlow"));
}

void AHexGridCreator::CreateHexGridFlow()
{
	switch (WorkflowState)
	{
	case Enum_HexGridCreatorWorkflowState::InitWorkflow:
		InitWorkflow();
		break;
	case Enum_HexGridCreatorWorkflowState::SpiralCreateCenter:
		SpiralCreateCenter();
		break;
	case Enum_HexGridCreatorWorkflowState::SpiralCreateNeighbors:
		SpiralCreateNeighbors();
		break;
	case Enum_HexGridCreatorWorkflowState::WriteTiles:
		WriteTilesToFile();
		break;
	case Enum_HexGridCreatorWorkflowState::WriteTilesNeighbor:
		WriteNeighborsToFile();
		break;
	case Enum_HexGridCreatorWorkflowState::WriteTileIndices:
		WriteTileIndicesToFile();
		break;
	case Enum_HexGridCreatorWorkflowState::WriteParams:
		WriteParamsToFile();
		break;
	case Enum_HexGridCreatorWorkflowState::Done:
		UE_LOG(HexGridCreator, Log, TEXT("Create HexGrid data done."));
		break;
	case Enum_HexGridCreatorWorkflowState::Error:
		UE_LOG(HexGridCreator, Warning, TEXT("CreateHexGridFlow Error!"));
		break;
	default:
		break;
	}
}

void AHexGridCreator::InitWorkflow()
{
	InitDirection();
	InitTileParams();
	InitLoopData();
	InitAxialDirections();

	FTimerHandle TimerHandle;
	WorkflowState = Enum_HexGridCreatorWorkflowState::SpiralCreateCenter;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(HexGridCreator, Log, TEXT("Init workflow done."));
}

void AHexGridCreator::InitDirection()
{
	//Init Q,R,S
	QDirection.Set(1.0, 0.0, 0.0);
	FVector ZAxis(0.0, 0.0, 1.0);
	FVector Vec = QDirection.RotateAngleAxis(120.0, ZAxis);
	RDirection.Set(Vec.X, Vec.Y, Vec.Z);
	Vec = RDirection.RotateAngleAxis(120.0, ZAxis);
	SDirection.Set(Vec.X, Vec.Y, Vec.Z);

	//Init Neighbor Directions
	Vec.Set(1.0, 0.0, 0.0);
	Vec = Vec.RotateAngleAxis(30.0, ZAxis);
	float Angle;
	FVector NDir;
	for (int32 i = 0; i <= 5; i++)
	{
		Angle = i * (-60.0);
		NDir = Vec.RotateAngleAxis(Angle, ZAxis);
		NeighborDirVectors.Add(FVector2D(NDir.X, NDir.Y));
	}
}

void AHexGridCreator::InitTileParams()
{
	TileWidth = TileSize * 2.0;
	TileHeight = TileSize * FMath::Sqrt(3.0);

}

void AHexGridCreator::InitLoopData()
{
	FlowControlUtility::InitLoopData(SpiralCreateCenterLoopData);
	SpiralCreateCenterLoopData.IndexSaved[0] = 1;
	FlowControlUtility::InitLoopData(SpiralCreateNeighborsLoopData);
	SpiralCreateNeighborsLoopData.IndexSaved[1] = 1;
	FlowControlUtility::InitLoopData(WriteTilesLoopData);
	FlowControlUtility::InitLoopData(WriteNeighborsLoopData);
	WriteNeighborsLoopData.IndexSaved[0] = 1;
	FlowControlUtility::InitLoopData(WriteTileIndicesLoopData);
}

void AHexGridCreator::InitAxialDirections()
{
	AxialDirectionVectors.Add(FIntPoint(1, 0));
	AxialDirectionVectors.Add(FIntPoint(1, -1));
	AxialDirectionVectors.Add(FIntPoint(0, -1));
	AxialDirectionVectors.Add(FIntPoint(-1, 0));
	AxialDirectionVectors.Add(FIntPoint(-1, 1));
	AxialDirectionVectors.Add(FIntPoint(0, 1));
}

FIntPoint AHexGridCreator::AxialAdd(const FIntPoint& Hex, const FIntPoint& Vec)
{
	return Hex + Vec;
}

FIntPoint AHexGridCreator::AxialDirection(const int32 Direction)
{
	return AxialDirectionVectors[Direction];
}

FIntPoint AHexGridCreator::AxialNeighbor(const FIntPoint& Hex, const int32 Direction)
{
	return AxialAdd(Hex, AxialDirection(Direction));
}

FIntPoint AHexGridCreator::AxialScale(const FIntPoint& Hex, const int32 Factor)
{
	return Hex * Factor;
}

void AHexGridCreator::ResetProgress()
{
	ProgressTarget = 0;
	ProgressCurrent = 0;
}

void AHexGridCreator::GetProgress(float& Out_Progress)
{
	float Rate;
	if (ProgressTarget == 0) {
		Out_Progress = 0;
	}
	else {
		Rate = float(ProgressCurrent) / float(ProgressTarget);
		Rate = Rate > 1.0 ? 1.0 : Rate;
		Out_Progress = Rate;
	}
}

M_LOAW_HEXGRID_API FString AHexGridCreator::FloatToString(double value, int32 MaximumFractionalDigits)
{
	FNumberFormattingOptions NumberFormatOptions;
	NumberFormatOptions.AlwaysSign = false;
	NumberFormatOptions.UseGrouping = false;
	NumberFormatOptions.RoundingMode = ERoundingMode::HalfFromZero;
	NumberFormatOptions.MinimumIntegralDigits = 1;
	NumberFormatOptions.MaximumIntegralDigits = 324;
	NumberFormatOptions.MinimumFractionalDigits = 0;
	NumberFormatOptions.MaximumFractionalDigits = MaximumFractionalDigits;

	FText::AsNumber(value, &NumberFormatOptions);
	return FText::AsNumber(value, &NumberFormatOptions).ToString();
}

void AHexGridCreator::SpiralCreateCenter()
{
	bool OnceLoop0 = true;
	bool OnceLoop1 = true;
	int32 Count = 0;
	TArray<int32> Indices = { 0, 0, 0 };
	bool SaveLoopFlag = false;

	if (!SpiralCreateCenterLoopData.HasInitialized) {
		SpiralCreateCenterLoopData.HasInitialized = true;
		RingInitFlag = false;
		InitGridCenter();
		ProgressTarget = 6 * (1 + GridRange) * GridRange / 2;
	}

	int32 i = SpiralCreateCenterLoopData.IndexSaved[0];
	i = i < 1 ? 1 : i;
	int32 j, k;

	for (; i <= GridRange; i++)
	{
		Indices[0] = i;
		if (!RingInitFlag) {
			//Init position2D
			RingInitFlag = true;
			FVector2D Vec = i * TileHeight * NeighborDirVectors[RING_START_DIRECTION_INDEX];
			TmpPosition2D.Set(Vec.X, Vec.Y);

			//Init hex Axial
			FIntPoint Point = AxialAdd(AxialScale(AxialDirection(RING_START_DIRECTION_INDEX), i), FIntPoint(0, 0));
			TmpHex.X = Point.X;
			TmpHex.Y = Point.Y;
		}

		j = OnceLoop0 ? SpiralCreateCenterLoopData.IndexSaved[1] : 0;
		for (; j <= 5; j++) {
			Indices[1] = j;
			k = OnceLoop1 ? SpiralCreateCenterLoopData.IndexSaved[2] : 0;
			for (; k <= i - 1; k++)
			{
				Indices[2] = k;
				FlowControlUtility::SaveLoopData(this, SpiralCreateCenterLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
				if (SaveLoopFlag) {
					return;
				}

				AddRingTileAndIndex();
				FindNeighborTileOfRing(j);

				ProgressCurrent = SpiralCreateCenterLoopData.Count;
				Count++;
			}
			OnceLoop1 = false;
		}
		RingInitFlag = false;
		OnceLoop0 = false;
	}

	ResetProgress();

	FTimerHandle TimerHandle;
	WorkflowState = Enum_HexGridCreatorWorkflowState::SpiralCreateNeighbors;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, SpiralCreateCenterLoopData.Rate, false);
	UE_LOG(HexGridCreator, Log, TEXT("Spiral create center done."));
}

void AHexGridCreator::InitGridCenter()
{
	FStructHexTileData Data;
	Data.AxialCoord.X = 0;
	Data.AxialCoord.Y = 0;
	Data.Position2D.Set(0.0, 0.0);
	Tiles.Empty();
	Tiles.Add(Data);

	TileIndices.Empty();
	TileIndices.Add(FIntPoint(0, 0), 0);

}

void AHexGridCreator::AddRingTileAndIndex()
{
	FStructHexTileData Data;
	Data.AxialCoord.X = TmpHex.X;
	Data.AxialCoord.Y = TmpHex.Y;
	Data.Position2D.Set(TmpPosition2D.X, TmpPosition2D.Y);
	int32 Index = Tiles.Add(Data);

	TileIndices.Add(FIntPoint(TmpHex.X, TmpHex.Y), Index);
}

void AHexGridCreator::FindNeighborTileOfRing(int32 DirIndex)
{
	FVector2D Pos = NeighborDirVectors[DirIndex] * TileHeight + TmpPosition2D;
	TmpPosition2D.Set(Pos.X, Pos.Y);

	FIntPoint Hex = AxialNeighbor(TmpHex, DirIndex);
	TmpHex.X = Hex.X;
	TmpHex.Y = Hex.Y;
}

void AHexGridCreator::SpiralCreateNeighbors()
{
	bool OnceLoop0 = true;
	bool OnceLoop1 = true;
	bool OnceLoop2 = true;
	int32 Count = 0;
	TArray<int32> Indices = { 0, 0, 0, 0 };
	bool SaveLoopFlag = false;

	if (!SpiralCreateNeighborsLoopData.HasInitialized) {
		SpiralCreateNeighborsLoopData.HasInitialized = true;
		RingInitFlag = false;
		ProgressTarget = Tiles.Num() * 6 * (1 + NeighborRange) * NeighborRange / 2;
	}

	int32 TileIndex = SpiralCreateNeighborsLoopData.IndexSaved[0];
	int32 i, j, k;

	FIntPoint center;

	for (; TileIndex < Tiles.Num(); TileIndex++)
	{
		Indices[0] = TileIndex;
		center = Tiles[TileIndex].AxialCoord;
		i = OnceLoop0 ? SpiralCreateNeighborsLoopData.IndexSaved[1] : 1;
		i = i < 1 ? 1 : i;
		for (; i <= NeighborRange; i++)
		{
			Indices[1] = i;
			if (!RingInitFlag) {
				RingInitFlag = true;

				AddTileNeighbor(TileIndex, i);

				FIntPoint Point = AxialAdd(AxialScale(AxialDirection(RING_START_DIRECTION_INDEX), i), center);
				TmpHex.X = Point.X;
				TmpHex.Y = Point.Y;
			}

			j = OnceLoop1 ? SpiralCreateNeighborsLoopData.IndexSaved[2] : 0;
			for (; j <= 5; j++) {
				Indices[2] = j;
				k = OnceLoop2 ? SpiralCreateNeighborsLoopData.IndexSaved[3] : 0;
				for (; k <= i - 1; k++) {
					Indices[3] = k;
					FlowControlUtility::SaveLoopData(this, SpiralCreateNeighborsLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
					if (SaveLoopFlag) {
						return;
					}

					SetTileNeighbor(TileIndex, i, j);
					ProgressCurrent = SpiralCreateNeighborsLoopData.Count;
					Count++;
				}
				OnceLoop2 = false;
			}
			RingInitFlag = false;
			OnceLoop1 = false;
		}
		OnceLoop0 = false;
	}

	ResetProgress();

	FTimerHandle TimerHandle;
	WorkflowState = Enum_HexGridCreatorWorkflowState::WriteTiles;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, SpiralCreateNeighborsLoopData.Rate, false);
	UE_LOG(HexGridCreator, Log, TEXT("Spiral create neighbors done."));
}

void AHexGridCreator::AddTileNeighbor(int32 TileIndex, int32 Radius)
{
	FStructHexTileNeighbors neighbors;
	neighbors.Radius = Radius;
	Tiles[TileIndex].Neighbors.Add(neighbors);
}

void AHexGridCreator::SetTileNeighbor(int32 TileIndex, int32 Radius, int32 DirIndex)
{
	Tiles[TileIndex].Neighbors[Radius - 1].Tiles.Add(FIntPoint(TmpHex.X, TmpHex.Y));

	FIntPoint Hex = AxialNeighbor(TmpHex, DirIndex);
	TmpHex.X = Hex.X;
	TmpHex.Y = Hex.Y;
}

void AHexGridCreator::WriteTilesToFile()
{
	FString FullPath;
	FString TilesDataPath = FString(TEXT(""));
	TilesDataPath.Append(DataFileRelPath).Append(TilesDataFileName);
	CreateFilePath(TilesDataPath, FullPath);

	FTimerHandle TimerHandle;
	std::ofstream ofs;
	if (!WriteTilesLoopData.HasInitialized) {
		WriteTilesLoopData.HasInitialized = true;
		ofs.open(*FullPath, std::ios::out | std::ios::trunc);
		ProgressTarget = Tiles.Num();
	}
	else {
		ofs.open(*FullPath, std::ios::out | std::ios::app);
	}

	if (!ofs || !ofs.is_open()) {
		UE_LOG(HexGridCreator, Warning, TEXT("Open file %s failed!"), *FullPath);
		WorkflowState = Enum_HexGridCreatorWorkflowState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteTilesLoopData.Rate, false);
		return;
	}

	WriteTiles(ofs);
}

void AHexGridCreator::WriteTiles(std::ofstream& ofs)
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	int32 i = WriteTilesLoopData.IndexSaved[0];
	for (; i <= Tiles.Num() - 1; i++)
	{
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, WriteTilesLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			ofs.close();
			return;
		}
		WriteTileLine(ofs, i);
		ProgressCurrent = WriteTilesLoopData.Count;
		Count++;
	}
	ofs.close();
	FTimerHandle TimerHandle;
	WorkflowState = Enum_HexGridCreatorWorkflowState::WriteTilesNeighbor;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteTilesLoopData.Rate, false);
	UE_LOG(HexGridCreator, Log, TEXT("Write tiles done."));

}

void AHexGridCreator::WriteTileLine(std::ofstream& ofs, int32 Index)
{
	FStructHexTileData Data = Tiles[Index];
	WriteAxialCoord(ofs, Data);
	WritePipeDelimiter(ofs);
	WritePosition2D(ofs, Data);
	WriteLineEnd(ofs);
}

void AHexGridCreator::WriteIndices(std::ofstream& ofs, int32 Index)
{
	FString Str = FString::FromInt(Index);
	ofs << TCHAR_TO_ANSI(*Str);
}

void AHexGridCreator::WriteAxialCoord(std::ofstream& ofs, const FStructHexTileData& Data)
{
	FIntPoint AC = Data.AxialCoord;
	FString Str = FString::FromInt(AC.X);
	Str.Append(*CommaDelim);
	Str.Append(FString::FromInt(AC.Y));
	ofs << TCHAR_TO_ANSI(*Str);
}

void AHexGridCreator::WritePosition2D(std::ofstream& ofs, const FStructHexTileData& Data)
{
	FVector2D Pos2D = Data.Position2D;
	/*FText Txt = UKismetTextLibrary::Conv_FloatToText(Pos2D.X, ERoundingMode::HalfFromZero, false, false, 1, 324, 0, 2);
	FString Str = UKismetTextLibrary::Conv_TextToString(Txt);*/
	FString Str = FloatToString(Pos2D.X, 2);
	Str.Append(*CommaDelim);
	/*Txt = UKismetTextLibrary::Conv_FloatToText(Pos2D.Y, ERoundingMode::HalfFromZero, false, false, 1, 324, 0, 2);
	Str.Append(UKismetTextLibrary::Conv_TextToString(Txt));*/
	Str.Append(FloatToString(Pos2D.Y, 2));
	ofs << TCHAR_TO_ANSI(*Str);
}

void AHexGridCreator::WriteNeighborsToFile()
{
	int32 i = WriteNeighborsLoopData.IndexSaved[0];
	FTimerHandle TimerHandle;
	std::ofstream ofs;
	ProgressTarget = Tiles.Num() * CalNeighborsWeight(NeighborRange);

	for (; i <= NeighborRange; i++)
	{
		FString NeighborPath;
		FString FullPath;
		CreateNeighborPath(NeighborPath, i);
		CreateFilePath(NeighborPath, FullPath);

		if (!WriteNeighborsLoopData.HasInitialized) {
			WriteNeighborsLoopData.HasInitialized = true;
			ofs.open(*FullPath, std::ios::out | std::ios::trunc);
		}
		else {
			ofs.open(*FullPath, std::ios::out | std::ios::app);
		}

		if (!ofs || !ofs.is_open()) {
			UE_LOG(HexGridCreator, Warning, TEXT("Open file %s failed!"), *FullPath);
			WorkflowState = Enum_HexGridCreatorWorkflowState::Error;
			GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteNeighborsLoopData.Rate, false);
			return;
		}
		if (!WriteNeighbors(ofs, i)) {
			return;
		}
	}

	WorkflowState = Enum_HexGridCreatorWorkflowState::WriteTileIndices;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteNeighborsLoopData.Rate, false);
	UE_LOG(HexGridCreator, Log, TEXT("Write neighbors done."));
}

void AHexGridCreator::CreateNeighborPath(FString& NeighborPath, int32 Radius)
{
	FString TilesNeighborPathPrefix = FString(TEXT(""));
	TilesNeighborPathPrefix.Append(DataFileRelPath).Append(TEXT("N"));
	NeighborPath.Append(TilesNeighborPathPrefix).Append(FString::FromInt(Radius)).Append(FString(TEXT(".data")));
}

int32 AHexGridCreator::CalNeighborsWeight(int32 Range)
{
	int32 weight = 0;
	for (int32 i = 1; i <= Range; i++)
	{
		weight += i * 6;
	}
	return weight;
}

bool AHexGridCreator::WriteNeighbors(std::ofstream& ofs, int32 Radius)
{
	int32 Count = 0;
	TArray<int32> Indices = { Radius, 0 };
	bool SaveLoopFlag = false;

	int32 ProgressPre = Tiles.Num() * CalNeighborsWeight(Radius - 1);
	int32 ProgressRatio = Radius * 6;
	int32 i = WriteNeighborsLoopData.IndexSaved[1];
	for (; i <= Tiles.Num() - 1; i++)
	{
		Indices[1] = i;
		FlowControlUtility::SaveLoopData(this, WriteNeighborsLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			ofs.close();
			return false;
		}
		WriteNeighborLine(ofs, i, Radius);
		ProgressCurrent = ProgressPre + WriteNeighborsLoopData.Count * ProgressRatio;
		Count++;
	}

	ofs.close();
	FlowControlUtility::InitLoopData(WriteNeighborsLoopData);
	WriteNeighborsLoopData.IndexSaved[0] = Radius;
	UE_LOG(HexGridCreator, Log, TEXT("Write neighbor N%d done."), Radius);
	return true;
}

void AHexGridCreator::WriteNeighborLine(std::ofstream& ofs, int32 Index, int32 Radius)
{
	FStructHexTileNeighbors Neighbors = Tiles[Index].Neighbors[Radius - 1];
	for (int32 i = 0; i < Neighbors.Tiles.Num(); i++)
	{
		FString Str = FString::FromInt(Neighbors.Tiles[i].X);
		Str.Append(*CommaDelim);
		Str.Append(FString::FromInt(Neighbors.Tiles[i].Y));
		if (i != Neighbors.Tiles.Num() - 1) {
			Str.Append(*SpaceDelim);
		}
		ofs << TCHAR_TO_ANSI(*Str);
	}
	WriteLineEnd(ofs);
}

void AHexGridCreator::WriteTileIndicesToFile()
{
	FString FullPath;
	FString TileIndicesDataPath = FString(TEXT(""));
	TileIndicesDataPath.Append(DataFileRelPath).Append(TileIndicesDataFileName);
	CreateFilePath(TileIndicesDataPath, FullPath);

	FTimerHandle TimerHandle;
	std::ofstream ofs;
	if (!WriteTileIndicesLoopData.HasInitialized) {
		WriteTileIndicesLoopData.HasInitialized = true;
		ofs.open(*FullPath, std::ios::out | std::ios::trunc);
		ProgressTarget = Tiles.Num();
	}
	else {
		ofs.open(*FullPath, std::ios::out | std::ios::app);
	}

	if (!ofs || !ofs.is_open()) {
		UE_LOG(HexGridCreator, Warning, TEXT("Open file %s failed!"), *FullPath);
		WorkflowState = Enum_HexGridCreatorWorkflowState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteTileIndicesLoopData.Rate, false);
		return;
	}

	WriteTileIndices(ofs);
}

void AHexGridCreator::WriteTileIndices(std::ofstream& ofs)
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	int32 i = WriteTileIndicesLoopData.IndexSaved[0];
	for (; i <= Tiles.Num() - 1; i++)
	{
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, WriteTileIndicesLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			ofs.close();
			return;
		}
		WriteTileIndicesLine(ofs, i);
		ProgressCurrent = WriteTileIndicesLoopData.Count;
		Count++;
	}
	ofs.close();
	FTimerHandle TimerHandle;
	WorkflowState = Enum_HexGridCreatorWorkflowState::WriteParams;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteTileIndicesLoopData.Rate, false);
	UE_LOG(HexGridCreator, Log, TEXT("Write tiles indices done."));
}

void AHexGridCreator::WriteTileIndicesLine(std::ofstream& ofs, int32 Index)
{
	FIntPoint key = Tiles[Index].AxialCoord;
	WriteIndicesKey(ofs, key);
	WritePipeDelimiter(ofs);
	WriteIndicesValue(ofs, Index);
	WriteLineEnd(ofs);
}

void AHexGridCreator::WriteIndicesKey(std::ofstream& ofs, const FIntPoint& key)
{
	FString Str = FString::FromInt(key.X);
	Str.Append(*CommaDelim);
	Str.Append(FString::FromInt(key.Y));
	ofs << TCHAR_TO_ANSI(*Str);
}

void AHexGridCreator::WriteIndicesValue(std::ofstream& ofs, int32 Index)
{
	FString Str = FString::FromInt(Index);
	ofs << TCHAR_TO_ANSI(*Str);
}

void AHexGridCreator::WriteParamsToFile()
{
	FString FullPath;
	FString ParamsDataPath = FString(TEXT(""));
	ParamsDataPath.Append(DataFileRelPath).Append(ParamsDataFileName);
	CreateFilePath(ParamsDataPath, FullPath);

	FTimerHandle TimerHandle;
	std::ofstream ofs;
	ofs.open(*FullPath, std::ios::out | std::ios::trunc);
	ProgressTarget = 1;

	if (!ofs || !ofs.is_open()) {
		UE_LOG(HexGridCreator, Warning, TEXT("Open file %s failed!"), *FullPath);
		WorkflowState = Enum_HexGridCreatorWorkflowState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		return;
	}

	WriteParams(ofs);
}

void AHexGridCreator::WriteParams(std::ofstream& ofs)
{
	WriteParamsContent(ofs);
	ProgressCurrent = 1;
	ofs.close();
	FTimerHandle TimerHandle;
	WorkflowState = Enum_HexGridCreatorWorkflowState::Done;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(HexGridCreator, Log, TEXT("Write params done."));
}

void AHexGridCreator::WriteParamsContent(std::ofstream& ofs)
{
	/*FText Txt = UKismetTextLibrary::Conv_FloatToText(TileSize, ERoundingMode::HalfFromZero, false, false, 1, 324, 0, 2);
	FString Str = UKismetTextLibrary::Conv_TextToString(Txt);*/
	FString Str = FloatToString(TileSize, 2);
	ofs << TCHAR_TO_ANSI(*Str);
	WritePipeDelimiter(ofs);
	Str = FString::FromInt(GridRange);
	ofs << TCHAR_TO_ANSI(*Str);
	WritePipeDelimiter(ofs);
	Str = FString::FromInt(NeighborRange);
	ofs << TCHAR_TO_ANSI(*Str);
	WriteLineEnd(ofs);
}
