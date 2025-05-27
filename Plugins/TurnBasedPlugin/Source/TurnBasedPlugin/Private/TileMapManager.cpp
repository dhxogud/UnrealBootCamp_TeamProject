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

// 타일 맵 생성 함수
void ATileMapManager::GenerateTileMap(int32 MapWidth, int32 MapHeight)
{
    if (!TileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TileClass is not set in ATileMapManager!"));
        return;
    }

    if (TileMap.IsEmpty() == false)
    {
		ClearTiles();  // 기존 타일이 존재하면 제거
    }

	m_MapWidth = MapWidth;  // 맵의 너비 설정
	m_MapHeight = MapHeight; // 맵의 높이 설정

    // 타일 배열 크기 설정
    TileMap.SetNum(MapWidth * MapHeight);  // 1D 배열로 크기 설정

    for (int32 Y = 0; Y < MapHeight; ++Y)
    {
        for (int32 X = 0; X < MapWidth; ++X)
        {
            // 타일 생성
            ATileBase * NewTile = GetWorld()->SpawnActor<ATileBase>(TileClass, FVector::ZeroVector, FRotator::ZeroRotator);

            // 타일의 크기를 설정
            TileSize = NewTile->GetTileSize();
            FVector Location(X * TileSize, Y * TileSize, 0.f);
			Location += FVector(TileSize / 2.f, TileSize / 2.f, 0.f);// 0,0 타일을 TileMapManager의 중심에 맞추기 위해 위치 조정

            // FSearchTileData 초기화
            FSearchTileData SearchTileData;
            SearchTileData.X = X;
            SearchTileData.Y = Y;
            SearchTileData.GCost = 0;
            SearchTileData.HCost = 0;
            SearchTileData.FCost = 0;
            SearchTileData.Parent = nullptr;

            NewTile->SearchTileData = SearchTileData;
			NewTile->TileMapManager = this;  // 타일 매니저 설정
            NewTile->bIsWalkable = true; // 기본값 설정

            FString TileName = FString::Printf(TEXT("Tile%d%d"), X, Y);  // 타일 이름 설정
            NewTile->SetActorLabel(TileName);
			NewTile->SetActorLocation(Location + GetActorLocation());  // 타일 위치 설정

            // 1D 배열에 저장
            TileMap[Y * MapWidth + X] = NewTile;  // 1D 배열로 저장
        }
    }
}

// 타일 초기화 및 제거 함수
void ATileMapManager::ClearTiles()
{
    // 타일 배열을 순회하며 타일을 제거
    for (int32 i = 0; i < TileMap.Num(); ++i)
    {
        ATileBase* Tile = TileMap[i];
        if (Tile)
        {
            // 타일 파괴
            Tile->Destroy();
        }
    }

    // 타일 배열 초기화
    TileMap.Empty();
}

ATileBase* ATileMapManager::GetTileAtIndex(int32 Index)
{
    // 유효한 범위 내에 있는지 확인
    if (Index >= 0 && Index < (m_MapWidth * m_MapHeight))
    {
        // 유효한 인덱스인 경우
        return TileMap[Index];
    }

    return nullptr;
}

ATileBase* ATileMapManager::GetTileAt(FVector Location)
{
    // 타일 크기를 기준으로 X, Y 좌표를 계산
    int32 X = FMath::FloorToInt(Location.X / TileSize);
    int32 Y = FMath::FloorToInt(Location.Y / TileSize);

    // 유효한 범위 내에 있는지 확인
    int32 Index = Y * m_MapWidth + X; // 1D 인덱스로 변환

    // 유효한 범위 내에 있는지 확인
    if (Index >= 0 && Index < (m_MapWidth * m_MapHeight))
    {
        // 유효한 인덱스인 경우
        return TileMap[Index];
    }

    UE_LOG(LogTemp, Warning, TEXT("GetTileAt: Invalid Location! X: %d, Y: %d"), X, Y);  // 오류 메시지 출력
    return nullptr; // 유효하지 않은 경우 null 반환
}

