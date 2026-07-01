//! A thread pool with graceful shutdown — the classic design from
//! The Book, chapter 21 (P3.3 in the study guide).
//!
//! Architecture: one mpsc channel of boxed jobs. The single `Sender` lives
//! in the pool; the single `Receiver` is shared by all workers behind
//! `Arc<Mutex<Receiver<Job>>>` (mpsc = multi-producer, *single*-consumer,
//! so consumers must take turns via the lock).
//!
//! Shutdown: `Drop` drops the sender first — every worker's `recv()` then
//! returns `Err`, its loop ends, and `Drop` joins each thread.
#![allow(dead_code, unused_variables)]

type Job = Box<dyn FnOnce() + Send + 'static>;

/// A fixed-size pool of worker threads executing submitted closures.
pub struct ThreadPool {
    // Suggested fields — adjust as you like, the tests only use the public API:
    // workers: Vec<Worker>,
    // sender: Option<std::sync::mpsc::Sender<Job>>,
}

impl ThreadPool {
    /// Create a pool with `n` worker threads.
    ///
    /// # Panics
    /// Panics if `n == 0`.
    pub fn new(n: usize) -> ThreadPool {
        todo!("implement ThreadPool::new")
    }

    /// Submit a closure for execution on some worker thread.
    ///
    /// `FnOnce() + Send + 'static`: called once, movable across threads,
    /// owns everything it captures.
    pub fn execute<F>(&self, f: F)
    where
        F: FnOnce() + Send + 'static,
    {
        todo!("implement ThreadPool::execute")
    }
}

impl Drop for ThreadPool {
    /// Graceful shutdown: close the job channel (drop the sender), then
    /// `join` every worker so all submitted jobs finish before `drop`
    /// returns. Must not hang — even for a pool that never ran a job.
    fn drop(&mut self) {
        todo!("implement Drop for ThreadPool")
    }
}
