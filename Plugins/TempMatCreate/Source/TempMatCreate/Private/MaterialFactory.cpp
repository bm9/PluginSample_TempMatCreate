

#include "TempMatCreatePrivatePCH.h"

#include "MaterialFactory.h"

#include "UnrealEd.h"
#include "ComponentReregisterContext.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"

#include "ContentBrowserModule.h"

#include "PackageTools.h"

#include "ObjectTools.h"

#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionConstant.h"

#define LOCTEXT_NAMESPACE "PluginMaterialFactory"

namespace
{
	FContentBrowserMenuExtender_SelectedAssets ContentBrowserExtenderDelegate;
	FDelegateHandle ContentBrowserExtenderDelegateHandle;
}

MaterialFactory::MaterialFactory()
{
}


MaterialFactory::~MaterialFactory()
{
}

void MaterialFactory::CreateUnrealMaterial(
	FString ParentObjName,
	FString TargetBasdName
)
{
	/*Assetのファイル名 */
	FString MaterialFullName = FString("M_") + TargetBasdName;
													   
	//禁止文字を削除する(もし材質名に禁止文字が含まれていた場合のフェイルセーフ)
	MaterialFullName = ObjectTools::SanitizeObjectName(MaterialFullName);

	/*ディレクトリ名とファイル名*/
	FString BasePackageName = FPackageName::GetLongPackagePath(ParentObjName) / MaterialFullName;
	BasePackageName = PackageTools::SanitizePackageName(BasePackageName);

	// The material could already exist in the project
	FName ObjectPath = *(BasePackageName + TEXT(".") + MaterialFullName);

	/* ターゲットMaterial */
	UMaterial* UnrealMaterial = NULL;

	// 既に同一名称でMaterialのAssetが存在しているかチェック
	UMaterial* FoundMaterial = LoadObject<UMaterial>(NULL, *ObjectPath.ToString());
	// do not override existing materials
	if (FoundMaterial)
	{
		/*警告文*/
		FText DialogText = FText::Format(
			LOCTEXT("PluginMatFoundText", "既にMaterialアセットが存在しています.\n削除してから出直しな！\nアセット名:{0}"),
			FText::FromString(*ObjectPath.ToString())
		);
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);

		/*ちなみに同じ名前で別のAsset種別(例；カーブアセットなど)だと、アセット名の末尾に2とか数字が追加され新規作成される*/
		return;
	}

	/*****************************/
	/* 新規Material Aseetを作成  */
	/*****************************/

	const FString Suffix(TEXT(""));
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString FinalPackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, Suffix, FinalPackageName, MaterialFullName);
	
	UPackage* Package = CreatePackage(NULL, *FinalPackageName);

	// create an unreal material asset
	auto MaterialFactory = NewObject<UMaterialFactoryNew>();

	UnrealMaterial = (UMaterial*)MaterialFactory->FactoryCreateNew(
		UMaterial::StaticClass(), Package, *MaterialFullName, RF_Standalone | RF_Public, NULL, GWarn);

	if (UnrealMaterial != NULL)
	{
		// Notify the asset registry
		FAssetRegistryModule::AssetCreated(UnrealMaterial);

		// Set the dirty flag so this package will get saved later
		Package->SetDirtyFlag(true);
	}
	else
	{

		/*警告文*/
		FText DialogText = FText::Format(
			LOCTEXT("PluginMatCreateNGText", "原因不明：Material Aseetの生成に失敗しました\nアセット名:{0}"),
			FText::FromString(*ObjectPath.ToString())
		);
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);

		/*ちなみに同じ名前で別のAsset種別(例；カーブアセットなど)だと、アセット名の末尾に2とか数字が追加され新規作成される*/
		return;
	}

	/************************/
	/* MaterialNodeを作るよ */
	/************************/
	CreateUnrealMaterial(UnrealMaterial);

	/************************/
	/* おまじない動作:更新系*/
	/************************/
	// let the material update itself if necessary
	//UnrealMaterial->PreEditChange(NULL);
	UnrealMaterial->PostEditChange();

	/*警告文*/
	FText DialogText = FText::Format(
		LOCTEXT("PluginMatCreateOKText", "正常：Material Aseetを生成しました\nアセット名:{0}"),
		FText::FromString(*ObjectPath.ToString())
	);
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
	return;
}



