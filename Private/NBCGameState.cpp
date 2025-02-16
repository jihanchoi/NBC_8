// Fill out your copyright notice in the Description page of Project Settings.


#include "NBCGameState.h"
#include "NBCGameInstance.h"
#include "NBCPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Blueprint/UserWidget.h"

ANBCGameState::ANBCGameState()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	LevelDuration = 12.0f;
	CurrentLevelIndex = 0;
	MaxLevels = 3;
	WaveIndex = 1;
}

void ANBCGameState::BeginPlay()
{
	Super::BeginPlay();

	StartLevel();

	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ANBCGameState::UpdateHUD,
		0.01f,
		true
	);
}

int32 ANBCGameState::GetScore() const
{
	return Score;
}

void ANBCGameState::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UNBCGameInstance* NBCGameInstance = Cast<UNBCGameInstance>(GameInstance);
		if (NBCGameInstance)
		{
			NBCGameInstance->AddToScore(Amount);
		}
	}
}

void ANBCGameState::StartLevel()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ANBCPlayerController* NBCPlayerController = Cast<ANBCPlayerController>(PlayerController))
		{
			NBCPlayerController->ShowGameHUD();
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UNBCGameInstance* NBCGameInstance = Cast<UNBCGameInstance>(GameInstance);
		if (NBCGameInstance)
		{
			CurrentLevelIndex = NBCGameInstance->CurrentLevelIndex;
		}
	}
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

	const int32 ItemToSpawn = 40;

	for (int32 i = 0; i < ItemToSpawn; i++)
	{
		if (FoundVolumes.Num() > 0)
		{
			ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
			if (SpawnVolume)
			{
				AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
				if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
				{
					SpawnedCoinCount++;
				}
			}
		}
	}

	GetWorldTimerManager().SetTimer(
		LevelTimerHandle,
		this,
		&ANBCGameState::OnLevelTimeUp,
		LevelDuration,
		false
	);
	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&ANBCGameState::OnWaveTimeUp,
		LevelDuration / 3.0f,
		true
	);
}

void ANBCGameState::OnLevelTimeUp()
{
	EndLevel();
}

void ANBCGameState::OnWaveTimeUp()
{
	WaveIndex++;

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

	const int32 ItemToSpawn = 10;

	for (int32 i = 0; i < ItemToSpawn; i++)
	{
		if (FoundVolumes.Num() > 0)
		{
			ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
			if (SpawnVolume)
			{
				AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
				if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
				{
					SpawnedCoinCount++;
				}
			}
		}
	}
}

void ANBCGameState::OnCoinCollected()
{
	CollectedCoinCount++;

	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{
		EndLevel();
	}
}

void ANBCGameState::EndLevel()
{
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UNBCGameInstance* NBCGameInstance = Cast<UNBCGameInstance>(GameInstance);
		if (NBCGameInstance)
		{
			AddScore(Score);
			CurrentLevelIndex++;
			NBCGameInstance->CurrentLevelIndex = CurrentLevelIndex;
		}
	}

	if (CurrentLevelIndex >= MaxLevels)
	{
		UpdateHUD();
		OnGameOver();
		return;
	}

	if(LevelMapNames.IsValidIndex(CurrentLevelIndex))
	{
		UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
	}
	else
	{
		OnGameOver();
	}
}
void ANBCGameState::OnGameOver()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ANBCPlayerController* NBCPlayerController = Cast<ANBCPlayerController>(PlayerController))
		{
			NBCPlayerController->ShowMainMenu(true);
			NBCPlayerController->SetPause(true);
		}
	}
}

void ANBCGameState::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ANBCPlayerController* NBCPlayerController = Cast<ANBCPlayerController>(PlayerController))
		{
			if (UUserWidget* HUDWidget = NBCPlayerController->GetHUDWidget())
			{
				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
				{
					float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time : %.1f"), RemainingTime)));
				}

				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						UNBCGameInstance* NBCGameInstance = Cast<UNBCGameInstance>(GameInstance);
						if (NBCGameInstance)
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score : %d"), NBCGameInstance->TotalScore)));
						}
					}
				}

				if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level %d"), CurrentLevelIndex+1)));
				}

				if (UTextBlock* WaveIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Wave"))))
				{
					WaveIndexText->SetText(FText::FromString(FString::Printf(TEXT("Wave %d"), WaveIndex)));
				}

				if (UProgressBar* HPProgressBar = Cast<UProgressBar>(HUDWidget->GetWidgetFromName(TEXT("HPBar"))))
				{
					if (ANBCCharacter* NBCCharactor = NBCPlayerController->GetNBCcharacter())
					{
						Hp = NBCCharactor->GetHelath();
						MaxHp = NBCCharactor->GetMaxHelath();
						HPProgressBar->SetPercent(Hp / MaxHp);
						if (UTextBlock* HpIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Hp"))))
						{
							HpIndexText->SetText(FText::FromString(FString::Printf(TEXT("%.1f / %.1f"), Hp, MaxHp)));
						}
					}
				}
			}
		}
	}
}