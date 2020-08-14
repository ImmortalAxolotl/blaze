#ifndef CODEC_H
#define CODEC_H

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#define ARRAY_SIZE(x) (sizeof (x) / sizeof *(x))

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ABS(a) ((a) < 0 ? -(a) : (a))

#define CLAMP(x, l, u) (MAX(MIN((x), (u)), (l)))

#define PI (3.141592653589f)

#define DEGREES_PER_RADIAN (360 / (2 * PI))

typedef int8_t mc_byte;
typedef int16_t mc_short;
typedef int32_t mc_int;
typedef int64_t mc_long;
typedef uint8_t mc_ubyte;
typedef uint16_t mc_ushort;
typedef uint32_t mc_uint;
typedef uint64_t mc_ulong;
typedef float mc_float;
typedef double mc_double;

typedef struct {
    unsigned char * ptr;
    mc_int size;
    mc_int index;
} memory_arena;

typedef struct {
    mc_int x;
    mc_int y;
    mc_int z;
} net_block_pos;

typedef struct {
    // @TODO(traks) could the compiler think that the buffer points to some
    // buffer that contains this struct itself, meaning it has to reload fields
    // after we write something to it?
    unsigned char * buf;
    int limit;
    int index;
    int error;
} buffer_cursor;

#define NET_STRING(x) ((net_string) {.size = sizeof (x) - 1, .ptr = (x)})

typedef struct {
    mc_int size;
    void * ptr;
} net_string;

enum nbt_tag {
    NBT_TAG_END,
    NBT_TAG_BYTE,
    NBT_TAG_SHORT,
    NBT_TAG_INT,
    NBT_TAG_LONG,
    NBT_TAG_FLOAT,
    NBT_TAG_DOUBLE,
    NBT_TAG_BYTE_ARRAY,
    NBT_TAG_STRING,
    NBT_TAG_LIST,
    NBT_TAG_COMPOUND,
    NBT_TAG_INT_ARRAY,
    NBT_TAG_LONG_ARRAY,

    // our own tags for internal use
    NBT_TAG_LIST_END,
    NBT_TAG_COMPOUND_IN_LIST,
    NBT_TAG_LIST_IN_LIST,
};

typedef union {
    struct {
        mc_uint buffer_index:22;
        mc_uint tag:5;
        mc_uint element_tag:5;
    };
    mc_uint next_compound_entry_offset;
    mc_uint list_size;
} nbt_tape_entry;

#define MAX_CHUNK_CACHE_RADIUS (10)

#define MAX_CHUNK_CACHE_DIAM (2 * MAX_CHUNK_CACHE_RADIUS + 1)

#define KEEP_ALIVE_SPACING (10 * 20)

#define KEEP_ALIVE_TIMEOUT (30 * 20)

#define MAX_CHUNK_SENDS_PER_TICK (2)

#define MAX_CHUNK_LOADS_PER_TICK (2)

// must be power of 2
#define MAX_ENTITIES (1024)

#define MAX_PLAYERS (100)

// in network id order
enum gamemode {
    GAMEMODE_SURVIVAL,
    GAMEMODE_CREATIVE,
    GAMEMODE_ADVENTURE,
    GAMEMODE_SPECTATOR,
};

// @NOTE(traks) I think of the Minecraft coordinate system as follows:
//
//        +Y
//        |
//        |
//        *---- +X (270 degrees)
//       /
//      /
//     +Z (0 degrees)
//
// Then north naturally corresponds to -Z, east to +X, etc. However, entity
// rotations along the Y axis are the opposite to what you might expect: adding
// degrees rotates clockwise instead of counter-clockwise (as is common in
// mathematics).

// in network id order
enum direction {
    DIRECTION_NEG_Y, // down
    DIRECTION_POS_Y, // up
    DIRECTION_NEG_Z, // north
    DIRECTION_POS_Z, // south
    DIRECTION_NEG_X, // west
    DIRECTION_POS_X, // east
};

typedef struct {
    mc_short x;
    mc_short z;
} chunk_pos;

#define CHUNK_LOADED (1u << 0)

typedef struct {
    int index_in_bucket;
    mc_ushort block_states[4096];
} chunk_section;

#define CHUNK_SECTIONS_PER_BUCKET (64)

typedef struct chunk_section_bucket chunk_section_bucket;

