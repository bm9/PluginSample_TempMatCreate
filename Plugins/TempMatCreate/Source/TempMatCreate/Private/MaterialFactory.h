// Copyright 2016 BlackMa9. All Rights Reserved.
#pragma once

#include "Engine.h"
#include "Factories/Factory.h"


class MaterialFactory
{
public:
	MaterialFactory();
	~MaterialFactory();



	//-------------------------------------------------------------------------
	//
	//-------------------------------------------------------------------------
	void CreateUnrealMaterial(
		FString ParentObjName,
		FString TargetBasdName
	);

	//-------------------------------------------------------------------------
	//
	//-------------------------------------------------------------------------
	void CreateUnrealMaterial(
		UMaterial* UnrealMaterial
	);

};

