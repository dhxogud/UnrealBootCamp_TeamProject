// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TileMapManager.generated.h"

UCLASS()
class TURNBASEDPLUGIN_API ATileMapManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATileMapManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
    // 타일 맵 생성 함수
	UFUNCTION(BlueprintCallable, Category = "Tile Map")
    void GenerateTileMap(int32 MapWidth, int32 MapHeight);

	// 타일 초기화 및 제거 함수
	UFUNCTION(BlueprintCallable, Category = "Tile Map")
	void ClearTiles();

	UFUNCTION(BlueprintCallable, Category = "Tile Map")
	ATileBase* GetTileAtIndex(int32 Index);

	// 타일의 위치를 찾는 함수
	UFUNCTION(BlueprintCallable, Category = "Tile Map")
	ATileBase* GetTileAt(FVector Location);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Map")
	float TileSize;  // 타일 크기

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Map")
	int32 m_MapWidth;  // 맵의 너비

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Map")
	int32 m_MapHeight; // 맵의 높이

	// 타일을 생성할 때 사용할 클래스 (블루프린트에서 편집 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Map")
	TSubclassOf<ATileBase> TileClass;  // ATile의 자식 클래스도 설정 가능

    // 타일 배열 (맵)
    // UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile Map") <-- 다차원 배열은 블루프린트 지원 불가능
    // TArray<TArray<ATileBase*>> TileMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile Map")
	TArray<ATileBase*> TileMap; // 1차원 배열로 변경

//---------------------------------------------[ Begin A* 경로 탐색 ]------------------------------------------------//

public:
	// A* 경로 탐색 함수
	UFUNCTION(BlueprintCallable, Category = "A Star")
	TArray<ATileBase*> FindPath(FVector StartLocation, FVector EndLocation);

	// 타일의 이웃을 구하는 함수
	TArray<ATileBase*> GetNeighbors(ATileBase* Tile);

	// 타일의 F값을 계산하는 함수
	ATileBase* GetLowestCostTile(TArray<ATileBase*>& TileList);

//---------------------------------------------[ End A* 경로 탐색 ]------------------------------------------------//

};
