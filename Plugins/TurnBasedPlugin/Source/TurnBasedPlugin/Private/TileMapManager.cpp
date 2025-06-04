// Fill out your copyright notice in the Description page of Project Settings.


#include "TileMapManager.h"
#include "Engine/World.h"
#include "TileBase.h"

// Sets default values
ATileMapManager::ATileMapManager() : TileSize(100.f) // 기본 타일 크기 설정
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATileMapManager::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ATileMapManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATileMapManager::GenerateTileMap(int32 MapWidth, int32 MapHeight)
{
    if (!TileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TileClass is not set in ATileMapManager!"));
        return;
    }

    if (TileMap.IsEmpty() == false)
    {
		ClearTiles();
    }

	m_MapWidth = MapWidth;
	m_MapHeight = MapHeight;

    TileMap.SetNum(MapWidth * MapHeight);

    for (int32 Y = 0; Y < MapHeight; ++Y)
    {
        for (int32 X = 0; X < MapWidth; ++X)
        {
            ATileBase * NewTile = GetWorld()->SpawnActor<ATileBase>(TileClass, FVector::ZeroVector, FRotator::ZeroRotator);

            TileSize = NewTile->GetTileSize();
            FVector Location(X * TileSize, Y * TileSize, 0.f);
			Location += FVector(TileSize / 2.f, TileSize / 2.f, 0.f);

            FSearchTileData SearchTileData;
            SearchTileData.X = X;
            SearchTileData.Y = Y;
            SearchTileData.GCost = 0;
            SearchTileData.HCost = 0;
            SearchTileData.FCost = 0;
            SearchTileData.Parent = nullptr;

            NewTile->SearchTileData = SearchTileData;
			NewTile->TileMapManager = this;
            NewTile->bIsWalkable = true;

            FString TileName = FString::Printf(TEXT("Tile%d%d"), X, Y);
            NewTile->SetActorLabel(TileName);
			NewTile->SetActorLocation(Location + GetActorLocation());

            TileMap[Y * MapWidth + X] = NewTile;
        }
    }
}

void ATileMapManager::ClearTiles()
{
    for (int32 i = 0; i < TileMap.Num(); ++i)
    {
        ATileBase* Tile = TileMap[i];
        if (Tile)
        {
            Tile->Destroy();
        }
    }

    TileMap.Empty();
}

ATileBase* ATileMapManager::GetTileAtIndex(int32 Index)
{
    if (Index >= 0 && Index < (m_MapWidth * m_MapHeight))
    {
        return TileMap[Index];
    }

    return nullptr;
}

ATileBase* ATileMapManager::GetTileAt(FVector Location)
{
    int32 X = FMath::FloorToInt(Location.X / TileSize);
    int32 Y = FMath::FloorToInt(Location.Y / TileSize);

    int32 Index = Y * m_MapWidth + X;

    if (Index >= 0 && Index < (m_MapWidth * m_MapHeight))
    {
        return TileMap[Index];
    }

    UE_LOG(LogTemp, Warning, TEXT("GetTileAt: Invalid Location! X: %d, Y: %d"), X, Y);
    return nullptr;
}

TArray<ATileBase*> ATileMapManager::FindPath(FVector StartLocation, FVector EndLocation)
{
    TArray<ATileBase*> OpenList;
    TArray<ATileBase*> ClosedList;

    ATileBase* StartTile = GetTileAt(StartLocation);
    ATileBase* GoalTile = GetTileAt(EndLocation);

    if (!StartTile || !GoalTile)
    {
        return TArray<ATileBase*>();
    }

    OpenList.Add(StartTile);

    while (OpenList.Num() > 0)
    {
        ATileBase* CurrentTile = GetLowestCostTile(OpenList);

        if (CurrentTile == GoalTile)
        {
            TArray<ATileBase*> Path;

            while (CurrentTile->SearchTileData.Parent != nullptr)
            {
                Path.Insert(CurrentTile, 0);
                CurrentTile = CurrentTile->SearchTileData.Parent;
            }

            ResetAllTileSearchData();

            return Path;
        }

        OpenList.Remove(CurrentTile);
        ClosedList.Add(CurrentTile);

        TArray<ATileBase*> Neighbors = GetNeighbors(CurrentTile);
        for (ATileBase* Neighbor : Neighbors)
        {
            if (ClosedList.Contains(Neighbor))
                continue;

            int32 TerrainCost = Neighbor->TerrainCost;
            int32 TentativeGCost = FVector::DistSquared(CurrentTile->GetActorLocation(), Neighbor->GetActorLocation()) + TerrainCost;

            if (TentativeGCost < Neighbor->SearchTileData.GCost || !OpenList.Contains(Neighbor))
            {
                Neighbor->SearchTileData.Parent = CurrentTile;
                Neighbor->SearchTileData.GCost = TentativeGCost;
                Neighbor->SearchTileData.HCost = FVector::Dist(Neighbor->GetActorLocation(), GoalTile->GetActorLocation());
                Neighbor->SearchTileData.FCost = Neighbor->SearchTileData.GCost + Neighbor->SearchTileData.HCost;

                if (!OpenList.Contains(Neighbor))
                {
                    OpenList.Add(Neighbor);
                }
            }
        }
    }
    return TArray<ATileBase*>();
}


TArray<ATileBase*> ATileMapManager::GetNeighbors(ATileBase* Tile)
{
    TArray<ATileBase*> Neighbors;
    int32 X = Tile->SearchTileData.X;
    int32 Y = Tile->SearchTileData.Y;

    int32 Index = Y * m_MapWidth + X;

    TArray<FVector> Directions = {
        FVector(1, 0, 0),
        FVector(-1, 0, 0),
        FVector(0, 1, 0),
        FVector(0, -1, 0)
    };

    for (const FVector& Direction : Directions)
    {
        int32 NewX = X + Direction.X;
        int32 NewY = Y + Direction.Y;

        int32 NewIndex = NewY * m_MapWidth + NewX;

        if (NewIndex >= 0 && NewIndex < (m_MapWidth * m_MapHeight))
        {
            ATileBase* Neighbor = TileMap[NewIndex];
            if (Neighbor && Neighbor->bIsWalkable)
            {
                Neighbors.Add(Neighbor);
            }
        }
    }

    return Neighbors;
}

ATileBase* ATileMapManager::GetLowestCostTile(TArray<ATileBase*>& TileList)
{
    ATileBase* LowestCostTile = nullptr;

    for (ATileBase* Tile : TileList)
    {
        if (!LowestCostTile || Tile->SearchTileData.FCost < LowestCostTile->SearchTileData.FCost)
        {
            LowestCostTile = Tile;
        }
    }

    return LowestCostTile;
}

void ATileMapManager::ResetAllTileSearchData()
{
    for (ATileBase* Tile : TileMap)
    {
        Tile->SearchTileData.Parent = nullptr;
        Tile->SearchTileData.GCost = 0;
        Tile->SearchTileData.HCost = 0;
        Tile->SearchTileData.FCost = 0;
    }
}