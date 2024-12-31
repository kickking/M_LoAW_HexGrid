// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainPointsCreator.h"
#include "FlowControlUtility.h"

//#include <iostream>
#include <fstream>

DEFINE_LOG_CATEGORY(TerrainPointsCreator);

ATerrainPointsCreator::ATerrainPointsCreator()
{
}

void ATerrainPointsCreator::CreateData()
{
	if (IsInWorkingState()) return;

	UE_LOG(TerrainPointsCreator, Log, TEXT("ATerrainPointsCreator::CreateData()."));
	BindDelegate();
	WorkflowState = Enum_TerrainPointsCreatorWorkflowState::InitWorkflow;
	CreateTerrainPointsFlow();
}

void ATerrainPointsCreator::BindDelegate()
{
	WorkflowDelegate.BindUFunction(Cast<UObject>(this), TEXT("CreateTerrainPointsFlow"));
}

void ATerrainPointsCreator::CreateTerrainPointsFlow()
{
	switch (WorkflowState)
	{
	case Enum_TerrainPointsCreatorWorkflowState::InitWorkflow:
		InitWorkflow();
		break;
	case Enum_TerrainPointsCreatorWorkflowState::SpiralCreateCenter:
		SpiralCreateCenter();
		break;
	case Enum_TerrainPointsCreatorWorkflowState::SpiralCreateNeighbors:
		SpiralCreateNeighbors();
		break;
	case Enum_TerrainPointsCreatorWorkflowState::WritePoints:
		WritePointsToFile();
		break;
	case Enum_TerrainPointsCreatorWorkflowState::WritePointsNeighbor:
		WriteNeighborsToFile();
		break;
	case Enum_TerrainPointsCreatorWorkflowState::WritePointIndices:
		WritePointIndicesToFile();
		break;
	case Enum_TerrainPointsCreatorWorkflowState::WriteParams:
		WriteParamsToFile();
		break;
	case Enum_TerrainPointsCreatorWorkflowState::Done:
		UE_LOG(TerrainPointsCreator, Log, TEXT("Create Terrain points done."));
		break;
	case Enum_TerrainPointsCreatorWorkflowState::Error:
		UE_LOG(TerrainPointsCreator, Warning, TEXT("CreateTerrainPointsFlow Error!"));
		break;
	default:
		break;
	}
}

void ATerrainPointsCreator::InitWorkflow()
{
	InitLoopData();

	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainPointsCreatorWorkflowState::SpiralCreateCenter;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainPointsCreator, Log, TEXT("Init workflow done."));
}

void ATerrainPointsCreator::InitLoopData()
{
	FlowControlUtility::InitLoopData(SpiralCreateCenterLoopData);
	SpiralCreateCenterLoopData.IndexSaved[0] = 1;
	FlowControlUtility::InitLoopData(SpiralCreateNeighborsLoopData);
	SpiralCreateNeighborsLoopData.IndexSaved[1] = 1;
	FlowControlUtility::InitLoopData(WritePointsLoopData);
	FlowControlUtility::InitLoopData(WriteNeighborsLoopData);
	WriteNeighborsLoopData.IndexSaved[0] = 1;
	FlowControlUtility::InitLoopData(WritePointIndicesLoopData);
}

void ATerrainPointsCreator::ResetProgress()
{
	ProgressTarget = 0;
	ProgressCurrent = 0;
}