struct chunk_section_bucket {
    chunk_section chunk_sections[CHUNK_SECTIONS_PER_BUCKET];
    // @TODO(traks) we use 2 * CHUNK_SECTIONS_PER_BUCKET 4096-byte pages for the
    // block states in the chunk sections. How much of the next page do we use?
    chunk_section_bucket * next;
    chunk_section_bucket * prev;
    int used_sections;
    // @TODO(traks) store this in longs?
    unsigned char used_map[CHUNK_SECTIONS_PER_BUCKET];
};

typedef struct {
    mc_ushort x:4;
    mc_ushort y:8;
    mc_ushort z:4;
} compact_chunk_block_pos;

typedef struct {
    chunk_section * sections[16];
    mc_ushort non_air_count[16];
    // need shorts to store 257 different heights
    mc_ushort motion_blocking_height_map[256];

    // increment if you want to keep a chunk available in the map, decrement
    // if you no longer care for the chunk.
    // If = 0 the chunk will be removed from the map at some point.
    mc_uint available_interest;
    unsigned flags;

    // @TODO(traks) more changed blocks, better compression. Figure out when
    // this limit can be exceeded. I highly doubt more than 16 blocks will be
    // changed per chunk per tick due to players except if player density is
    // very high.
    compact_chunk_block_pos changed_blocks[16];
    mc_ubyte changed_block_count;
} chunk;

#define CHUNKS_PER_BUCKET (32)

#define CHUNK_MAP_SIZE (1024)

typedef struct chunk_bucket chunk_bucket;

struct chunk_bucket {
    chunk_bucket * next_bucket;
    int size;
    chunk_pos positions[CHUNKS_PER_BUCKET];
    chunk chunks[CHUNKS_PER_BUCKET];
};

typedef struct {
    unsigned char value_count;
    // name size, name, value size, value, value size, value, etc.
    unsigned char tape[255];
} block_property_spec;

typedef struct {
    mc_ushort base_state;
    unsigned char property_count;
    unsigned char property_specs[8];
    unsigned char default_value_indices[8];
} block_properties;

typedef struct {
    mc_ubyte max_stack_size;
} item_type;

typedef struct {
    mc_int type;
    mc_ubyte size;
} item_stack;

// in network id order
enum entity_type {
    ENTITY_AREA_EFFECT_CLOUD,
    ENTITY_ARMOR_STAND,
    ENTITY_ARROW,
    ENTITY_BAT,
    ENTITY_BEE,
    ENTITY_BLAZE,
    ENTITY_BOAT,
    ENTITY_CAT,
    ENTITY_CAVE_SPIDER,
    ENTITY_CHICKEN,
    ENTITY_COD,
    ENTITY_COW,
    ENTITY_CREEPER,
    ENTITY_DOLPHIN,
    ENTITY_DONKEY,
    ENTITY_DRAGON_FIREBALL,
    ENTITY_DROWNED,
    ENTITY_ELDER_GUARDIAN,
    ENTITY_END_CRYSTAL,
    ENTITY_ENDER_DRAGON,
    ENTITY_ENDERMAN,
    ENTITY_ENDERMITE,
    ENTITY_EVOKER,
    ENTITY_EVOKER_FANGS,
    ENTITY_EXPERIENCE_ORB,
    ENTITY_EYE_OF_ENDER,
    ENTITY_FALLING_BLOCK,
    ENTITY_FIREWORK_ROCKET,
    ENTITY_FOX,
    ENTITY_GHAST,
    ENTITY_GIANT,
    ENTITY_GUARDIAN,
    ENTITY_HOGLIN,
    ENTITY_HORSE,
    ENTITY_HUSK,
    ENTITY_ILLUSIONER,
    ENTITY_IRON_GOLEM,
    ENTITY_ITEM,
    ENTITY_ITEM_FRAME,
    ENTITY_FIREBALL,
    ENTITY_LEASH_KNOT,
    ENTITY_LIGHTNING_BOLT,
    ENTITY_LLAMA,
    ENTITY_LLAMA_SPIT,
    ENTITY_MAGMA_CUBE,
    ENTITY_MINECART,
    ENTITY_CHEST_MINECART,
    ENTITY_COMMAND_BLOCK_MINECART,
    ENTITY_FURNACE_MINECART,
    ENTITY_HOPPER_MINECART,
    ENTITY_SPAWNER_MINECART,
    ENTITY_TNT_MINECART,
    ENTITY_MULE,
    ENTITY_MOOSHROOM,
    ENTITY_OCELOT,
    ENTITY_PAINTING,
    ENTITY_PANDA,
    ENTITY_PARROT,
    ENTITY_PHANTOM,
    ENTITY_PIG,
    ENTITY_PIGLIN,
    ENTITY_PIGLIN_BRUTE,
    ENTITY_PILLAGER,
    ENTITY_POLAR_BEAR,
    ENTITY_TNT,
    ENTITY_PUFFERFISH,
    ENTITY_RABBIT,
    ENTITY_RAVAGER,
    ENTITY_SALMON,
    ENTITY_SHEEP,
    ENTITY_SHULKER,
    ENTITY_SHULKER_BULLET,
    ENTITY_SILVERFISH,
    ENTITY_SKELETON,
    ENTITY_SKELETON_HORSE,
    ENTITY_SLIME,
    ENTITY_SMALL_FIREBALL,
    ENTITY_SNOW_GOLEM,
    ENTITY_SNOWBALL,
    ENTITY_SPECTRAL_ARROW,
    ENTITY_SPIDER,
    ENTITY_SQUID,
    ENTITY_STRAY,
    ENTITY_STRIDER,
    ENTITY_EGG,
    ENTITY_ENDER_PEARL,
    ENTITY_EXPERIENCE_BOTTLE,
    ENTITY_POTION,
    ENTITY_TRIDENT,
    ENTITY_TRADER_LLAMA,
    ENTITY_TROPICAL_FISH,
    ENTITY_TURTLE,
    ENTITY_VEX,
    ENTITY_VILLAGER,
    ENTITY_VINDICATOR,
    ENTITY_WANDERING_TRADER,
    ENTITY_WITCH,
    ENTITY_WITHER,
    ENTITY_WITHER_SKELETON,
    ENTITY_WITHER_SKULL,
    ENTITY_WOLF,
    ENTITY_ZOGLIN,
    ENTITY_ZOMBIE,
    ENTITY_ZOMBIE_HORSE,
    ENTITY_ZOMBIE_VILLAGER,
    ENTITY_ZOMBIFIED_PIGLIN,
    ENTITY_PLAYER,
    ENTITY_FISHING_BOBBER,
    ENTITY_NULL, // not used in vanilla
};

