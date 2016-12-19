// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "TempMatCreatePrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "TempMatCreateStyle.h"
#include "TempMatCreateCommands.h"

#include "LevelEditor.h"

#include "MaterialFactory.h"

static const FName TempMatCreateTabName("TempMatCreate");

#define LOCTEXT_NAMESPACE "FTempMatCreateModule"

void FTempMatCreateModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FTempMatCreateStyle::Initialize();
	FTempMatCreateStyle::ReloadTextures();

	FTempMatCreateCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FTempMatCreateCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FTempMatCreateModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FTempMatCreateModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FTempMatCreateModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FTempMatCreateModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FTempMatCreateStyle::Shutdown();

	FTempMatCreateCommands::Unregister();
}

void FTempMatCreateModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FTempMatCreateModule::PluginButtonClicked()")),
							FText::FromString(TEXT("TempMatCreate.cpp"))
					   );
	//FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	MaterialFactory matFact;
	matFact.CreateUnrealMaterial(TEXT("/Game/"),TEXT("TempMaterial"));
}

void FTempMatCreateModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FTempMatCreateCommands::Get().PluginAction);
}

void FTempMatCreateModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FTempMatCreateCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTempMatCreateModule, TempMatCreate)