TArray<ATileBase*> ATileMapManager::FindPath(FVector StartLocation, FVector EndLocation)
{
    TArray<ATileBase*> OpenList;
    TArray<ATileBase*> ClosedList;

    // 시작점과 목표점 타일 찾기
    ATileBase* StartTile = GetTileAt(StartLocation);
    ATileBase* GoalTile = GetTileAt(EndLocation);

    if (!StartTile || !GoalTile)
    {
        return TArray<ATileBase*>(); // 시작점이나 목표점이 유효하지 않으면 빈 경로 반환
    }

    OpenList.Add(StartTile);

    while (OpenList.Num() > 0)
    {
        // F값이 가장 작은 타일을 선택
        ATileBase* CurrentTile = GetLowestCostTile(OpenList);

        // 목표 타일에 도달했으면 경로 추적 시작
        if (CurrentTile == GoalTile)
        {
            TArray<ATileBase*> Path;
            // 경로 추적
            while (CurrentTile->SearchTileData.Parent != nullptr)
            {
                Path.Insert(CurrentTile, 0); // 경로를 뒤에서부터 추적
                CurrentTile = CurrentTile->SearchTileData.Parent; // 부모 타일로 이동
            }

			// 경로 완료 후 부모 초기화
            for (ATileBase* Tile : Path)
            {
                if (Tile)
                {
                    Tile->SearchTileData.Parent = nullptr;  // 부모를 초기화
                }
            }

            return Path; // 경로 반환
        }

        OpenList.Remove(CurrentTile);
        ClosedList.Add(CurrentTile);

        // 이웃 타일 탐색
        TArray<ATileBase*> Neighbors = GetNeighbors(CurrentTile);
        for (ATileBase* Neighbor : Neighbors)
        {
            if (ClosedList.Contains(Neighbor))
                continue; // 이미 검사한 타일은 건너뛰기

			int32 TerrainCost = Neighbor->TerrainCost;// 지형 비용
            int32 TentativeGCost = FVector::DistSquared(CurrentTile->GetActorLocation(), Neighbor->GetActorLocation()) + TerrainCost;

            // GCost가 더 작거나 OpenList에 없는 경우
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

    return TArray<ATileBase*>(); // 경로를 찾을 수 없으면 빈 배열 반환
}

TArray<ATileBase*> ATileMapManager::GetNeighbors(ATileBase* Tile)
{
    TArray<ATileBase*> Neighbors;
    int32 X = Tile->SearchTileData.X;
    int32 Y = Tile->SearchTileData.Y;

    // 1D 배열에서 인덱스 계산
    int32 Index = Y * m_MapWidth + X;

    // 상하좌우, 대각선 방향으로 이웃 타일을 탐색
    TArray<FVector> Directions = {
        FVector(1, 0, 0),   // 오른쪽
        FVector(-1, 0, 0),  // 왼쪽
        FVector(0, 1, 0),   // 위
        FVector(0, -1, 0),  // 아래
        FVector(1, 1, 0),   // 우상
        FVector(-1, -1, 0), // 좌하
        FVector(1, -1, 0),  // 우하
        FVector(-1, 1, 0)   // 좌상
    };

    for (const FVector& Direction : Directions)
    {
        int32 NewX = X + Direction.X;
        int32 NewY = Y + Direction.Y;

        // 1D 배열에서 유효한 범위 내에 있는지 확인
        int32 NewIndex = NewY * m_MapWidth + NewX;

        if (NewIndex >= 0 && NewIndex < (m_MapWidth * m_MapHeight)) // 유효한 인덱스 확인
        {
            ATileBase* Neighbor = TileMap[NewIndex];
            if (Neighbor && Neighbor->bIsWalkable) // 이동 가능한 타일만 추가
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