#define ENTITY_INDEX_MASK (MAX_ENTITIES - 1)

// Top 12 bits are used for the generation, lowest 20 bits can be used for the
// index into the entity table. Bits actually used for the index depends on
// MAX_ENTITIES.
static_assert(MAX_ENTITIES <= (1UL << 20), "MAX_ENTITIES too large");
typedef mc_uint entity_id;

// Player inventory slots are indexed as follows:
//
//  0           the crafting grid result slot
//  1-4         the 2x2 crafting grid slots
//  5-8         the 4 armour slots
//  9-35        the 36 main inventory slots
//  36-44       hotbar slots
//  45          off hand slot
//
// Here are some defines for convenience.
#define PLAYER_SLOTS (46)
#define PLAYER_FIRST_HOTBAR_SLOT (36)
#define PLAYER_LAST_HOTBAR_SLOT (44)
#define PLAYER_OFF_HAND_SLOT (45)

typedef struct {
    unsigned char username[16];
    int username_size;

    item_stack slots_prev_tick[PLAYER_SLOTS];
    item_stack slots[PLAYER_SLOTS];
    static_assert(PLAYER_SLOTS <= 64, "Too many player slots");
    mc_ulong slots_needing_update;
    unsigned char selected_slot;

    unsigned char gamemode;

    // @NOTE(traks) the server doesn't tell clients the body rotation of
    // players. The client determines the body rotation based on the player's
    // movement and their head rotation. However, we do need to send a players
    // head rotation using the designated packet, otherwise heads won't rotate.
    // @NOTE(traks) these values shouldn't exceed the range [0, 360] by too
    // much, otherwise float -> integer conversion errors may occur.
    mc_float head_rot_x;
    mc_float head_rot_y;
} player_data;

typedef struct {
    item_stack contents;
} item_data;

#define ENTITY_IN_USE ((unsigned) (1 << 0))

#define ENTITY_TELEPORTING ((unsigned) (1 << 1))

#define ENTITY_ON_GROUND ((unsigned) (1 << 2))

typedef struct {
    mc_double x;
    mc_double y;
    mc_double z;
    entity_id eid;
    unsigned flags;
    unsigned type;

    union {
        player_data player;
        item_data item;
    };
} entity_data;

#define PLAYER_BRAIN_IN_USE ((unsigned) (1 << 0))

#define PLAYER_BRAIN_DID_INIT_PACKETS ((unsigned) (1 << 1))

#define PLAYER_BRAIN_SENT_TELEPORT ((unsigned) (1 << 2))

