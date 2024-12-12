// Copyright Epic Games, Inc. All Rights Reserved.

#include "LoAW_GridDataCreator.h"
#include "LoAW_GridDataCreatorStyle.h"
#include "LoAW_GridDataCreatorCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Kismet/GameplayStatics.h"
#include "DataCreator.h"

static const FName LoAW_GridDataCreatorTabName("LoAW_GridDataCreator");

#define LOCTEXT_NAMESPACE "FLoAW_GridDataCreatorModule"

void FLoAW_GridDataCreatorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLoAW_GridDataCreatorStyle::Initialize();
	FLoAW_GridDataCreatorStyle::ReloadTextures();

	FLoAW_GridDataCreatorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLoAW_GridDataCreatorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FLoAW_GridDataCreatorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLoAW_GridDataCreatorModule::RegisterMenus));
}

void FLoAW_GridDataCreatorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FLoAW_GridDataCreatorStyle::Shutdown();

	FLoAW_GridDataCreatorCommands::Unregister();
}

void FLoAW_GridDataCreatorModule::PluginButtonClicked()
{
	AActor* FoundActor;
	FoundActor = FindActor(ADataCreator::StaticClass());
	if (FoundActor) {
		ADataCreator* Creator = Cast<ADataCreator>(FoundActor);
		Creator->CreateData();
	}
}

AActor* FLoAW_GridDataCreatorModule::FindActor(TSubclassOf<AActor> ActorClass)
{
	TArray<AActor*> FoundActor;
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (World) {
		UGameplayStatics::GetAllActorsOfClass(World, ActorClass, FoundActor);
		if (FoundActor.Num() > 0) {
			return FoundActor[0];
		}
	}
	return nullptr;
}

void FLoAW_GridDataCreatorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FLoAW_GridDataCreatorCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLoAW_GridDataCreatorCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLoAW_GridDataCreatorModule, LoAW_GridDataCreator)