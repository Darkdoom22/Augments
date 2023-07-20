#include "pch.h"
#include <cstdint>
#include <memory>
#include <array>

#include "Scanner.h"
#include "LuaCoreWrapper.h"
#include <format>

constexpr auto PathMap = std::array<const char*, 4>{
	"A",
	"B",
	"C",
	"D",
};

struct AugmentLookupItemData_t
{
	uint32_t ExtDataBlock1;
	uint32_t ExtDataBlock2;
	uint32_t ExtDataBlock3;
	uint16_t ItemId;
	uint16_t Unk1;//used for weapon augments
	uint16_t Unk2;//used for weapon augments
};

constexpr auto fnGetAugmentSystemFourDataPattern = "\x53\x55\x56\x57\x33\xc9\xe8\x00\x00\x00\x00\x8b\x74\x24\x18";
constexpr auto fnGetAugmentSystemFourDataMask = "xxxxxxx????xxxx";

using fnGetAugmentSystemFourData = int(__cdecl*)(int32_t*, AugmentLookupItemData_t*);

const auto oGetAugmentSystemFourData = reinterpret_cast<fnGetAugmentSystemFourData>(FindPattern::ScanModIn(
	const_cast<char*>(fnGetAugmentSystemFourDataPattern),
	const_cast<char*>(fnGetAugmentSystemFourDataMask),
	"FFXiMain.dll"));


//Not needed currently, but here for reference
//all the augment data for this system is also stored in here some bytes after the RP map, via some *really* weird offsetting using ExtDataBlock3
//that offset gets you to an entry with the resource id and a table of stat values based on rank, with the stat index being: (dataStruct.extDataBlock2 >> 0x12 & 0x1f) * 2 + 0xc)
//the game loops over these 4 times with different offsetting each time to populate all the augment data, but it's a bit of a mess so that's why we're just calling the function that does all this instead of trying to replicate it

/*
super small, sig includes some of the preceding function. will hardcode the map instead if this is prone to breaking on updates
*                    **************************************************************
                     *                          FUNCTION                          *
                     **************************************************************
                     uint16_t * __stdcall GetAugmentRpLookupTable(void)
     uint16_t *        EAX:4          <RETURN>
                     GetAugmentRpLookupTable                         XREF[1]:     RetrieveAugmentInfo_UnityDyna:10
100fe4b0 a1 08 5d        MOV        EAX,[g_AugmentRpLookupTable]                     = ??
         56 10
100fe4b5 c3              RET

 
constexpr auto fnGetAugmentRpLookupTablePattern = "\x5f\x5e\x5d\x5b\x83\xc4\x08\xc3\x90\x90\xa1\x00\x00\x00\x00\xc3";
constexpr auto fnGetAugmentRpLookupTableMask = "xxxxxxxxxxx????x";
constexpr auto fnGetAugmentRpLookupTableOffset = 10;

using fnGetAugmentRpLookupTable = uint16_t * (__stdcall*)();

const auto oGetAugmentRpLookupTable = reinterpret_cast<fnGetAugmentRpLookupTable>(FindPattern::ScanModIn(
	const_cast<char*>(fnGetAugmentRpLookupTablePattern),
	const_cast<char*>(fnGetAugmentRpLookupTableMask),
	"FFXiMain.dll") + fnGetAugmentRpLookupTableOffset);*/

//TODO: decide on a better name maybe
static int GetAugmentSystemFourData(lua_State* L)
{
	if(!oGetAugmentSystemFourData)
	{
		return LuaCoreWrapper::oLuaL_Error(L, "Failed to find oGetAugmentSystemFourData function, the signature may have changed");
	}

	if(LuaCoreWrapper::oLua_GetTop(L) != 4)
	{
		return LuaCoreWrapper::oLuaL_Error(L, "Invalid number of arguments passed to GetAugmentSystemFourData, expected 4 (exdatablock1, exdatablock2, exdatablock3, itemId)");
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

	constexpr auto augmentDataSize = 86;

	const auto augmentData = std::make_unique<int32_t[]>(augmentDataSize);
	const auto result = oGetAugmentSystemFourData(augmentData.get(), &augmentLookupItemData);

	LuaCoreWrapper::oLua_NewTable(L);

	/*_raw sub table*/

	LuaCoreWrapper::oLua_NewTable(L);


	for(auto i = 0; i < augmentDataSize; i++)
	{
		LuaCoreWrapper::oLua_PushNumber(L, augmentData[i]);
		LuaCoreWrapper::oLua_RawSetI(L, -2, i + 1);
	}

	LuaCoreWrapper::oLua_PushNumber(L, augmentDataSize);
	LuaCoreWrapper::oLua_SetField(L, -2, "count");

	LuaCoreWrapper::oLua_SetField(L, -2, "_raw");

	/*back to parent*/

	for(auto i = 0; i < augmentDataSize; i++)
	{
		if(i % 2 == 0)
		{
			if (augmentData[i] == 0)
			{
				break;
			}

			LuaCoreWrapper::oLua_PushNumber(L, augmentData[i]);
			LuaCoreWrapper::oLua_SetField(L, -2, std::format("Augment {} Id", i / 2 + 1).c_str());
		}
		else
		{
			LuaCoreWrapper::oLua_PushNumber(L, augmentData[i]);
			LuaCoreWrapper::oLua_SetField(L, -2, std::format("Augment {} Potency", i / 2 + 1).c_str());
		}
	}

	//ranks are hard capped at 30, there is a check for this in the game code that uses the current max rank if it's less than 31 or clamps it to 30 otherwise
	//how the game unpacks these
	/*const auto rank = augmentLookupItemData.ExtDataBlock2 >> 0x12 & 0x1f;
	const auto maxRank = ((augmentLookupItemData.ExtDataBlock2 >> 0x17 & 3) + 3) * 5;
	const auto path = augmentLookupItemData.ExtDataBlock2 & 3;
	const auto tnl = maxRank == rank ? 0 : pRpMap[rank] - (augmentLookupItemData.ExtDataBlock2 >> 2 & 0xffff);*/

	//no idea why there is so much empty space, maybe expansion room idk

	LuaCoreWrapper::oLua_PushString(L, PathMap[augmentData[77]]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Path");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[78]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Rank");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[79]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Tnl");

	LuaCoreWrapper::oLua_PushNumber(L, augmentData[80]);
	LuaCoreWrapper::oLua_SetField(L, -2, "Max Rank");
	
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