#define PLAYER_BRAIN_GOT_ALIVE_RESPONSE ((unsigned) (1 << 3))

#define PLAYER_BRAIN_SHIFTING ((unsigned) (1 << 4))

#define PLAYER_BRAIN_SPRINTING ((unsigned) (1 << 5))

#define PLAYER_BRAIN_INITIALISED_TAB_LIST ((unsigned) (1 << 6))

typedef struct {
    unsigned char sent;
} chunk_cache_entry;

typedef struct {
    int sock;
    unsigned flags;
    unsigned char rec_buf[65536];
    int rec_cursor;

    unsigned char send_buf[1048576];
    int send_cursor;

    // The radius of the client's view distance, excluding the centre chunk,
    // and including an extra outer rim the client doesn't render but uses
    // for connected blocks and such.
    int chunk_cache_radius;
    mc_short chunk_cache_centre_x;
    mc_short chunk_cache_centre_z;
    int new_chunk_cache_radius;
    // @TODO(traks) maybe this should just be a bitmap
    chunk_cache_entry chunk_cache[MAX_CHUNK_CACHE_DIAM * MAX_CHUNK_CACHE_DIAM];

    mc_int current_teleport_id;

    unsigned char language[16];
    int language_size;
    mc_int chat_visibility;
    mc_ubyte sees_chat_colours;
    mc_ubyte model_customisation;
    mc_int main_hand;

    mc_ulong last_keep_alive_sent_tick;

    entity_id eid;

    // @TODO(traks) this feels a bit silly, but very simple
    entity_id tracked_entities[MAX_ENTITIES];
} player_brain;

typedef struct {
    mc_ushort size;
    unsigned char text[512];
} global_msg;

typedef struct {
    entity_id eid;
} tab_list_entry;

typedef struct {
    // index into string buffer for name size + value
    int name_index;
    int value_count;
    // index into value id buffer for array of values
    int values_index;
} tag_spec;

typedef struct {
    int size;
    tag_spec tags[128];
} tag_list;

#define RESOURCE_LOC_MAX_SIZE (256)

typedef struct {
    unsigned char size;
    mc_ushort id;
    mc_uint buf_index;
} resource_loc_entry;

typedef struct {
    mc_int size_mask;
    mc_int string_buf_size;
    resource_loc_entry * entries;
    unsigned char * string_buf;
    mc_int last_string_buf_index;
} resource_loc_table;

// Currently dimension types have the following configurable properties that are
// shared with the client. These have the effects:
//
//  - fixed_time (optional long): time of day always equals this
//  - has_skylight (bool): sky light levels, whether it can
//    thunder, whether daylight sensors work, phantom spawning
//  - has_ceiling (bool): affects thunder, map rendering, mob
//    spawning algorithm, respawn logic
//  - ultrawarm (bool): whether water can be placed, affects ice
//    melting, and how far and how fast lava flows
//  - natural (bool): whether players can sleep and whether
//    zombified piglin can spawn from portals
//  - coordinate_scale (double): vanilla overworld has 1 and vanilla
//    nether has 8. affects teleporting between worlds
//  - piglin_safe (bool): false if piglins convert to zombified
//    piglins as in the vanilla overworld
//  - bed_works (bool): true if beds can set spawn point. else beds will
//    explode when used
//  - respawn_anchor_works (bool): true if respawn anchors can
//    set spawn point. else they explode when used
//  - has_raids (bool): whether raids spawn
//  - logical_height (int in [0, 256]): seems to only affect
//    chorus fruit teleportation and nether portal spawning, not
//    the actual maximum world height
//  - infiniburn (resource loc): the resource location of a
//    block tag that is used to check whether fire should keep
//    burning forever on tagged blocks
//  - effects (resource loc): affects cloud height, fog, sky colour, etc. for
//    the client
//  - ambient_light (float): affects brightness visually and some mob AI

#define DIMENSION_HAS_SKYLIGHT ((unsigned) (0x1 << 0))

#define DIMENSION_HAS_CEILING ((unsigned) (0x1 << 1))

#define DIMENSION_ULTRAWARM ((unsigned) (0x1 << 2))

#define DIMENSION_NATURAL ((unsigned) (0x1 << 3))

#define DIMENSION_PIGLIN_SAFE ((unsigned) (0x1 << 4))

#define DIMENSION_BED_WORKS ((unsigned) (0x1 << 5))

#define DIMENSION_RESPAWN_ANCHOR_WORKS ((unsigned) (0x1 << 6))

