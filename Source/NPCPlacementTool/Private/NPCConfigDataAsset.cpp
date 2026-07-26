#include "NPCConfigDataAsset.h"

bool UNPCConfigDataAsset::GetNPCConfig(int32 Index, FNPCDefinition& OutConfig) const
{
	if (NPCDefinitions.IsValidIndex(Index))
	{
		OutConfig = NPCDefinitions[Index];
		return true;
	}
	return false;
}

bool UNPCConfigDataAsset::GetNPCByHotKey(int32 InHotKey, FNPCDefinition& OutConfig) const
{
	for (const FNPCDefinition& Def : NPCDefinitions)
	{
		if (Def.HotKey == InHotKey)
		{
			OutConfig = Def;
			return true;
		}
	}
	return false;
}
