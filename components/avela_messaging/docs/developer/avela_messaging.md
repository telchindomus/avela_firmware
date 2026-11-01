# `avela_messaging`: service requests and status

Messaging provides plain request types and statically allocated FreeRTOS queues. Keeping requests here lets a caller
ask a service to act without depending on its implementation component.

`Application` routes the Request variant to each service. Its alternatives include `PlayerRequest`,
`SetupNetworkRequest`, `AcceptUploadsRequest`, and `WeatherUpdateRequest`. Feature-specific fields retain their
meanings in the receiving component's reference.

## Request queues

A service owns `RequestQueue<RequestType, Capacity>`. `send()` may be called from another task; `take()` is used by
the service on the main loop. Values cross tasks as copies and must be trivially copyable.

The default capacity is eight items. Requests are taken in order. `send()` is nonblocking and returns false when
full; the caller must handle refusal rather than assume the action occurred. `take()` returns no value when the
queue is empty. Storage is created with xQueueCreateStatic.

Relative actions remain relative until processed. Two queued volume increments are two changes, rather than two
stale absolute values computed from the same displayed model.

## Status queues

`StatusQueue<StatusType>` holds one published value. `publish()` overwrites it only when the status changes.
`read()` uses a peek, so multiple frontend reads do not consume it. Status values must be trivially copyable; the
constructor publishes a default value.

Worker latest-value queues carry requests and reports; the worker receives and consumes the latest item. The
[radio reference](../../../avela_radio/docs/developer/avela_radio.md#tasks-and-diagnostics) describes those queues.

## Ownership

Queues are clients' interfaces to the service, not permission to modify its data model. The owning service takes
requests and publishes status on the main loop. Persistent configuration instead goes through
[Settings](../../../avela_settings/docs/developer/avela_settings.md). The
[architecture](../../../../docs/developer/ARCHITECTURE.md#persistent-settings-and-service-requests) explains how
these paths fit together.