void ATerrainPointsCreator::SpiralCreateCenter()
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
		ProgressTarget = NeighborStep * (1 + GridRange) * GridRange / 2;
	}

	int32 i = SpiralCreateCenterLoopData.IndexSaved[0];
	i = i < 1 ? 1 : i;
	int32 j, k;

	for (; i <= GridRange; i++)
	{
		Indices[0] = i;
		if (!RingInitFlag) {
			RingInitFlag = true;
			Quad quad = Quad::NeighborDirection(TERRAIN_POINTS_RING_START_DIRECTION_INDEX);
			quad = Quad::Scale(quad, i);
			TmpQuad = Quad::Add(quad, Quad(Points[0].AxialCoord));
		}

		j = OnceLoop0 ? SpiralCreateCenterLoopData.IndexSaved[1] : 0;
		for (; j < NeighborStep; j++) {
			Indices[1] = j;
			k = OnceLoop1 ? SpiralCreateCenterLoopData.IndexSaved[2] : 0;
			for (; k <= i - 1; k++) {
				Indices[2] = k;
				FlowControlUtility::SaveLoopData(this, SpiralCreateCenterLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
				if (SaveLoopFlag) {
					return;
				}
				AddRingPointAndIndex();
				FindNeighborPointOfRing(j);

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
	WorkflowState = Enum_TerrainPointsCreatorWorkflowState::SpiralCreateNeighbors;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, SpiralCreateCenterLoopData.Rate, false);
	UE_LOG(TerrainPointsCreator, Log, TEXT("Spiral create center done."));
}

void ATerrainPointsCreator::InitGridCenter()
{
	FStructTerrainPointData Data;
	Data.AxialCoord.X = 0;
	Data.AxialCoord.Y = 0;
	Points.Empty();
	Points.Add(Data);

	PointIndices.Empty();
	PointIndices.Add(Data.AxialCoord, 0);
}

void ATerrainPointsCreator::AddRingPointAndIndex()
{
	FStructTerrainPointData Data;
	Data.AxialCoord.X = TmpQuad.GetCoord().X;
	Data.AxialCoord.Y = TmpQuad.GetCoord().Y;
	int32 Index = Points.Add(Data);

	PointIndices.Add(FIntPoint(Data.AxialCoord.X, Data.AxialCoord.Y), Index);
}

void ATerrainPointsCreator::FindNeighborPointOfRing(int32 DirIndex)
{
	TmpQuad = Quad::Neighbor(TmpQuad, DirIndex);
}

void ATerrainPointsCreator::SpiralCreateNeighbors()
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
		ProgressTarget = Points.Num() * NeighborStep * (1 + NeighborRange) * NeighborRange / 2;
	}

	int32 PointIndex = SpiralCreateNeighborsLoopData.IndexSaved[0];
	int32 i, j, k;
	Quad center;

	for (; PointIndex < Points.Num(); PointIndex++)
	{
		Indices[0] = PointIndex;
		center.SetAxialInt(Points[PointIndex].AxialCoord);
		i = OnceLoop0 ? SpiralCreateNeighborsLoopData.IndexSaved[1] : 1;
		i = i < 1 ? 1 : i;
		for (; i <= NeighborRange; i++) {
			Indices[1] = i;
			if (!RingInitFlag) {
				RingInitFlag = true;
				AddPointNeighbor(PointIndex, i);

				Quad quad = Quad::NeighborDirection(TERRAIN_POINTS_RING_START_DIRECTION_INDEX);
				quad = Quad::Scale(quad, i);
				TmpQuad = Quad::Add(quad, center);
			}

			j = OnceLoop1 ? SpiralCreateNeighborsLoopData.IndexSaved[2] : 0;
			for (; j < NeighborStep; j++) {
				Indices[2] = j;
				k = OnceLoop2 ? SpiralCreateNeighborsLoopData.IndexSaved[3] : 0;
				for (; k <= i - 1; k++) {
					Indices[3] = k;
					FlowControlUtility::SaveLoopData(this, SpiralCreateNeighborsLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
					if (SaveLoopFlag) {
						return;
					}
					SetPointNeighbor(PointIndex, i, j);
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
	WorkflowState = Enum_TerrainPointsCreatorWorkflowState::WritePoints;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, SpiralCreateNeighborsLoopData.Rate, false);
	UE_LOG(TerrainPointsCreator, Log, TEXT("Spiral create neighbors done."));
}

void ATerrainPointsCreator::AddPointNeighbor(int32 PointIndex, int32 Radius)
{
	FStructTerrainPointNeighbors neighbors;
	neighbors.Radius = Radius;
	Points[PointIndex].Neighbors.Add(neighbors);
}

void ATerrainPointsCreator::SetPointNeighbor(int32 PointIndex, int32 Radius, int32 DirIndex)
{
	Points[PointIndex].Neighbors[Radius - 1].Points.Add(FIntPoint(TmpQuad.GetCoord().X, TmpQuad.GetCoord().Y));
	FindNeighborPointOfRing(DirIndex);
}

void ATerrainPointsCreator::WritePointsToFile()
{
	FString FullPath;
	FString PointsDataPath = FString(TEXT(""));
	PointsDataPath.Append(DataFileRelPath).Append(PointsDataFileName);
	CreateFilePath(PointsDataPath, FullPath);

	FTimerHandle TimerHandle;
	std::ofstream ofs;
	if (!WritePointsLoopData.HasInitialized) {
		WritePointsLoopData.HasInitialized = true;
		ofs.open(*FullPath, std::ios::out | std::ios::trunc);
		ProgressTarget = Points.Num();
	}
	else {
		ofs.open(*FullPath, std::ios::out | std::ios::app);
	}

	if (!ofs || !ofs.is_open()) {
		UE_LOG(TerrainPointsCreator, Warning, TEXT("Open file %s failed!"), *FullPath);
		WorkflowState = Enum_TerrainPointsCreatorWorkflowState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WritePointsLoopData.Rate, false);
		return;
	}

	WritePoints(ofs);
}

void ATerrainPointsCreator::WritePoints(std::ofstream& ofs)
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	int32 i = WritePointsLoopData.IndexSaved[0];
	for (; i <= Points.Num() - 1; i++)
	{
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, WritePointsLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			ofs.close();
			return;
		}
		WritePointLine(ofs, i);
		ProgressCurrent = WritePointsLoopData.Count;
		Count++;
	}
	ofs.close();
	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainPointsCreatorWorkflowState::WritePointsNeighbor;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WritePointsLoopData.Rate, false);
	UE_LOG(TerrainPointsCreator, Log, TEXT("Write points done."));
}

