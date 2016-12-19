// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "TempMatCreatePrivatePCH.h"
#include "TempMatCreateCommands.h"

#define LOCTEXT_NAMESPACE "FTempMatCreateModule"

void FTempMatCreateCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "TempMatCreate", "Execute TempMatCreate action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
