import json
import sqlite3
from character import Character
from items import *
import json

class CharacterDB(object):
    persistent_stats = ['name', 'color', 'portrait', 'level', 'physical', 'mental', 'social', 'hp', 'sp', 'mp']

    def __init__(self, filename="ttrpg.db"):
        self.connection = sqlite3.connect(filename)
        self._create_table()

    def close(self):
        if self.connection:
            self.connection.close()
        self.connection = None

    def __del__(self):
        self.close()

    def _create_table(self):
        cur = self.connection.cursor()
        cur.executescript(f"""
            BEGIN;
            CREATE TABLE IF NOT EXISTS character (Id INTEGER PRIMARY KEY, user, guild, {", ".join(self.persistent_stats)});
            CREATE TABLE IF NOT EXISTS inventory (character, item, properties, location);
            COMMIT;""")

    # TODO: move database character link to another file
    def store(self, guild, user, c):
        with self.connection as con:
            columns=dict(user=str(user), guild=str(guild))
            columns.update({stat:c[stat] for stat in self.persistent_stats})
            columns['id'] = c.id
            query = f"""REPLACE INTO character ({", ".join(columns)}) VALUES 
                ({", ".join(f":{p}" for p in columns)});"""
            cur = con.execute(query, columns)
            c.id = cur.lastrowid
            cur = self._clear_inventory(cur, c.id)
            self._store_inventory(cur, c.id, c)

    @staticmethod
    def _encode_location(item, c):
        if c.main_hand() == item:
            return "main"
        elif c.off_hand() == item:
            return "off"
        elif item in c.worn:
            return "worn"
        else:
            return None

    def _store_inventory(self, cursor, idx, c):
        cursor = self.connection.cursor()
        item_columns = [dict(item=type(item).__name__,
                             properties=json.dumps(props) if (props := item.properties()) else None,
                             location = self._encode_location(item, c)) for item in c.inventory]
        if not item_columns:
            return
        query = f"""INSERT INTO inventory (character, item, properties, location) VALUES 
                ({int(idx)}, :item, :properties, :location)"""
        return cursor.executemany(query, item_columns)

    def _clear_inventory(self, cursor, idx):
        return cursor.execute(f"""DELETE FROM inventory WHERE character==?""", [idx])

    def exists(self, guild, user, name):
            return self._find_character(guild, user, name) is not None

    def retrieve(self, guild, user, name=None):
        if user is None and name is None:
            raise RuntimeError("Can not retrieve a character without a name or a user")

        name_query=f"""AND name=:name COLLATE NOCASE""" if name is not None else ''
        user_query=f"""AND user=:user""" if user is not None else ''

        query = f"""SELECT Id, {", ".join(self.persistent_stats)} FROM character 
            WHERE guild=:guild {user_query} {name_query}
            ORDER BY id DESC LIMIT 1"""

        cursor = self.connection.execute(query, dict(user=str(user), guild=str(guild), name=str(name)))
        columns = [x[0] for x in cursor.description]
        if row := cursor.fetchone():
            record = {col: row[idx] for idx, col in enumerate(columns)}
            c = Character(**record)
            inventory = self._retrieve_inventory(cursor, record['Id'])

            c.inventory = [i for location_items in inventory.values() for i in location_items]
            c.worn = inventory.get('worn', [])
            c.held['main'] = inventory.get('main', [None])[0]
            c.held['off'] = inventory.get('off', [None])[0]
            return c
        else:
            return None

    item_types = {cls.__name__: cls for cls in (Weapon, RangedWeapon, Shield, Armor, Equipment)}

    @staticmethod
    def _create_item(item, properties):
        if item_type := CharacterDB.item_types.get(item):
            return item_type(**properties)
        else:
            raise ValueError(f"Item type '{item}' unsupported.")

    def _retrieve_inventory(self, cursor, idx):
        query = f"""SELECT item, properties, location FROM inventory WHERE character=:id"""
        response = cursor.execute(query, dict(id=idx))
        result = dict()
        while row:=response.fetchone():
            location = row[2]
            result[location] = result.get(location,[]) + [self._create_item(row[0], json.loads(row[1]) if row[1] else dict())]
        return result

    def _find_character(self, guild, user, name):
        user_query = f"""AND user=:user""" if user is not None else ''
        query = f"""SELECT Id FROM character WHERE guild=:guild {user_query} AND name=:name COLLATE NOCASE ORDER BY Id DESC LIMIT 1"""
        response = self.connection.execute(query, dict(user=str(user), guild=str(guild), name=str(name)))
        if result := response.fetchone():
            return result[0]
        else:
            return None

    def _delete_character(self, cursor, idx):
        return cursor.execute(f"""DELETE FROM character WHERE Id=?""", [idx])

    def _delete_inventory(self, cursor, idx):
        return cursor.execute(f"""DELETE FROM inventory WHERE character=?""", [idx])

    def user(self, guild, name):
        query = f"""SELECT user FROM character WHERE guild=:guild AND name=:name COLLATE NOCASE ORDER BY Id DESC LIMIT 1"""
        response = self.connection.execute(query, dict(guild=str(guild), name=str(name)))
        if result := response.fetchone():
            return result[0]
        else:
            return None

    def delete(self, guild, user, name):
        idx = self._find_character(guild, user, name)
        if idx is None:
            raise RuntimeError(f"There is no character named '{name}' to retire.")
        with self.connection as con:
            cursor = self._delete_character(con.cursor(), idx)
            self._delete_inventory(cursor, idx)
        return idx

