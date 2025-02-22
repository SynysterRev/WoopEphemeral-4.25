// Golem Project - Créajeux 2020

#include "BlueprintFunctionLibraryHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Player/CharacterControllerFPS.h"

bool UBlueprintFunctionLibraryHelper::FindScreenEdgeLocation(UObject* WorldContextObject, const FVector& _targetLocation, const float _edgePercent, float& _rotation, FVector2D& _screenPosition)
{
	FVector2D screenPosition;

	const FVector2D viewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
	const FVector2D viewportCenter = viewportSize / 2.0f;

	UWorld* world = GEngine->GetWorldFromContextObject(WorldContextObject);
	if (world == nullptr) return false;

	APlayerController* controller = UGameplayStatics::GetPlayerController(world, 0);
	ACharacterControllerFPS* character = Cast<ACharacterControllerFPS>(controller->GetPawn());

	if (character == nullptr) return false;

	FVector forward = character->GetActorForwardVector();
	FVector offset = (_targetLocation - character->GetActorLocation()).GetSafeNormal();

	float dot = FVector::DotProduct(forward, offset);
	bool behindCam = (dot < 0);

	controller->ProjectWorldLocationToScreen(_targetLocation, screenPosition);

	if (screenPosition.X >= 0.f && screenPosition.X <= viewportSize.X
		&& screenPosition.Y >= 0.f && screenPosition.Y <= viewportSize.Y)
	{
		_screenPosition = screenPosition;
		return true;
	}

	screenPosition -= viewportCenter;

	float angleRadians = FMath::Atan2(screenPosition.Y, screenPosition.X);
	_rotation = FMath::RadiansToDegrees(angleRadians);
	angleRadians -= FMath::DegreesToRadians(90.0f);

	float cos = cosf(angleRadians);
	float sin = -sinf(angleRadians);

	screenPosition = FVector2D(viewportCenter.X + sin * 150.0f, viewportCenter.Y + cos * 150.0f);

	float m = cos / sin;

	FVector2D screenBounds = viewportCenter * _edgePercent;

	if (cos > 0)
	{
		screenPosition = FVector2D(screenBounds.Y / m, screenBounds.Y);
	}
	else
	{
		screenPosition = FVector2D(-screenBounds.Y / m, -screenBounds.Y);
	}

	if (screenPosition.X > screenBounds.X)
	{
		screenPosition = FVector2D(screenBounds.X, screenBounds.X * m);
	}
	else if (screenPosition.X < -screenBounds.X)
	{
		screenPosition = FVector2D(-screenBounds.X, -screenBounds.X * m);
	}

	screenPosition += viewportCenter;

	_screenPosition = screenPosition;
	return false;
}
