// Fill out your copyright notice in the Description page of Project Settings.


#include "NBCPlayerController.h"
#include "NBCGameState.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "NBCGameInstance.h"

ANBCPlayerController::ANBCPlayerController()
	:InputMappingContext(nullptr),
	 MoveAction(nullptr),
	 JumpAction(nullptr),
	 LookAction(nullptr),
	 SprintAction(nullptr),
	 HUDWidgetClass(nullptr),
	 HUDWidgetInstance(nullptr),
	 NBCCharacterInstansce(nullptr),
	 MainMenuWidgetClass(nullptr),
	 MainMenuWidgetInstance(nullptr) {}

void ANBCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* SubSystem = LocalPayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				SubSystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	FString CurrentMapName = GetWorld()->GetMapName();
	if (CurrentMapName.Contains("MenuLevel"))
	{
		ShowMainMenu(false);
	}
}

UUserWidget* ANBCPlayerController::GetHUDWidget() const
{
	return HUDWidgetInstance;
}

ANBCCharacter* ANBCPlayerController::GetNBCcharacter() const
{
	return NBCCharacterInstansce;
}

void ANBCPlayerController::ShowMainMenu(bool bIsRestart)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UUserWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->AddToViewport();

			bShowMouseCursor = true;
			SetInputMode(FInputModeUIOnly());
		}

		if (UTextBlock* ButtonText = Cast<UTextBlock>(MainMenuWidgetInstance->GetWidgetFromName(TEXT("StartButtonText"))))
		{
			if (bIsRestart)
			{
				ButtonText->SetText(FText::FromString(TEXT("RESTART")));
			}
			else
			{
				ButtonText->SetText(FText::FromString(TEXT("START")));
			}
		}
	}
}

void ANBCPlayerController::ShowGameHUD()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->RemoveFromParent();
		HUDWidgetInstance = nullptr;
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
		MainMenuWidgetInstance = nullptr;
	}

	if (HUDWidgetClass)
	{
		HUDWidgetInstance = CreateWidget<UUserWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport();

			bShowMouseCursor = false;
			SetInputMode(FInputModeGameOnly());
		}

		ANBCGameState* NBCGameState = GetWorld() ? GetWorld()->GetGameState<ANBCGameState>() : nullptr;

		if (NBCGameState)
		{
			NBCGameState->UpdateHUD();
		}
	}
}

void ANBCPlayerController::StartGame()
{
	if (UNBCGameInstance* NBCGameInstance = Cast<UNBCGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		NBCGameInstance->CurrentLevelIndex = 0;
		NBCGameInstance->TotalScore = 0;
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("BasicLevel"));
}