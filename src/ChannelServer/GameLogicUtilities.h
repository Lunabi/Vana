/*
Copyright (C) 2008-2009 Vana Development Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef GAMELOGICUTILITIES_H
#define GAMELOGICUTILITIES_H

#include "GameConstants.h"

namespace GameLogicUtilities {
	inline uint8_t getInventory(int32_t itemid) { return static_cast<uint8_t>(itemid / 1000000); }
	inline int32_t getItemType(int32_t itemid) { return (itemid / 10000); }
	inline bool isArrow(int32_t itemid) { return (getItemType(itemid) == 206); }
	inline bool isStar(int32_t itemid) { return (getItemType(itemid) == 207); }
	inline bool isBullet(int32_t itemid) { return (getItemType(itemid) == 233); }
	inline bool isRechargeable(int32_t itemid) { return (isBullet(itemid) || isStar(itemid)); }
	inline bool isEquip(int32_t itemid) { return (getInventory(itemid) == 1); }
	inline bool isPet(int32_t itemid) { return (itemid >= 5000000 && itemid <= 5000053); }
	inline bool isOverall(int32_t itemid) { return (getItemType(itemid) == 105); }
	inline bool is2hWeapon(int32_t itemid) { return (getItemType(itemid) / 10 == 14); }

	inline bool isBeginnerSkill(int32_t skillid) { return (skillid / 1000000 == 0); }
	inline bool isFourthJobSkill(int32_t skillid) { return ((skillid / 10000) % 10 == 2); }
	inline bool isSummon(int32_t skillid) { return (skillid == Jobs::Sniper::Puppet  || skillid == Jobs::Ranger::Puppet || skillid == Jobs::Priest::Summon_Dragon || skillid == Jobs::Ranger::Silver_Hawk || skillid == Jobs::Sniper::Golden_Eagle || skillid == Jobs::DarkKnight::Beholder || skillid == Jobs::FPArchMage::Elquines || skillid == Jobs::ILArchMage::Ifrit || skillid == Jobs::Bishop::Bahamut || skillid == Jobs::Bowmaster::Phoenix || skillid == Jobs::Marksman::Frostprey || skillid == Jobs::Outlaw::Octopus || skillid == Jobs::Outlaw::Gaviota); }
	inline bool isPuppet(int32_t skillid) { return (skillid == Jobs::Sniper::Puppet || skillid == Jobs::Ranger::Puppet); }
};

#endif