void ATerrainPointsCreator::WritePointLine(std::ofstream& ofs, int32 Index)
{
	FStructTerrainPointData Data = Points[Index];
	WriteAxialCoord(ofs, Data);
	WriteLineEnd(ofs);
}

void ATerrainPointsCreator::WriteAxialCoord(std::ofstream& ofs, const FStructTerrainPointData& Data)
{
	FIntPoint AC = Data.AxialCoord;
	FString Str = FString::FromInt(AC.X);
	Str.Append(*CommaDelim);
	Str.Append(FString::FromInt(AC.Y));
	ofs << TCHAR_TO_ANSI(*Str);
}

void ATerrainPointsCreator::WriteNeighborsToFile()
{
	int32 i = WriteNeighborsLoopData.IndexSaved[0];
	FTimerHandle TimerHandle;
	std::ofstream ofs;
	ProgressTarget = Points.Num() * CalNeighborsWeight(NeighborRange);

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
			UE_LOG(TerrainPointsCreator, Warning, TEXT("Open file %s failed!"), *FullPath);
			WorkflowState = Enum_TerrainPointsCreatorWorkflowState::Error;
			GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteNeighborsLoopData.Rate, false);
			return;
		}
		if (!WriteNeighbors(ofs, i)) {
			return;
		}
	}

	WorkflowState = Enum_TerrainPointsCreatorWorkflowState::WritePointIndices;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WriteNeighborsLoopData.Rate, false);
	UE_LOG(TerrainPointsCreator, Log, TEXT("Write neighbors done."));
}

int32 ATerrainPointsCreator::CalNeighborsWeight(int32 Range)
{
	int32 weight = 0;
	for (int32 i = 1; i <= Range; i++)
	{
		weight += i * NeighborStep;
	}
	return weight;
}

void ATerrainPointsCreator::CreateNeighborPath(FString& NeighborPath, int32 Radius)
{
	FString TilesNeighborPathPrefix = FString(TEXT(""));
	TilesNeighborPathPrefix.Append(DataFileRelPath).Append(TEXT("N"));
	NeighborPath.Append(TilesNeighborPathPrefix).Append(FString::FromInt(Radius)).Append(FString(TEXT(".data")));
}

bool ATerrainPointsCreator::WriteNeighbors(std::ofstream& ofs, int32 Radius)
{
	int32 Count = 0;
	TArray<int32> Indices = { Radius, 0 };
	bool SaveLoopFlag = false;

	int32 ProgressPre = Points.Num() * CalNeighborsWeight(Radius - 1);
	int32 ProgressRatio = Radius * NeighborStep;
	int32 i = WriteNeighborsLoopData.IndexSaved[1];
	for (; i <= Points.Num() - 1; i++)
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
	UE_LOG(TerrainPointsCreator, Log, TEXT("Write neighbor N%d done."), Radius);
	return true;
}

void ATerrainPointsCreator::WriteNeighborLine(std::ofstream& ofs, int32 Index, int32 Radius)
{
	FStructTerrainPointNeighbors Neighbors = Points[Index].Neighbors[Radius - 1];
	for (int32 i = 0; i < Neighbors.Points.Num(); i++)
	{
		FString Str = FString::FromInt(Neighbors.Points[i].X);
		Str.Append(*CommaDelim);
		Str.Append(FString::FromInt(Neighbors.Points[i].Y));
		if (i != Neighbors.Points.Num() - 1) {
			Str.Append(*SpaceDelim);
		}
		ofs << TCHAR_TO_ANSI(*Str);
	}
	WriteLineEnd(ofs);
}

