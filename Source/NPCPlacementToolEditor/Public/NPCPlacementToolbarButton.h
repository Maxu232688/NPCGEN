#pragma once

#include "CoreMinimal.h"

class FNPCPlacementToolbarButton
{
public:
	void Initialize();
	void Shutdown();

private:
	TSharedPtr<class FUICommandList> CommandList;

	void OnStartPlacementClicked();
	void AddToolbarExtension(FToolBarBuilder& Builder);
};