#define DIMENSION_HAS_RAIDS ((unsigned) (0x1 << 7))

typedef struct {
    unsigned char name[64];
    unsigned char name_size;
    mc_long fixed_time; // -1 if not used
    unsigned flags;
    mc_double coordinate_scale;
    mc_int logical_height;
    unsigned char infiniburn[128];
    unsigned char infiniburn_size;
    unsigned char effects[128];
    unsigned char effects_size;
    mc_float ambient_light;
} dimension_type;

enum biome_precipitation {
    BIOME_PRECIPITATION_NONE,
    BIOME_PRECIPITATION_RAIN,
    BIOME_PRECIPITATION_SNOW,
};

enum biome_category {
    BIOME_CATEGORY_NONE,
    BIOME_CATEGORY_TAIGA,
    BIOME_CATEGORY_EXTREME_HILLS,
    BIOME_CATEGORY_JUNGLE,
    BIOME_CATEGORY_MESA,
    BIOME_CATEGORY_PLAINS,
    BIOME_CATEGORY_SAVANNA,
    BIOME_CATEGORY_ICY,
    BIOME_CATEGORY_THE_END,
    BIOME_CATEGORY_BEACH,
    BIOME_CATEGORY_FOREST,
    BIOME_CATEGORY_OCEAN,
    BIOME_CATEGORY_DESERT,
    BIOME_CATEGORY_RIVER,
    BIOME_CATEGORY_SWAMP,
    BIOME_CATEGORY_MUSHROOM,
    BIOME_CATEGORY_NETHER,
};

enum biome_temperature_modifier {
    BIOME_TEMPERATURE_MOD_NONE,
    BIOME_TEMPERATURE_MOD_FROZEN,
};

enum biome_grass_colour_modifier {
    BIOME_GRASS_COLOUR_MOD_NONE,
    BIOME_GRASS_COLOUR_MOD_DARK_FOREST,
    BIOME_GRASS_COLOUR_MOD_SWAMP,
};

// @TODO(traks) description of all fields in the biome struct

typedef struct {
    unsigned char name[64];
    unsigned char name_size;
    unsigned char precipitation;
    unsigned char category;
    mc_float temperature;
    mc_float downfall;
    unsigned temperature_mod;
    mc_float depth;
    mc_float scale;

    mc_int fog_colour;
    mc_int water_colour;
    mc_int water_fog_colour;
    mc_int sky_colour;
    mc_int foliage_colour_override; // -1 if not used
    mc_int grass_colour_override; // -1 if not used
    unsigned char grass_colour_mod;

    // @TODO(traks) complex ambient particle settings

    unsigned char ambient_sound[64];
    unsigned char ambient_sound_size;

    unsigned char mood_sound[64];
    unsigned char mood_sound_size;
    mc_int mood_sound_tick_delay;
    mc_int mood_sound_block_search_extent;
    mc_double mood_sound_offset;

    unsigned char additions_sound[64];
    unsigned char additions_sound_size;
    mc_double additions_sound_tick_chance;

    unsigned char music_sound[64];
    unsigned char music_sound_size;
    mc_int music_min_delay;
    mc_int music_max_delay;
    mc_ubyte music_replace_current_music;
} biome;

typedef struct {
    unsigned long long current_tick;

    entity_data entities[MAX_ENTITIES];
    mc_ushort next_entity_generations[MAX_ENTITIES];
    mc_int entity_count;

    player_brain player_brains[MAX_PLAYERS];
    int player_brain_count;

    // All chunks that should be loaded. Stored in a request list to allow for
    // ordered loads. If a
    // @TODO(traks) appropriate size
    chunk_pos chunk_load_requests[64];
    int chunk_load_request_count;

    // global messages for the current tick
    global_msg global_msgs[16];
    int global_msg_count;

    item_type item_types[1000];
    int item_type_count;

    void * short_lived_scratch;
    mc_int short_lived_scratch_size;

    tab_list_entry tab_list_added[64];
    int tab_list_added_count;
    tab_list_entry tab_list_removed[64];
    int tab_list_removed_count;
    tab_list_entry tab_list[MAX_PLAYERS];
    int tab_list_size;

    tag_list block_tags;
    tag_list entity_tags;
    tag_list fluid_tags;
    tag_list item_tags;
    int tag_name_count;
    int tag_value_id_count;
    unsigned char tag_name_buf[1 << 12];
    mc_ushort tag_value_id_buf[1 << 12];

    resource_loc_table block_resource_table;
    resource_loc_table item_resource_table;
    resource_loc_table entity_resource_table;
    resource_loc_table fluid_resource_table;

    dimension_type dimension_types[32];
    int dimension_type_count;

    biome biomes[128];
    int biome_count;
} server;

