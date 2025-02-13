#include "NBCGameMode.h"
#include "NBCCharacter.h"
#include "NBCPlayerController.h"
#include "NBCGameState.h"

ANBCGameMode::ANBCGameMode()
{
	DefaultPawnClass = ANBCCharacter::StaticClass();
	PlayerControllerClass = ANBCPlayerController::StaticClass();
	GameStateClass = ANBCGameState::StaticClass();
}