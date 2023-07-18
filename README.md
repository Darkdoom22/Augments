# Augments

Currently supports equipment augment system 4 (Unity, Dynamis-D, Su5, JSE neck)

#Including the dll

```lua
local addonPath = windower.addon_path:gsub('\\', '/')
local dllPath = string.format('%s%s', addonPath, 'Augments.dll')
local AugmentInterface = assert(package.loadlib(dllPath, 'luaopen_Augments'))()
```

# Functions

```cpp
GetAugmentSystemFourData(uint32_t extDataBlock1, uint32_t extDataBlock2, uint32_t extDataBlock3, uint16_t itemId)
```

# Usage sample

```lua
local augmentRes = require('resources').augments

--find an item in your bag with system 4 augments and then..
local exDataBlock1, exDataBlock2, exDataBlock3 = item.extdata:unpack("III",1)
local augments = AugmentInterface.GetAugmentSystemFourData(exDataBlock1, exDataBlock2, exDataBlock3, item.id)

--this table contains the augment resource id and potency
print(augments["Augment 1 Id"])
print(augments["Augment 1 Potency"])
print(augmentRes[augments["Augment 1 Id"]].en)

--where you can directly access up to augments["Augment 4 Id"], etc

--or you can loop on the raw returned data 
for i=1, augments._raw.count, 1 do
  windower.add_to_chat(14, string.format("%d:[%d]", i, augments._raw[i])
end

--as well as
print(augments["Path"])
print(augments["Rank"])
print(augments["Max Rank"])
print(augments["Tnl"])
```


