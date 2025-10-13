#include "valheim.map.h"
#include "valheim.memory.h"

b8 valheim_initMap( valheim_Map *map, u64 stride, u64 capacity, valheim_MapHash hash, valheim_Allocator *allocator ) {
	map->size = 0;
	map->capacity = capacity;
	map->stride = stride;
	map->buckets = valheim_allocate( allocator, sizeof( valheim_MapNode *) * capacity );
	map->hash = hash;
	return map->buckets != NULL;
}

void valheim_deinitMap( valheim_Map *map, valheim_Allocator *allocator ) {
	for ( u64 iBucket = 0; iBucket < map->capacity; ++iBucket ) {
		if ( map->buckets[ iBucket ] ) {
			valheim_free( allocator, map->buckets[ iBucket ]->data );
			valheim_free( allocator, map->buckets[ iBucket ] );
		}
	}

	valheim_free( allocator, map->buckets );
}

b8 valheim_mapInsert( valheim_Map *map, const void *key, u64 keySize, const void *value, valheim_Allocator *allocator ) {
	const u64 k = map->hash( key, keySize, 0 ) % map->capacity;

	valheim_MapNode *newNode = valheim_allocate( allocator, sizeof *newNode );
	newNode->keyStride = keySize;
	newNode->data = valheim_allocate( allocator, keySize + map->stride );
	
	valheim_copyMemory( ( u8 * ) newNode->data, key, keySize );
	valheim_copyMemory( ( u8 * ) newNode->data + keySize, value, map->stride );

	if ( map->buckets[ k ] == NULL ) {
		map->buckets[ k ] = newNode;
		map->size++;
	} else {
	}

	return true;
}

b8 valheim_mapFind( valheim_Map *map, const void *key, u64 keySize, void *value ) {
	const u64 k = map->hash( key, keySize, 0 ) % map->capacity;
	if ( map->buckets[ k ] != NULL ) {
		valheim_copyMemory( value, ( u8 * ) map->buckets[ k ]->data + keySize, map->stride );
		return true;
	}

	return false;
}

b8 valheim_mapUpdate( valheim_Map *map, const void *key, u64 keySize, const void *value ) {
	const u64 k = map->hash( key, keySize, 0 ) % map->capacity;

	if ( map->buckets[ k ] != NULL ) {
		valheim_copyMemory( ( u8 * ) map->buckets[ k ]->data, value, map->stride );
		return true;
	}

	return false;
}