void ATerrainPointsCreator::WritePointIndicesToFile()
{
	FString FullPath;
	FString PointIndicesDataPath = FString(TEXT(""));
	PointIndicesDataPath.Append(DataFileRelPath).Append(PointIndicesDataFileName);
	CreateFilePath(PointIndicesDataPath, FullPath);

	FTimerHandle TimerHandle;
	std::ofstream ofs;
	if (!WritePointIndicesLoopData.HasInitialized) {
		WritePointIndicesLoopData.HasInitialized = true;
		ofs.open(*FullPath, std::ios::out | std::ios::trunc);
		ProgressTarget = Points.Num();
	}
	else {
		ofs.open(*FullPath, std::ios::out | std::ios::app);
	}

	if (!ofs || !ofs.is_open()) {
		UE_LOG(TerrainPointsCreator, Warning, TEXT("Open file %s failed!"), *FullPath);
		WorkflowState = Enum_TerrainPointsCreatorWorkflowState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WritePointIndicesLoopData.Rate, false);
		return;
	}
	WritePointIndices(ofs);
}

void ATerrainPointsCreator::WritePointIndices(std::ofstream& ofs)
{
	int32 Count = 0;
	TArray<int32> Indices = { 0 };
	bool SaveLoopFlag = false;

	int32 i = WritePointIndicesLoopData.IndexSaved[0];
	for (; i <= Points.Num() - 1; i++)
	{
		Indices[0] = i;
		FlowControlUtility::SaveLoopData(this, WritePointIndicesLoopData, Count, Indices, WorkflowDelegate, SaveLoopFlag);
		if (SaveLoopFlag) {
			ofs.close();
			return;
		}
		WritePointIndicesLine(ofs, i);
		ProgressCurrent = WritePointIndicesLoopData.Count;
		Count++;
	}
	ofs.close();
	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainPointsCreatorWorkflowState::WriteParams;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, WritePointIndicesLoopData.Rate, false);
	UE_LOG(TerrainPointsCreator, Log, TEXT("Write points indices done."));
}

void ATerrainPointsCreator::WritePointIndicesLine(std::ofstream& ofs, int32 Index)
{
	FIntPoint key = Points[Index].AxialCoord;
	WriteIndicesKey(ofs, key);
	WritePipeDelimiter(ofs);
	WriteIndicesValue(ofs, Index);
	WriteLineEnd(ofs);
}

void ATerrainPointsCreator::WriteIndicesKey(std::ofstream& ofs, const FIntPoint& key)
{
	FString Str = FString::FromInt(key.X);
	Str.Append(*CommaDelim);
	Str.Append(FString::FromInt(key.Y));
	ofs << TCHAR_TO_ANSI(*Str);
}

void ATerrainPointsCreator::WriteIndicesValue(std::ofstream& ofs, int32 Index)
{
	FString Str = FString::FromInt(Index);
	ofs << TCHAR_TO_ANSI(*Str);
}

void ATerrainPointsCreator::WriteParamsToFile()
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
		UE_LOG(TerrainPointsCreator, Warning, TEXT("Open file %s failed!"), *FullPath);
		WorkflowState = Enum_TerrainPointsCreatorWorkflowState::Error;
		GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
		return;
	}

	WriteParams(ofs);
}

void ATerrainPointsCreator::WriteParams(std::ofstream& ofs)
{
	WriteParamsContent(ofs);
	ProgressCurrent = 1;
	ofs.close();
	FTimerHandle TimerHandle;
	WorkflowState = Enum_TerrainPointsCreatorWorkflowState::Done;
	GetWorldTimerManager().SetTimer(TimerHandle, WorkflowDelegate, DefaultTimerRate, false);
	UE_LOG(TerrainPointsCreator, Log, TEXT("Write params done."));
}

void ATerrainPointsCreator::WriteParamsContent(std::ofstream& ofs)
{
	FString Str = FString::FromInt(GridRange);
	ofs << TCHAR_TO_ANSI(*Str);
	WritePipeDelimiter(ofs);
	Str = FString::FromInt(NeighborRange);
	ofs << TCHAR_TO_ANSI(*Str);
	WriteLineEnd(ofs);
}

void ATerrainPointsCreator::GetProgress(float& Out_Progress)
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