void
logs(void * format, ...);

void
logs_errno(void * format);

void *
alloc_in_arena(memory_arena * arena, mc_int size);

mc_int
net_read_varint(buffer_cursor * cursor);

void
net_write_varint(buffer_cursor * cursor, mc_int val);

int
net_varint_size(mc_int val);

mc_long
net_read_varlong(buffer_cursor * cursor);

void
net_write_varlong(buffer_cursor * cursor, mc_long val);

mc_int
net_read_int(buffer_cursor * cursor);

void
net_write_int(buffer_cursor * cursor, mc_int val);

mc_short
net_read_short(buffer_cursor * cursor);

void
net_write_short(buffer_cursor * cursor, mc_short val);

mc_byte
net_read_byte(buffer_cursor * cursor);

void
net_write_byte(buffer_cursor * cursor, mc_byte val);

mc_ushort
net_read_ushort(buffer_cursor * cursor);

void
net_write_ushort(buffer_cursor * cursor, mc_ushort val);

mc_ulong
net_read_ulong(buffer_cursor * cursor);

void
net_write_ulong(buffer_cursor * cursor, mc_ulong val);

mc_uint
net_read_uint(buffer_cursor * cursor);

void
net_write_uint(buffer_cursor * cursor, mc_uint val);

mc_ubyte
net_read_ubyte(buffer_cursor * cursor);

void
net_write_ubyte(buffer_cursor * cursor, mc_ubyte val);

net_string
net_read_string(buffer_cursor * cursor, mc_int max_size);

void
net_write_string(buffer_cursor * cursor, net_string val);

mc_float
net_read_float(buffer_cursor * cursor);

void
net_write_float(buffer_cursor * cursor, mc_float val);

mc_double
net_read_double(buffer_cursor * cursor);

void
net_write_double(buffer_cursor * cursor, mc_double val);

net_block_pos
net_read_block_pos(buffer_cursor * cursor);

void
net_write_data(buffer_cursor * cursor, void * restrict src, size_t size);

nbt_tape_entry *
nbt_move_to_key(net_string matcher, nbt_tape_entry * tape,
        buffer_cursor * cursor);

net_string
nbt_get_string(net_string matcher, nbt_tape_entry * tape,
        buffer_cursor * cursor);

nbt_tape_entry *
nbt_get_compound(net_string matcher, nbt_tape_entry * tape,
        buffer_cursor * cursor);

nbt_tape_entry *
load_nbt(buffer_cursor * cursor, memory_arena * arena, int max_level);

void
print_nbt(nbt_tape_entry * tape, buffer_cursor * cursor,
        memory_arena * arena, int max_levels);

void
begin_timed_block(char * name);

void
end_timed_block();

int
find_property_value_index(block_property_spec * prop_spec, net_string val);

chunk *
get_or_create_chunk(chunk_pos pos);

chunk *
get_chunk_if_loaded(chunk_pos pos);

chunk *
get_chunk_if_available(chunk_pos pos);

void
chunk_set_block_state(chunk * ch, int x, int y, int z, mc_ushort block_state);

mc_ushort
chunk_get_block_state(chunk * ch, int x, int y, int z);

void
try_read_chunk_from_storage(chunk_pos pos, chunk * ch,
        memory_arena * scratch_arena,
        block_properties * block_properties_table,
        block_property_spec * block_property_specs,
        resource_loc_table * block_resource_table);

chunk_section *
alloc_chunk_section(void);

void
free_chunk_section(chunk_section * section);

void
clean_up_unused_chunks(void);

entity_data *
resolve_entity(server * serv, entity_id eid);

entity_data *
try_reserve_entity(server * serv, unsigned type);

void
evict_entity(server * serv, entity_id eid);

void
teleport_player(player_brain * brain, entity_data * entity,
        mc_double new_x, mc_double new_y, mc_double new_z,
        mc_float new_rot_x, mc_float new_rot_y);

void
tick_player_brain(player_brain * brain, server * serv,
        memory_arena * tick_arena);

void
send_packets_to_player(player_brain * brain, server * serv,
        memory_arena * tick_arena);

mc_short
resolve_resource_loc_id(net_string resource_loc, resource_loc_table * table);

int
net_string_equal(net_string a, net_string b);

#endif
