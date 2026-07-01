#ifndef PROPERC_TODO_H
#define PROPERC_TODO_H

/*
 * TODO(msg) — placeholder that makes an unimplemented function a HARD COMPILE
 * ERROR until you replace it with a real implementation.
 *
 * Usage inside a stub body:
 *     int my_fn(int x) {
 *         TODO("implement my_fn");
 *     }
 *
 * Delete the TODO(...) line and write your code. The message tells you (and
 * the compiler) exactly what is missing.
 */
#define TODO(msg) _Static_assert(0, "TODO: " msg)

#endif /* PROPERC_TODO_H */
