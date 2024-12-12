// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FLoAW_GridDataCreatorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	AActor* FindActor(TSubclassOf<AActor> ActorClass);
	void RegisterMenus();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
