

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FECSProjectilesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
