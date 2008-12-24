/*
Copyright (C) 2008 Vana Development Team

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
#include "Pets.h"
#include "Database.h"
#include "Drops.h"
#include "DropsPacket.h"
#include "Inventory.h"
#include "InventoryPacket.h"
#include "Maps.h"
#include "Movement.h"
#include "PacketReader.h"
#include "PetsPacket.h"
#include "Player.h"
#include "Pos.h"
#include "Randomizer.h"
#include "Reactors.h"
#include "Timer/Time.h"
#include "Timer/Timer.h"
#include <functional>
#include <string>

using std::string;
using std::tr1::bind;

unordered_map<int32_t, PetInfo> Pets::petsInfo;
unordered_map<int32_t, unordered_map<int32_t, PetInteractInfo> > Pets::petsInteractInfo;

int16_t Pets::exps[29] = {1, 3, 6, 14, 31, 60, 108, 181, 287, 434, 632, 891, 1224, 1642, 2161, 2793, 3557, 4467, 5542, 6801, 8263, 9950, 11882, 14084, 16578, 19391, 22548, 26074, 30000};

/* Pet class */
Pet::Pet(Player *player, Item *item) : player(player), type(item->id), index(-1), name(Pets::petsInfo[type].name), level(1), closeness(0), fullness(100), summoned(false) {
	mysqlpp::Query query = Database::getCharDB().query();
	query << "INSERT INTO pets (name) VALUES ("<< mysqlpp::quote << this->name << ")";
	mysqlpp::SimpleResult res = query.execute();
	this->id = (int32_t) res.insert_id();
	item->petid = this->id;
}

Pet::Pet(Player *player, Item *item, int8_t index, string name, int8_t level, int16_t closeness, int8_t fullness, int8_t inventorySlot) :
	player(player),
	id(item->petid),
	type(item->id),
	index(index),
	name(name),
	level(level),
	closeness(closeness),
	fullness(fullness),
	inventorySlot(inventorySlot),
	summoned(false) {}

void Pet::levelUp() {
	level += 1;
	PetsPacket::levelUp(player, this);
}

void Pet::setName(const string &name) {
	this->name = name;
	PetsPacket::changeName(player, this);
	PetsPacket::updatePet(player, this);
}

void Pet::addCloseness(int16_t amount) {
	this->closeness += amount;
	if (this->closeness > 30000)
		this->closeness = 30000;

	while (this->closeness >= Pets::exps[level - 1] && level < 30) {
		levelUp();
	}
	PetsPacket::updatePet(player, this);
}

void Pet::modifyFullness(int8_t offset, bool sendPacket) {
	this->fullness += offset;

	if (this->fullness > 100)
		this->fullness = 100;
	else if (this->fullness < 0)
		this->fullness = 0;

	if (sendPacket)
		PetsPacket::updatePet(player, this);
}

void Pet::startTimer() {
	Timer::Id id(Timer::Types::PetTimer, getIndex(), 0); // The timer will automatically stop if another pet gets inserted into this index
	clock_t length = (6 - Pets::petsInfo[getType()].hunger) * 60000; // TODO: Better formula
	new Timer::Timer(bind(&Pet::modifyFullness, this, -1, true), id, player->getTimers(), 0, length);
}

/* Pets namespace */
void Pets::handle_movement(Player *player, PacketReader &packet) {
	int32_t petid = packet.getInt();
	Pet *pet = player->getPets()->getPet(petid);
	packet.skipBytes(8);
	Movement::parseMovement(pet, packet);
	packet.reset(10);
	PetsPacket::showMovement(player, pet, packet.getBuffer(), packet.getBufferLength() - 9);
}

void Pets::handle_chat(Player *player, PacketReader &packet) {
	int32_t petid = packet.getInt();
	packet.skipBytes(5);
	int8_t act = packet.getByte();
	string message = packet.getString();
	PetsPacket::showChat(player, player->getPets()->getPet(petid), message, act);
}

