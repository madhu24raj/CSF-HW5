MS1:
Madhu: sender implementation
Ryan: reciever implementaion

Both did connection, any shared utils stuff

MS2:
Madhu: room class logic, user struct, server side loop
Ryan: MessageQueue, main Server setup, server-side receiver loop


Eventually your report about how you implemented thread synchronization
in the server should go here

Synchronization Report:

approach and threading model server uses a thread-per-client model; each client connection gets its own worker thread that runs until they disconnect. since multiple threads access shared data like the room list and message queues, we need synchronization to prevent race conditions.

shared data and critical sections we have 3 main areas that need locking:

A. The Room Map (Server class)

shared data: m_rooms (the map of room names to pointers)

critical section: find_or_create_room

primitive: m_lock (mutex)

reasoning: std::map isn't thread safe. if two threads try to add a room at the same time, it corrupts the map. we lock the whole lookup/creation process so it's atomic.

B. The Room Members List (Room class)

shared data: members (set of users)

critical section: add_member, remove_member, broadcast_message

primitive: lock (mutex)

reasoning: users join/leave randomly. if we are looping through members in broadcast_message and someone leaves (changing the set), the iterator invalidates and we crash. holding the lock during the whole loop prevents this.

C. The Message Queue (MessageQueue class)

shared data: m_messages (deque of message pointers)

critical section: enqueue and dequeue

primitive: m_lock (mutex) and m_avail (semaphore)

reasoning: standard producer-consumer pattern. mutex protects the deque from concurrent push/pop. semaphore is used to wake up the receiver thread so it doesn't have to busy-wait.

meeting synchronization requirements

no lost updates: locking the server map ensures we don't make duplicate rooms. locking the room list ensures every current user gets the message during a broadcast.

memory safety: messages are heap allocated. the receiver is responsible for deleting them after dequeueing to prevent leaks.

hazard avoidance (deadlocks and races) to stop deadlocks, we follow a strict order and keep critical sections small:

server lock scope: we only hold the server lock to find the room, then release it immediately. we never hold it while doing room operations.

lock hierarchy: the order is always Room Lock -> MessageQueue Lock. we never try to grab the Room lock if we already have the Queue lock. this one-way dependency stops cycles.

semaphore safety: inside dequeue, the sem_timedwait happens before we grab the mutex. this way we aren't sleeping while holding the lock, which would block senders.