#ifndef __EVENT_CORE_H__
#define __EVENT_CORE_H__

#include "gfc_hashmap.h"

typedef uint32_t EventTypeID;

// Holds info and type
typedef struct Event {
  void *event_data;
  EventTypeID type;

} Event;

// Will hold the linked list of events to queue and process
typedef struct {

} EventQueue;

// Stores an event and how to free it
typedef struct {

  EventTypeID id;

  size_t data_size;

  void (*free_data)(void *data);

} EventTypeCatalog;

// Takes a reaction and a command
typedef void (*EventCallback)(Event *event, void *context);

// Wait for emission and then reacts

typedef struct {

  EventTypeID type;
  EventCallback callback;
  void *context;

} EventListener;

#endif