void Pets::handle_summon(Player *player, PacketReader &packet) {
	packet.skipBytes(4);
	int16_t slot = packet.getShort();
	bool master = packet.getByte() == 1;
	bool multipet = player->getSkills()->getSkillLevel(8) > 0;
	Pet *pet = player->getPets()->getPet(player->getInventory()->getItem(5, slot)->petid);
	pet->setPos(player->getPos());

	if (pet->isSummoned()) { // Removing a pet
		player->getPets()->setSummoned(0, pet->getIndex());
		if (pet->getIndex() == 0) {
			Timer::Id id(Timer::Types::PetTimer, pet->getIndex(), 0);
			player->getTimers()->removeTimer(id);
			if (!multipet)
				player->getPets()->setSummoned(0, 0);
		}
		for (int8_t i = (pet->getIndex() + 1); i < 3; i++) {
			if (Pet *move = player->getPets()->getSummoned(i)) {
				move->setIndex(i - 1);
				player->getPets()->setSummoned(move->getId(), i - 1);
				player->getPets()->setSummoned(0, i);
				if (i - 1 == 0) {
					pet->startTimer();
				}
			}
		}
		pet->setIndex(-1);
		pet->setSummoned(false);
		PetsPacket::petSummoned(player, pet);
	}
	else { // Summoning a Pet
		if (!multipet || master) {
			for (int8_t i = 2; i > 0; i--) {
				if (player->getPets()->getSummoned(i - 1) && !player->getPets()->getSummoned(i)) {
					Pet *move = player->getPets()->getSummoned(i - 1);
					player->getPets()->setSummoned(0, i - 1);
					player->getPets()->setSummoned(move->getId(), i);
					move->setIndex(i);
				}
			}
			pet->setIndex(0);
			pet->setSummoned(true);

			Pet *kicked = player->getPets()->getSummoned(0);
			if (!multipet && kicked != 0) {
				kicked->setIndex(-1);
				kicked->setSummoned(false);
				Timer::Id id(Timer::Types::PetTimer, kicked->getIndex(), 0);
				player->getTimers()->removeTimer(id);
				PetsPacket::petSummoned(player, pet, true);
			}
			else
				PetsPacket::petSummoned(player, pet);

			player->getPets()->setSummoned(pet->getId(), 0);
			pet->startTimer();
		}
		else {
			for (int8_t i = 0; i < 3; i++) {
				if (!player->getPets()->getSummoned(i)) {
					player->getPets()->setSummoned(pet->getId(), i);
					pet->setIndex(i);
					pet->setSummoned(true);
					PetsPacket::petSummoned(player, pet);
					pet->startTimer();
					break;
				}
			}
		}
	}
	PetsPacket::blankUpdate(player);
}

void Pets::handle_feed(Player *player, PacketReader &packet) {
	packet.skipBytes(4);
	int16_t slot = packet.getShort();
	int32_t item = packet.getInt();
	if (Pet *pet = player->getPets()->getSummoned(0)) {
		Inventory::takeItem(player, item, 1);

		bool success = (pet->getFullness() < 100);
		PetsPacket::showAnimation(player, pet, 1, success);
		if (success) {
			pet->modifyFullness(30, false);
			if (Randomizer::Instance()->randInt(99) < 60)
				pet->addCloseness(1);
		}
	}
	else {
		InventoryPacket::blankUpdate(player);
	}
}

void Pets::handle_command(Player *player, PacketReader &packet) {
	int32_t petid = packet.getInt();
	packet.skipBytes(5);
	int8_t act = packet.getByte();
	Pet *pet = player->getPets()->getPet(petid);
	bool success = (Randomizer::Instance()->randInt(100) < petsInteractInfo[pet->getType()][act].prob);
	if (success) {
		pet->addCloseness(petsInteractInfo[pet->getType()][act].increase);
	}
	PetsPacket::showAnimation(player, pet, act, success);
}

void Pets::changeName(Player *player, const string &name) {
	if (Pet *pet = player->getPets()->getSummoned(0)) {
		pet->setName(name);
	}
}

void Pets::showPets(Player *player) {
	for (int8_t i = 0; i < 3; i++) {
		if (Pet *pet = player->getPets()->getSummoned(i)) {
			if (!pet->isSummoned()) {
				if (pet->getIndex() == 0) {
					pet->startTimer();
				}
				pet->setSummoned(true);
			}
			pet->setPos(player->getPos());
			PetsPacket::petSummoned(player, pet, false, true);
		}
	}
	PetsPacket::updateSummonedPets(player);
}
