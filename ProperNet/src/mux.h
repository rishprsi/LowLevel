#ifndef PROPERNET_MUX_H
#define PROPERNET_MUX_H

/*
 * I/O multiplexing with poll(2): one thread, many clients.
 *
 * This is deliberately poll() and not epoll/kqueue — poll is portable POSIX
 * and is enough to learn the event-loop shape. (epoll belongs to the Linux
 * HTTP-server project later in Phase 4.)
 */

enum { MUX_MAX_CLIENTS = 64 };

/*
 * Serve an echo service on an already-listening socket, using a SINGLE
 * thread and poll(2). No fork, no threads, no blocking on any one client.
 *
 *  - Accept clients as they arrive, up to nclients_expected in total
 *    (stop poll()ing the listener once all expected clients were accepted).
 *  - Whenever any connected client has data, read one chunk and write that
 *    exact chunk back to THAT client (a full echo: handle partial writes).
 *  - When a client hits EOF (read returns 0), close its fd.
 *  - Return 0 once all nclients_expected clients have connected AND
 *    disconnected. Return -1 on invalid arguments (listen_fd < 0,
 *    nclients_expected < 1 or > MUX_MAX_CLIENTS) or any fatal error
 *    (poll/accept/read/write failure).
 *
 * Clients interleave: A may send while B is mid-conversation. Your loop must
 * never mix up whose bytes are whose — the tests run 3 concurrent clients
 * that each verify they get back exactly their own bytes, in order.
 */
int echo_poll_serve(int listen_fd, int nclients_expected);

#endif /* PROPERNET_MUX_H */