void MaterialFactory::CreateUnrealMaterial(
	UMaterial* UnrealMaterial)
{

	//Blenmd Modeを Maskedに設定
	UnrealMaterial->BlendMode = BLEND_Masked;

	// MaterialのBaseColorに何も接続されていない場合
	if (UnrealMaterial->BaseColor.Expression == NULL)
	{
		//Vector4 Nodeを定義
		UMaterialExpressionVectorParameter* vec4Expression
			= NewObject<UMaterialExpressionVectorParameter>(UnrealMaterial);

		/* 対象MaterialにVec4 Nodeを追加する*/
		UnrealMaterial->Expressions.Add(vec4Expression);


		/* Vec4 Nodeの配置位置とNode名を設定する*/
		vec4Expression->MaterialExpressionEditorX = -300;
		vec4Expression->MaterialExpressionEditorY = 0;
		vec4Expression->SetEditableName("BaseColor");

		/* パラメータ設定が面倒なのでランダムでRGBを設定する*/
		vec4Expression->DefaultValue.R = 0.5f + (0.5f*FMath::Rand()) / RAND_MAX;
		vec4Expression->DefaultValue.G = 0.5f + (0.5f*FMath::Rand()) / RAND_MAX;
		vec4Expression->DefaultValue.B = 0.5f + (0.5f*FMath::Rand()) / RAND_MAX;
		vec4Expression->DefaultValue.A = 1.0f;

		/* 対象MaterialのBaseColorピンに先ほど定義したVec4Nodeを接続する */
		UnrealMaterial->BaseColor.Expression = vec4Expression;

		/* 多分Material側BaseColorの更新だと思うが、資料がないのでさっぱりわからない。おまじないで必要のはず。*/
		TArray<FExpressionOutput> Outputs = UnrealMaterial->BaseColor.Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		UnrealMaterial->BaseColor.Mask = Output->Mask;
		UnrealMaterial->BaseColor.MaskR = Output->MaskR;
		UnrealMaterial->BaseColor.MaskG = Output->MaskG;
		UnrealMaterial->BaseColor.MaskB = Output->MaskB;
		UnrealMaterial->BaseColor.MaskA = Output->MaskA;
	}

	// MaterialのMetallicに何も接続されていない場合
	if (UnrealMaterial->Metallic.Expression == NULL)
	{

		//Vector4 Nodeを定義
		UMaterialExpressionScalarParameter* sclrExpression
			= NewObject<UMaterialExpressionScalarParameter>(UnrealMaterial);

		/* 対象MaterialにscalarParam Nodeを追加する*/
		UnrealMaterial->Expressions.Add(sclrExpression);

		/* scalarParam Nodeの配置位置とNode名を設定する*/
		sclrExpression->MaterialExpressionEditorX = -300;
		sclrExpression->MaterialExpressionEditorY = 200;
		sclrExpression->SetEditableName("Metallic");

		/* パラメータ設定が面倒なのでランダムでRを設定する*/
		sclrExpression->DefaultValue = 0.5f + (0.5f*FMath::Rand()) / RAND_MAX;

		/* 対象MaterialのMetallicピンに先ほど定義したVec4Nodeを接続する */
		UnrealMaterial->Metallic.Expression = sclrExpression;

		/* 多分Material側Metallicの更新だと思うが、資料がないのでさっぱりわからない。おまじないで必要のはず。*/
		TArray<FExpressionOutput> Outputs = UnrealMaterial->Metallic.Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		UnrealMaterial->Metallic.Mask = Output->Mask;
		UnrealMaterial->Metallic.MaskR = Output->MaskR;
		UnrealMaterial->Metallic.MaskG = Output->MaskG;
		UnrealMaterial->Metallic.MaskB = Output->MaskB;
		UnrealMaterial->Metallic.MaskA = Output->MaskA;
	}

	// MaterialのEmissiveColorに何も接続されていない場合
	if (UnrealMaterial->EmissiveColor.Expression == NULL)
	{
		/********************/
		/* Vec4定義         */
		/********************/
		//Vector4 Nodeを定義
		UMaterialExpressionVectorParameter* vec4Expression
			= NewObject<UMaterialExpressionVectorParameter>(UnrealMaterial);

		/* 対象MaterialにVec4 Nodeを追加する*/
		UnrealMaterial->Expressions.Add(vec4Expression);

		/* Vec4 Nodeの配置位置とNode名を設定する*/
		vec4Expression->MaterialExpressionEditorX = -500;
		vec4Expression->MaterialExpressionEditorY = 300;
		vec4Expression->SetEditableName("EmissiveColor");

		/* パラメータ設定RGBを設定する*/
		vec4Expression->DefaultValue.R = 1.0f;
		vec4Expression->DefaultValue.G = 0.5f;
		vec4Expression->DefaultValue.B = 0.0f;
		vec4Expression->DefaultValue.A = 1.0f;

		/********************/
		/* scaler定義       */
		/********************/
		//Vector4 Nodeを定義
		UMaterialExpressionScalarParameter* sclrExpression
			= NewObject<UMaterialExpressionScalarParameter>(UnrealMaterial);

		/* 対象MaterialにscalarParam Nodeを追加する*/
		UnrealMaterial->Expressions.Add(sclrExpression);

		/* scalarParam Nodeの配置位置とNode名を設定する*/
		sclrExpression->MaterialExpressionEditorX = -500;
		sclrExpression->MaterialExpressionEditorY = 500;
		sclrExpression->SetEditableName("EmissivePower");

		/* パラメータ設定*/
		sclrExpression->DefaultValue = 4.0f;

		/********************/
		/* Vec4とscalerのMlt*/
		/********************/
		//Add Nodeを定義
		UMaterialExpressionMultiply* multiExpression
			= NewObject<UMaterialExpressionMultiply>(UnrealMaterial);

		/* 対象MaterialにMulti Nodeを追加する*/
		UnrealMaterial->Expressions.Add(multiExpression);

		/* Multi Nodeの配置位置を設定する*/
		multiExpression->MaterialExpressionEditorX = -200;
		multiExpression->MaterialExpressionEditorY = 400;

		/*Multi nodeのAピンにVec4 Nodeを接続*/
		multiExpression->A.Expression = vec4Expression;
		/*Multi NodeのBピンにScaler Nodeを接続*/
		multiExpression->B.Expression = sclrExpression;

		/* 対象MaterialのEmissiveColorピンに先ほど定義したMulti Nodeを接続する */
		UnrealMaterial->EmissiveColor.Expression = multiExpression;

		/* 多分Material側EmissiveColorの更新だと思うが、資料がないのでさっぱりわからない。おまじないで必要のはず。*/
		TArray<FExpressionOutput> Outputs = UnrealMaterial->EmissiveColor.Expression->GetOutputs();
		FExpressionOutput* Output = Outputs.GetData();
		UnrealMaterial->EmissiveColor.Mask = Output->Mask;
		UnrealMaterial->EmissiveColor.MaskR = Output->MaskR;
		UnrealMaterial->EmissiveColor.MaskG = Output->MaskG;
		UnrealMaterial->EmissiveColor.MaskB = Output->MaskB;
		UnrealMaterial->EmissiveColor.MaskA = Output->MaskA;
	}
	return;
}
