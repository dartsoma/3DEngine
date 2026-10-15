#include "event_core.h"
#include "simple_logger.h"

static void event_default_cleanup(void* data)
{
    free(data);
}
// why size_t
void event_system_init(EventSystem* system, size_t max_queue_size)
{
    if (!system) {
        slog("Event: Bad system - event_system_init");
        return;
    }

    memset(system, 0, sizeof(EventSystem));

    system->queue.head = NULL;
    system->queue.tail = NULL;
    system->queue.count = 0;
    system->queue.max_events = max_queue_size;

    system->registry = gfc_hashmap_new();
    system->listeners = gfc_hashmap_new();

    system->next_event_id = 1;
    system->next_type_id = 1;

    system->running = 1;
}

void event_system_shutdown(EventSystem* system)
{
    // recursive removal of events based on type

    system->queue.tail

    // free hashmap listeners

    // free hashmap internet

    // free system
}

int event_register_type(EventSystem* system,
    const char* name,
    size_t data_size,
    void (*cleanup)(void* data))
{
    if (!system || !name) {
        slog("Event: Bad system / name - event_register_type");
        return 0;
    }

    EventTypeRegistry* registry = malloc(sizeof(EventTypeRegistry));

    if (!registry) {
        slog("Event: Bad registry - event_register_type");
        return 0;
    }

    registry->type_id = system->next_type_id++;

    // check for buffer overflow
    if (strlen(registry->name) >= 50) {
        slog("Event: Buffer overflow - event_register_type");
        return 0;
    }

    strncpy(
        registry->name,
        name,
        sizeof(registry->name) - 1);

    registry->name[sizeof(registry->name) - 1] = '\0';

    registry->data_size = data_size;
    registry->data_free = cleanup;

    gfc_hashmap_insert(
        system->registry,
        registry->type_id,
        registry);

    return registry->type_id;
}
// fix
Uint8 event_add_listener(EventSystem* system,
    uint32_t event_type_id,
    EventCallback callback,
    void* user_data,
    int priority)
{

    if (!system || !callback)
        return 0;

    EventListener* listener = gfc_hashmap_get(
        system->listeners,
        event_type_id);



    return 1;
}

Event* event_create(EventSystem* system,
    uint32_t event_type_id,
    void* data,
    size_t data_size,
    uint32_t frames)
{
    if (!system || !data) {
        slog("Event: Bad system / data - event_create");
        return NULL;
    }

    EventTypeRegistry* registry = gfc_hashmap_get(
        system->registry,
        type_id);

    if (!registry) {
        slog("Event: Bad registry - event_create");
        return NULL;
    }

    if (data_size != registry->data_size) {
        slog("Event: Incorrect register data size - event_create");
        return NULL;
    }

    Event* event = malloc(sizeof(Event));

    if (!event) {
        slog("Event: Bad event - event_create");
        return NULL;
    }

    event->data.data = malloc(data_size);

    if (!event->data.data) {
        slog("Event: Bad event data - event_create");
        free(event);
        return NULL;
    }

    memcpy(
        event->data.data,
        data,
        data_size);

    event->id = system->next_event_id++;

    event->frame = frame;

    event->data.type_id = type_id;

    event->data.data_size = data_size;

    event->data.free = registry->data_free;

    event->next = NULL;

    return event;
}

static void event_destroy(Event* event)
{
    if (!event) {
        slog("Event: Bad event - event_destroy");
        return;
    }

    if (event->data.data) {
        if (event->data.free)
            event->data.free(event->data.data);
        else
            free(event->data.data);
    }

    free(event);
}

Uint8 event_push(EventSystem* system, Event* event)
{
    if (!system || !event) {
        slog("Event: Bad system/event - event_push");
        return 0;
    }

    EventQueue* queue = system->queue;

    if (queue->count >= queue->max_events) {
        slog("Event: Queue exceeds maxiumum - event_push");
        return 0;
    }

    if (!queue->head) {
        queue->head = event;
        queue->tail = event;
    } else {
        queue->tail->next = event;
        queue->tail = event;
    }

    queue->count++;

    return 1;
}

Uint8 event_emit(EventSystem* system,
    uint32_t event_type_id,
    void* data,
    size_t data_size)
{
    if (!system) {
        // will flood do not slog
        return 0;
    }

    if (!event) {
        slog("Event: Bad event - event_emit");
        return 0;
    }

    if (!event_push(system, event)) {
        event_destroy(event);
        return 0;
    }

    return 1;
}

// Brainstorm Bottom

void event_process_queue(EventSystem* system)
{
    if (!system) {
        // will flood logs if I add slog
        return 0;
    }

    EventQueue* queue = system->queue;

    while (queue->head) {
        Event* event = queue->head;
        queue->head = event->next;

        if (!queue->head)
            queue->tail = NULL;

        queue->count--;

        EventListenerList* list = gfc_hashmap_get(system->listeners, event->data.type_id);

        if (list) {
            for (size_t i = 0;
                i < list->count;
                i++) {
                EventListener* listener = &list->listeners[i];

                listener->callback(
                    event,
                    listener->context);
            }
        }
        event_destroy(event);
    }

    return 1;
}

Uint8 event_remove_listener(EventSystem* system,
    uint32_t event_type_id,
    EventCallback callback,
    void* user_data) {



    }

Uint8 event_get_type_id(EventSystem* system, const char* name) { }

Uint8 event_type_exists(EventSystem* system, Uint8 type_id) { }
