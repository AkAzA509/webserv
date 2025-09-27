# webserv

## Session management

The server now keeps track of per-client sessions using an HttpOnly cookie named `session_id`.

- Incoming requests reuse the `session_id` cookie when present. Missing or expired cookies trigger a new secure identifier that is returned via `Set-Cookie`.
- Session metadata (creation time, last activity, client IP and arbitrary key/value pairs) lives server-side in memory inside `SessionData`.
- Sessions expire after 30 minutes of inactivity and are cleaned automatically during request handling and timeout sweeps.
- Use `Server::setSessionValue` and `Server::getSessionValue` to persist custom state across requests, and `Server::touchSession` to prolong a session explicitly when needed.

Adjust the helpers in `srcs/parsing/Server.cpp` if you need different cookie names, lifetimes or storage semantics.
