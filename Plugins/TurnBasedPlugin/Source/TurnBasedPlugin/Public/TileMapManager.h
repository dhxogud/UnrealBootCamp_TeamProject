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
	UFUNCTION(BlueprintCallable, Category = "Tile Map")
    void GenerateTileMap(int32 MapWidth, int32 MapHeight);

	UFUNCTION(BlueprintCallable, Category = "Tile Map")
	void ClearTiles();

	UFUNCTION(BlueprintCallable, Category = "Tile Map")
	ATileBase* GetTileAtIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Tile Map")
	ATileBase* GetTileAt(FVector Location);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Map")
	float TileSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Map")
	int32 m_MapWidth;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Map")
	int32 m_MapHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Map")
	TSubclassOf<ATileBase> TileClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile Map")
	TArray<ATileBase*> TileMap;

public:
	UFUNCTION(BlueprintCallable, Category = "A Star")
	TArray<ATileBase*> FindPath(FVector StartLocation, FVector EndLocation);

	TArray<ATileBase*> GetNeighbors(ATileBase* Tile);

	ATileBase* GetLowestCostTile(TArray<ATileBase*>& TileList);

	void ResetAllTileSearchData();

};
