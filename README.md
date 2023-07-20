# Augments

Currently supports equipment augment system 4 (Unity, Dynamis-D, Su5, JSE neck)

# Including the dll

```lua
local addonPath = windower.addon_path:gsub('\\', '/')
local dllPath = string.format('%s%s', addonPath, 'Augments.dll')
local AugmentInterface = assert(package.loadlib(dllPath, 'luaopen_Augments'))()
```

# Functions

```cpp
GetAugmentSystemFourData(const char* extdata, uint16_t itemId)
```

# Usage sample

```lua
local augmentRes = require('resources').augments

--find an item in your bag with system 4 augments and then..
local augments = AugmentInterface.GetAugmentSystemFourData(item.extdata, item.id)

--this table contains the augment resource id and potency
print(augments["Augment 1 Id"])
print(augments["Augment 1 Potency"])
print(augmentRes[augments["Augment 1 Id"]].en)

--where you can access up to augments["Augment Count"] worth of entries

--additionally, you can determine how many augments are intended to be displayed on which line by accessing

print(augments["Line 1 Augment Count"])

--where you can access up to augments["Line Count"] worth of entries 

--as well as
print(augments["Path"])
print(augments["Rank"])
print(augments["Max Rank"])
print(augments["Tnl"])
print(augments["Main Hand"])

--for anything else, you can loop on the raw data this is all parsed from
for i=1, augments._raw.count, 1 do
  windower.add_to_chat(14, string.format("%d:[%d]", i, augments._raw[i])
end

```


