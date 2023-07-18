#include "pch.h"
#include <cstdint>
#include <memory>

#include "Scanner.h"
#include "LuaCoreWrapper.h"


struct AugmentLookupItemData_t
{
	uint32_t ExtDataBlock1;
	uint32_t ExtDataBlock2;
	uint32_t ExtDataBlock3;
	uint16_t ItemId;
	uint16_t Unk1;//used for weapon augments
	uint16_t Unk2;//used for weapon augments
};

const auto fnGetAugmentSystemFourDataPattern = "\x53\x55\x56\x57\x33\xc9\xe8\x00\x00\x00\x00\x8b\x74\x24\x18";
const auto fnGetAugmentSystemFourDataMask = "xxxxxxx????xxxx";

using fnGetAugmentSystemFourData = int(__cdecl*)(uint32_t*, AugmentLookupItemData_t*);

const auto oGetAugmentSystemFourData = reinterpret_cast<fnGetAugmentSystemFourData>(FindPattern::ScanModIn(
	const_cast<char*>(fnGetAugmentSystemFourDataPattern),
	const_cast<char*>(fnGetAugmentSystemFourDataMask),
	"FFXiMain.dll"));


//TODO: decide on a better name maybe
static int GetAugmentSystemFourData(lua_State* L)
{
	if(!oGetAugmentSystemFourData)
	{
		return LuaCoreWrapper::oLuaL_Error(L, "Failed to find oGetAugmentSystemFourData function, the signature may have changed");
	}

	if(LuaCoreWrapper::oLua_GetTop(L) != 4)
	{
		return LuaCoreWrapper::oLuaL_Error(L, "Invalid number of arguments passed to RetrieveAugmentData, expected 4 (exdatablock1, exdatablock2, exdatablock3, itemId)");
	}

	//TODO: accept these in a more flexible/nicer way
	const auto extDataBlock1 = static_cast<uint32_t>(LuaCoreWrapper::oLuaL_CheckInteger(L, 1));
	const auto extDataBlock2 = static_cast<uint32_t>(LuaCoreWrapper::oLuaL_CheckInteger(L, 2));
	const auto extDataBlock3 = static_cast<uint32_t>(LuaCoreWrapper::oLuaL_CheckInteger(L, 3));
	const auto itemId = static_cast<uint16_t>(LuaCoreWrapper::oLuaL_CheckInteger(L, 4));

	auto augmentLookupItemData = AugmentLookupItemData_t{
		extDataBlock1, //exdatablock1
		extDataBlock2, //exdatablock2
		extDataBlock3, //exdatablock3
		itemId, //itemId
		0, //unk, only populated in memory for weapons that I saw but appears to work fine without it //TODO: figure out what this is
		0, //unk, only populated in memory for weapons that I saw but appears to work fine without it
	};

	const auto augmentData = std::make_unique<uint32_t[]>(86);
	const auto result = oGetAugmentSystemFourData(augmentData.get(), &augmentLookupItemData);

	LuaCoreWrapper::oLua_NewTable(L);

	/*_raw sub table*/

	LuaCoreWrapper::oLua_NewTable(L);

	constexpr auto augmentDataSize = 86;

	for(auto i = 0; i < augmentDataSize; i++)
	{
		LuaCoreWrapper::oLua_PushNumber(L, augmentData[i]);
		LuaCoreWrapper::oLua_RawSetI(L, -2, i + 1);
	}

	LuaCoreWrapper::oLua_PushNumber(L, augmentDataSize);
	LuaCoreWrapper::oLua_SetField(L, -2, "count");

	LuaCoreWrapper::oLua_SetField(L, -2, "_raw");

	/*back to parent*/

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[0]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Augment 1 Id");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[1]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Augment 1 Potency");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[2]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Augment 2 Id");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[3]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Augment 2 Potency");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[4]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Augment 3 Id");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[5]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Augment 3 Potency");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[6]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Augment 4 Id");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[7]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Augment 4 Potency");

	//TODO: implement rank, path, tnl, etc fields

	return 1;
}

extern "C" int __declspec(dllexport) luaopen_Augments(lua_State* L)
{
	constexpr struct luaL_Reg funcs[] = {
		{"GetAugmentSystemFourData", GetAugmentSystemFourData},
		{nullptr, nullptr}
	};

	LuaCoreWrapper::oLuaL_Register(L, "Augments", funcs);
	return 1;
}