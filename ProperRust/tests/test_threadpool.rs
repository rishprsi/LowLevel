use proper_rust::threadpool::ThreadPool;
use std::collections::HashSet;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

#[test]
fn all_tasks_run_before_drop_returns() {
    let counter = Arc::new(AtomicUsize::new(0));
    {
        let pool = ThreadPool::new(4);
        for _ in 0..1000 {
            let counter = Arc::clone(&counter);
            pool.execute(move || {
                counter.fetch_add(1, Ordering::SeqCst);
            });
        }
        // Pool dropped here: graceful shutdown must wait for all 1000 tasks.
    }
    assert_eq!(counter.load(Ordering::SeqCst), 1000);
}

#[test]
fn tasks_run_on_multiple_threads() {
    let ids: Arc<Mutex<HashSet<thread::ThreadId>>> = Arc::new(Mutex::new(HashSet::new()));
    {
        let pool = ThreadPool::new(4);
        for _ in 0..16 {
            let ids = Arc::clone(&ids);
            pool.execute(move || {
                ids.lock().unwrap().insert(thread::current().id());
                // Sleep so one worker can't gobble every job.
                thread::sleep(Duration::from_millis(20));
            });
        }
    }
    let distinct = ids.lock().unwrap().len();
    assert!(
        distinct > 1,
        "expected tasks on multiple threads, all ran on {distinct}"
    );
}

#[test]
fn tasks_do_not_run_on_caller_thread() {
    let caller = thread::current().id();
    let ran_on = Arc::new(Mutex::new(None));
    {
        let pool = ThreadPool::new(1);
        let ran_on = Arc::clone(&ran_on);
        pool.execute(move || {
            *ran_on.lock().unwrap() = Some(thread::current().id());
        });
    }
    let worker = ran_on.lock().unwrap().expect("task never ran");
    assert_ne!(worker, caller);
}

#[test]
fn dropping_unused_pool_does_not_hang() {
    let pool = ThreadPool::new(4);
    drop(pool);
    // Reaching this line at all is the assertion.
}

#[test]
fn single_worker_runs_tasks_in_order() {
    let log = Arc::new(Mutex::new(Vec::new()));
    {
        let pool = ThreadPool::new(1);
        for i in 0..50 {
            let log = Arc::clone(&log);
            pool.execute(move || {
                log.lock().unwrap().push(i);
            });
        }
    }
    let log = log.lock().unwrap();
    assert_eq!(*log, (0..50).collect::<Vec<_>>());
}

#[test]
fn pool_is_reusable_across_bursts() {
    let counter = Arc::new(AtomicUsize::new(0));
    {
        let pool = ThreadPool::new(2);
        for _ in 0..100 {
            let counter = Arc::clone(&counter);
            pool.execute(move || {
                counter.fetch_add(1, Ordering::SeqCst);
            });
        }
        thread::sleep(Duration::from_millis(50));
        for _ in 0..100 {
            let counter = Arc::clone(&counter);
            pool.execute(move || {
                counter.fetch_add(1, Ordering::SeqCst);
            });
        }
    }
    assert_eq!(counter.load(Ordering::SeqCst), 200);
}

#[test]
#[should_panic]
fn zero_sized_pool_panics() {
    let _ = ThreadPool::new(0);
}
