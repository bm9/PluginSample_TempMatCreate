// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "TempMatCreateStyle.h"

class FTempMatCreateCommands : public TCommands<FTempMatCreateCommands>
{
public:

	FTempMatCreateCommands()
		: TCommands<FTempMatCreateCommands>(TEXT("TempMatCreate"), NSLOCTEXT("Contexts", "TempMatCreate", "TempMatCreate Plugin"), NAME_None, FTempMatCreateStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
